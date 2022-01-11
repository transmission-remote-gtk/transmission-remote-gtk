/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011-2013  Alan Fitton

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <gio/gio.h>
#include <glib/gstdio.h>
#if HAVE_GEOIP
#include <GeoIP.h>
#include <GeoIPCity.h>
#endif

#include "trg-tree-view.h"
#include "torrent.h"
#include "trg-client.h"
#include "trg-peers-model.h"
#include "trg-model.h"
#include "util.h"

G_DEFINE_TYPE(TrgPeersModel, trg_peers_model, GTK_TYPE_LIST_STORE)
#define TRG_PEERS_MODEL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_PEERS_MODEL, TrgPeersModelPrivate))
#if HAVE_GEOIP
typedef struct _TrgPeersModelPrivate TrgPeersModelPrivate;

struct _TrgPeersModelPrivate {
    GeoIP *geoip;
    GeoIP *geoipv6;
    GeoIP *geoipcity;
};
#endif

static void trg_peers_model_class_init(TrgPeersModelClass *
                                       klass G_GNUC_UNUSED)
{
#if HAVE_GEOIP
    g_type_class_add_private(klass, sizeof(TrgPeersModelPrivate));
#endif
}

static gboolean
find_existing_peer_item_foreachfunc(GtkTreeModel * model,
                                    GtkTreePath *
                                    path G_GNUC_UNUSED,
                                    GtkTreeIter * iter, gpointer data)
{
    struct peerAndIter *pi = (struct peerAndIter *) data;
    gchar *ip;

    gtk_tree_model_get(model, iter, PEERSCOL_IP, &ip, -1);

    if (g_strcmp0(ip, pi->ip) == 0) {
        pi->iter = *iter;
        pi->found = TRUE;
    }

    g_free(ip);

    return pi->found;
}

static gboolean
find_existing_peer_item(TrgPeersModel * model, JsonObject * p,
                        GtkTreeIter * iter)
{
    struct peerAndIter pi;
    pi.ip = peer_get_address(p);
    pi.found = FALSE;

    gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                           find_existing_peer_item_foreachfunc, &pi);

    if (pi.found == TRUE)
        *iter = pi.iter;

    return pi.found;
}

struct ResolvedDnsIdleData {
    GtkTreeRowReference *rowRef;
    gchar *rdns;
};

static gboolean resolved_dns_idle_cb(gpointer data)
{
    struct ResolvedDnsIdleData *idleData = data;
    GtkTreeModel *model =
        gtk_tree_row_reference_get_model(idleData->rowRef);
    GtkTreePath *path = gtk_tree_row_reference_get_path(idleData->rowRef);

    if (path != NULL) {
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter(model, &iter, path) == TRUE) {
            gtk_list_store_set(GTK_LIST_STORE(model), &iter, PEERSCOL_HOST,
                               idleData->rdns, -1);
        }
        gtk_tree_path_free(path);
    }

    gtk_tree_row_reference_free(idleData->rowRef);
    g_free(idleData->rdns);
    g_free(idleData);

    return FALSE;
}

static void resolved_dns_cb(GObject * source_object, GAsyncResult * res,
                            gpointer data)
{

    gchar *rdns =
        g_resolver_lookup_by_address_finish(G_RESOLVER(source_object),
                                            res, NULL);
    GtkTreeRowReference *rowRef = data;

    if (rdns != NULL) {
        struct ResolvedDnsIdleData *idleData =
            g_new(struct ResolvedDnsIdleData, 1);
        idleData->rdns = rdns;
        idleData->rowRef = rowRef;
        g_idle_add(resolved_dns_idle_cb, idleData);
    } else {
        gtk_tree_row_reference_free(rowRef);
    }
}

#if HAVE_GEOIP
/* for handling v4 or v6 addresses. string is owned by GeoIP, should not be freed. */
static const gchar* lookup_country(TrgPeersModel *model, const gchar *address) {
	TrgPeersModelPrivate *priv = TRG_PEERS_MODEL_GET_PRIVATE(model);

	if (strchr(address, ':') && priv->geoipv6)
		return GeoIP_country_name_by_addr_v6(priv->geoipv6, address);
	else if (priv->geoip)
		return GeoIP_country_name_by_addr(priv->geoip, address);
	else
		return NULL;
}
#endif

