/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011  Alan Fitton

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>

#ifdef HAVE_LIBPROXY
#include <proxy.h>
#endif

#include "trg-prefs.h"
#include "util.h"
#include "dispatch.h"
#include "trg-client.h"

enum {
    CLIENT_SIGNAL_PROFILE_CHANGE,
    CLIENT_SIGNAL_PROFILE_NEW,
    CLIENT_SIGNAL_COUNT
};

static guint signals[CLIENT_SIGNAL_COUNT] = { 0 };

G_DEFINE_TYPE (TrgClient, trg_client, G_TYPE_OBJECT)

#define TRG_CLIENT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_CLIENT, TrgClientPrivate))

typedef struct _TrgClientPrivate TrgClientPrivate;

struct _TrgClientPrivate {
    char *session_id;
    gboolean activeOnlyUpdate;
    guint failCount;
    guint interval;
    guint min_interval;
    gint64 updateSerial;
    JsonObject *session;
    gboolean ssl;
    float version;
    char *url;
    char *username;
    char *password;
    char *proxy;
    GHashTable *torrentTable;
    GThreadPool *pool;
    GMutex *updateMutex;
    TrgPrefs *prefs;
};

static void
trg_client_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
trg_client_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
trg_client_dispose (GObject *object)
{
  G_OBJECT_CLASS (trg_client_parent_class)->dispose (object);
}

static void
trg_client_class_init (TrgClientClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TrgClientPrivate));

  object_class->get_property = trg_client_get_property;
  object_class->set_property = trg_client_set_property;
  object_class->dispose = trg_client_dispose;

  signals[CLIENT_SIGNAL_PROFILE_CHANGE] =
          gtk_signal_new ("client-profile-changed",
                      GTK_RUN_LAST,
                      G_TYPE_FROM_CLASS(object_class),
                      GTK_SIGNAL_OFFSET (TrgClientClass, client_profile_changed),
                      gtk_marshal_NONE__NONE,
                      GTK_TYPE_NONE, 0);

  signals[CLIENT_SIGNAL_PROFILE_NEW] =
          gtk_signal_new ("client-profile-new",
                      GTK_RUN_LAST,
                      G_TYPE_FROM_CLASS(object_class),
                      GTK_SIGNAL_OFFSET (TrgClientClass, client_profile_new),
                      gtk_marshal_NONE__NONE,
                      GTK_TYPE_NONE, 0);
}

static void
trg_client_init (TrgClient *self)
{
}

TrgClient*
trg_client_new (void)
{
    TrgClient *tc = g_object_new (TRG_TYPE_CLIENT, NULL);
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    TrgPrefs *prefs = priv->prefs = trg_prefs_new();

    trg_prefs_load(prefs);

    priv->updateMutex = g_mutex_new();
    priv->activeOnlyUpdate =
        trg_prefs_get_bool(prefs,
                              TRG_PREFS_KEY_UPDATE_ACTIVE_ONLY, TRG_PREFS_PROFILE);
    priv->pool = dispatch_init_pool(tc);

    return tc;
}

float trg_client_get_version(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->version;
}

void trg_client_set_session(TrgClient * tc, JsonObject * session)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);

    if (priv->session)
        json_object_unref(priv->session);

    session_get_version(session, &priv->version);

    priv->session = session;
}

TrgPrefs *trg_client_get_prefs(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->prefs;
}