void
trg_peers_model_update(TrgPeersModel * model, TrgTreeView * tv,
                       gint64 updateSerial, JsonObject * t, gint mode)
{
#if HAVE_GEOIP
    TrgPeersModelPrivate *priv = TRG_PEERS_MODEL_GET_PRIVATE(model);
    gboolean doGeoLookup =
        trg_tree_view_is_column_showing(tv, PEERSCOL_COUNTRY);
    gboolean doGeoCityLookup =
        trg_tree_view_is_column_showing(tv, PEERSCOL_CITY);
#endif

    gboolean doHostLookup =
        trg_tree_view_is_column_showing(tv, PEERSCOL_HOST);
    JsonArray *peers;
    GtkTreeIter peerIter;
    GList *li, *peersList;
    gboolean isNew;

    peers = torrent_get_peers(t);

    if (mode == TORRENT_GET_MODE_FIRST)
        gtk_list_store_clear(GTK_LIST_STORE(model));

    peersList = json_array_get_elements(peers);
    for (li = peersList; li; li = g_list_next(li)) {
        JsonObject *peer = json_node_get_object((JsonNode *) li->data);
        const gchar *address = NULL, *flagStr;
#if HAVE_GEOIP
        const gchar *country = NULL;
        GeoIPRecord *city = NULL;
#endif

        if (mode == TORRENT_GET_MODE_FIRST
            || find_existing_peer_item(model, peer, &peerIter) == FALSE) {
            gtk_list_store_append(GTK_LIST_STORE(model), &peerIter);

            address = peer_get_address(peer);
#if HAVE_GEOIP
            if (address) {       /* just in case address wasn't set */
            	if (doGeoLookup)
            		country = lookup_country(model, address);
            	if (doGeoCityLookup)
            		city = GeoIP_record_by_addr(priv->geoipcity, address);
            }
#endif
            gtk_list_store_set(GTK_LIST_STORE(model), &peerIter,
                               PEERSCOL_ICON, "network-workgroup",
                               PEERSCOL_IP, address,
#if HAVE_GEOIP
                               PEERSCOL_COUNTRY, country ? country : "",
                               PEERSCOL_CITY, city ? city->city : "",
#endif
                               PEERSCOL_CLIENT, peer_get_client_name(peer),
                               -1);

            isNew = TRUE;
        } else {
            isNew = FALSE;
        }

#if HAVE_GEOIP
        if (city)
        	GeoIPRecord_delete(city);
#endif

        flagStr = peer_get_flagstr(peer);
        gtk_list_store_set(GTK_LIST_STORE(model), &peerIter,
                           PEERSCOL_FLAGS, flagStr, PEERSCOL_PROGRESS,
                           peer_get_progress(peer), PEERSCOL_DOWNSPEED,
                           peer_get_rate_to_client(peer), PEERSCOL_UPSPEED,
                           peer_get_rate_to_peer(peer),
                           PEERSCOL_UPDATESERIAL, updateSerial, -1);

        if (doHostLookup && isNew == TRUE) {
            GtkTreePath *path =
                gtk_tree_model_get_path(GTK_TREE_MODEL(model),
                                        &peerIter);
            GtkTreeRowReference *treeRef =
                gtk_tree_row_reference_new(GTK_TREE_MODEL(model), path);
            GInetAddress *inetAddr;
            GResolver *resolver;

            gtk_tree_path_free(path);

            inetAddr = g_inet_address_new_from_string(address);
            resolver = g_resolver_get_default();
            g_resolver_lookup_by_address_async(resolver, inetAddr, NULL,
                                               resolved_dns_cb, treeRef);
            g_object_unref(resolver);
            g_object_unref(inetAddr);
        }
    }

    g_list_free(peersList);

    if (mode != TORRENT_GET_MODE_FIRST)
        trg_model_remove_removed(GTK_LIST_STORE(model),
                                 PEERSCOL_UPDATESERIAL, updateSerial);
}