int trg_client_populate_with_settings(TrgClient * tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    TrgPrefs *prefs = priv->prefs;

    gint port;
    gchar *host;
#ifdef HAVE_LIBPROXY
    pxProxyFactory *pf = NULL;
#endif

    g_free(priv->url);
    priv->url = NULL;

    g_free(priv->username);
    priv->username = NULL;

    g_free(priv->password);
    priv->password = NULL;

    port =
        trg_prefs_get_int(prefs, TRG_PREFS_KEY_PORT, TRG_PREFS_PROFILE);

    host = trg_prefs_get_string(prefs, TRG_PREFS_KEY_HOSTNAME, TRG_PREFS_PROFILE);
    if (!host || strlen(host) < 1)
        return TRG_NO_HOSTNAME_SET;

    priv->ssl = trg_prefs_get_bool(prefs, TRG_PREFS_KEY_SSL, TRG_PREFS_PROFILE);

    priv->url =
        g_strdup_printf("%s://%s:%d/transmission/rpc",
                        priv->ssl ? "https" : "http", host, port);
    g_free(host);

    priv->interval =
        trg_prefs_get_int(prefs, TRG_PREFS_KEY_UPDATE_INTERVAL, TRG_PREFS_PROFILE);
    if (priv->interval < 1)
        priv->interval = TRG_INTERVAL_DEFAULT;

    priv->min_interval =
        trg_prefs_get_int(prefs, TRG_PREFS_KEY_MINUPDATE_INTERVAL, TRG_PREFS_PROFILE);
    if (priv->interval < 1)
        priv->interval = TRG_INTERVAL_DEFAULT;

    priv->username =
        trg_prefs_get_string(prefs, TRG_PREFS_KEY_USERNAME, TRG_PREFS_PROFILE);

    priv->password =
        trg_prefs_get_string(prefs, TRG_PREFS_KEY_PASSWORD, TRG_PREFS_PROFILE);

    g_free(priv->proxy);
    priv->proxy = NULL;

#ifdef HAVE_LIBPROXY
    if ((pf = px_proxy_factory_new())) {
        char **proxies = px_proxy_factory_get_proxies(pf, priv->url);
        int i;

        for (i = 0; proxies[i]; i++) {
            if (g_str_has_prefix(proxies[i], "http")) {
                g_free(priv->proxy);
                priv->proxy = proxies[i];
            } else {
                g_free(proxies[i]);
            }
        }

        g_free(proxies);
        px_proxy_factory_free(pf);
    }
#endif

    return 0;
}

gchar *trg_client_get_password(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->password;
}

gchar *trg_client_get_username(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->username;
}

gchar *trg_client_get_url(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->url;
}

gchar *trg_client_get_session_id(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->session_id;
}

void trg_client_set_session_id(TrgClient *tc, gchar *session_id)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->session_id = session_id;
}

void trg_client_status_change(TrgClient *tc, gboolean connected)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    if (!connected) {
        json_object_unref(priv->session);
        priv->session = NULL;
    }
}

JsonObject* trg_client_get_session(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->session;
}

void trg_client_thread_pool_push(TrgClient *tc, gpointer data, GError **err)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    g_thread_pool_push(priv->pool, data, err);
}

void trg_client_inc_serial(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->updateSerial++;
}

gint64 trg_client_get_serial(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->updateSerial;
}

gboolean trg_client_get_ssl(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->ssl;
}

gchar *trg_client_get_proxy(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->proxy;
}

void trg_client_set_torrent_table(TrgClient *tc, GHashTable *table)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->torrentTable = table;
}

GHashTable* trg_client_get_torrent_table(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->torrentTable;
}

gboolean trg_client_is_connected(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->session != NULL;
}

void trg_client_updatelock(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    g_mutex_lock(priv->updateMutex);
}

guint trg_client_get_failcount(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->failCount;
}

guint trg_client_inc_failcount(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return ++(priv->failCount);
}

void trg_client_reset_failcount(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->failCount = 0;
}

void trg_client_updateunlock(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    g_mutex_unlock(priv->updateMutex);
}

gboolean trg_client_get_activeonlyupdate(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->activeOnlyUpdate;
}

void trg_client_set_activeonlyupdate(TrgClient *tc, gboolean activeOnlyUpdate)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->activeOnlyUpdate = activeOnlyUpdate;
}

guint trg_client_get_minimised_interval(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->min_interval;
}

guint trg_client_get_interval(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->interval;
}

void trg_client_set_interval(TrgClient *tc, guint interval)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->interval = interval;
}

void trg_client_set_minimised_interval(TrgClient *tc, guint interval)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->min_interval = interval;
}