static void trg_peers_model_init(TrgPeersModel * self)
{
#if HAVE_GEOIP
    TrgPeersModelPrivate *priv = TRG_PEERS_MODEL_GET_PRIVATE(self);
    gchar *geoip_db_path = NULL;
    gchar *geoip_v6_db_path = NULL;
    gchar *geoip_city_db_path = NULL;
    gchar *geoip_city_alt_db_path = NULL;
#endif

    GType column_types[PEERSCOL_COLUMNS];

    column_types[PEERSCOL_ICON] = G_TYPE_STRING;
    column_types[PEERSCOL_IP] = G_TYPE_STRING;
#if HAVE_GEOIP
    column_types[PEERSCOL_COUNTRY] = G_TYPE_STRING;
    column_types[PEERSCOL_CITY] = G_TYPE_STRING;
#endif
    column_types[PEERSCOL_HOST] = G_TYPE_STRING;
    column_types[PEERSCOL_FLAGS] = G_TYPE_STRING;
    column_types[PEERSCOL_PROGRESS] = G_TYPE_DOUBLE;
    column_types[PEERSCOL_DOWNSPEED] = G_TYPE_INT64;
    column_types[PEERSCOL_UPSPEED] = G_TYPE_INT64;
    column_types[PEERSCOL_CLIENT] = G_TYPE_STRING;
    column_types[PEERSCOL_UPDATESERIAL] = G_TYPE_INT64;

    gtk_list_store_set_column_types(GTK_LIST_STORE(self), PEERSCOL_COLUMNS,
                                    column_types);

#if HAVE_GEOIP
#ifdef G_OS_WIN32
    geoip_db_path = trg_win32_support_path("GeoIP.dat");
    geoip_v6_db_path = trg_win32_support_path("GeoIPv6.dat");
    geoip_city_db_path = trg_win32_support_path("GeoLiteCity.dat");
    geoip_city_alt_db_path = trg_win32_support_path("GeoIPCity.dat");
#else
    geoip_db_path = g_strdup(TRG_GEOIP_DATABASE);
    geoip_v6_db_path = g_strdup(TRG_GEOIPV6_DATABASE);
    geoip_city_db_path = g_strdup(TRG_GEOIP_CITY_DATABASE);
    geoip_city_alt_db_path = g_strdup(TRG_GEOIP_CITY_ALT_DATABASE);
#endif

    if (g_file_test(geoip_db_path, G_FILE_TEST_EXISTS) == TRUE)
        priv->geoip = GeoIP_open(geoip_db_path,
                                 GEOIP_STANDARD | GEOIP_CHECK_CACHE);

    if (g_file_test(geoip_v6_db_path, G_FILE_TEST_EXISTS) == TRUE)
        priv->geoipv6 = GeoIP_open(geoip_v6_db_path,
                                   GEOIP_STANDARD | GEOIP_CHECK_CACHE);

    if (g_file_test(geoip_city_db_path, G_FILE_TEST_EXISTS) == TRUE)
        priv->geoipcity = GeoIP_open(geoip_city_db_path,
                                   GEOIP_STANDARD | GEOIP_CHECK_CACHE);
    else if (g_file_test(geoip_city_alt_db_path, G_FILE_TEST_EXISTS) == TRUE)
        priv->geoipcity = GeoIP_open(geoip_city_alt_db_path,
                                   GEOIP_STANDARD | GEOIP_CHECK_CACHE);

    if (priv->geoipcity)
    	GeoIP_set_charset(priv->geoipcity, GEOIP_CHARSET_UTF8);

    g_free(geoip_city_db_path);
    g_free(geoip_city_alt_db_path);
    g_free(geoip_db_path);
    g_free(geoip_v6_db_path);
#endif
}

#if HAVE_GEOIP
static gboolean trg_peers_model_add_city_foreach(GtkTreeModel *model,
        GtkTreePath *path,
        GtkTreeIter *iter,
        gpointer data) {
	TrgPeersModelPrivate *priv = TRG_PEERS_MODEL_GET_PRIVATE(model);
	gchar *address = NULL;
	GeoIPRecord *record = NULL;

	gtk_tree_model_get(GTK_TREE_MODEL(model), iter, PEERSCOL_IP, &address, -1);
	record = GeoIP_record_by_addr(priv->geoipcity, address);

	if (record) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, PEERSCOL_CITY, record->city, -1);
		GeoIPRecord_delete(record);
	}

	g_free(address);

	return FALSE;
}

gboolean trg_peers_model_has_city_db(TrgPeersModel *model) {
	TrgPeersModelPrivate *priv = TRG_PEERS_MODEL_GET_PRIVATE(model);
	return priv->geoipcity != NULL;
}

gboolean trg_peers_model_has_country_db(TrgPeersModel *model) {
	TrgPeersModelPrivate *priv = TRG_PEERS_MODEL_GET_PRIVATE(model);
	return priv->geoip != NULL;
}

void trg_peers_model_add_city_column(TrgPeersModel *model) {
	TrgPeersModelPrivate *priv = TRG_PEERS_MODEL_GET_PRIVATE(model);
	if (priv->geoipcity)
		gtk_tree_model_foreach(GTK_TREE_MODEL(model), trg_peers_model_add_city_foreach, NULL);
}

static gboolean trg_peers_model_add_country_foreach(GtkTreeModel *model,
        GtkTreePath *path,
        GtkTreeIter *iter,
        gpointer data) {
	gchar *address = NULL;

	gtk_tree_model_get(GTK_TREE_MODEL(model), iter, PEERSCOL_IP, &address, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), iter, PEERSCOL_COUNTRY, lookup_country(TRG_PEERS_MODEL(model), address), -1);

	g_free(address);

	return FALSE;
}

void trg_peers_model_add_country_column(TrgPeersModel *model) {
	TrgPeersModelPrivate *priv = TRG_PEERS_MODEL_GET_PRIVATE(model);
	if (priv->geoip)
		gtk_tree_model_foreach(GTK_TREE_MODEL(model), trg_peers_model_add_country_foreach, NULL);
}
#endif


TrgPeersModel *trg_peers_model_new()
{
    return g_object_new(TRG_TYPE_PEERS_MODEL, NULL);
}
