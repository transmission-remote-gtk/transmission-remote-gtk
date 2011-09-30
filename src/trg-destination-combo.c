/*
 * transmission-remote-gtk - Transmission RPC client for GTK
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

#include <gtk/gtk.h>

#include "trg-client.h"
#include "torrent.h"
#include "trg-torrent-model.h"
#include "trg-destination-combo.h"
#include "util.h"

G_DEFINE_TYPE(TrgDestinationCombo, trg_destination_combo,
              GTK_TYPE_COMBO_BOX_ENTRY)
#define TRG_DESTINATION_COMBO_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_DESTINATION_COMBO, TrgDestinationComboPrivate))
typedef struct _TrgDestinationComboPrivate TrgDestinationComboPrivate;

struct _TrgDestinationComboPrivate {
    TrgClient *client;
};

enum {
    PROP_0,
    PROP_CLIENT
};

static void
trg_destination_combo_get_property(GObject * object, guint property_id,
                                   GValue * value, GParamSpec * pspec)
{
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
trg_destination_combo_set_property(GObject * object, guint property_id,
                                   const GValue * value,
                                   GParamSpec * pspec)
{
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static gboolean g_slist_str_set_add(GSList ** list, const gchar * string, gint pos)
{
    GSList *li;
    for (li = *list; li; li = g_slist_next(li))
        if (!g_strcmp0((gchar *) li->data, string))
            return FALSE;

    if (pos < 0)
        *list = g_slist_insert_sorted(*list, (gpointer)string, (GCompareFunc)g_strcmp0);
    else
        *list = g_slist_insert(*list, (gpointer)string, pos);

    return TRUE;
}

static GObject *trg_destination_combo_constructor(GType type,
                                                  guint
                                                  n_construct_properties,
                                                  GObjectConstructParam
                                                  * construct_params)
{
    GObject *object = G_OBJECT_CLASS
        (trg_destination_combo_parent_class)->constructor(type,
                                                          n_construct_properties,
                                                          construct_params);
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(object);

    TrgClient *client = priv->client;
    GSList *dirs = NULL;
    GSList *sli;
    GList *li;
    GList *torrentItemRefs;

    GtkTreeRowReference *rr;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkListStore *comboModel;
    JsonObject *t;

    gchar *defaultDownDir =
            g_strdup(session_get_download_dir(trg_client_get_session(client)));
    rm_trailing_slashes(defaultDownDir);

    comboModel = gtk_list_store_new(1, G_TYPE_STRING);

    trg_client_updatelock(client);
    torrentItemRefs = g_hash_table_get_values(trg_client_get_torrent_table(client));
    for (li = torrentItemRefs; li; li = g_list_next(li)) {
        rr = (GtkTreeRowReference *) li->data;
        model = gtk_tree_row_reference_get_model(rr);
        path = gtk_tree_row_reference_get_path(rr);

        if (path) {
            GtkTreeIter iter;

            if (gtk_tree_model_get_iter(model, &iter, path)) {
                gchar *dd;
                gtk_tree_model_get(model, &iter, TORRENT_COLUMN_JSON, &t,
                        TORRENT_COLUMN_DOWNLOADDIR, &dd,
                                   -1);;
                if (dd && g_strcmp0(dd, defaultDownDir))
                    g_slist_str_set_add(&dirs, dd, -1);
                else
                    g_free(dd);
            }

            gtk_tree_path_free(path);
        }
    }

    trg_client_updateunlock(client);

    g_list_free(torrentItemRefs);
    if (defaultDownDir)
        g_slist_str_set_add(&dirs, defaultDownDir, 0);

    for (sli = dirs; sli; sli = g_slist_next(sli))
        gtk_list_store_insert_with_values(comboModel, NULL, INT_MAX, 0,
                                          (gchar *) sli->data, -1);

    gtk_combo_box_set_model(GTK_COMBO_BOX(object),
                            GTK_TREE_MODEL(comboModel));

    gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(object), 0);

    g_object_unref(comboModel);
    g_slist_foreach(dirs, (GFunc)g_free, NULL);
    g_slist_free(dirs);

    return object;
}

static void
trg_destination_combo_class_init(TrgDestinationComboClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgDestinationComboPrivate));

    object_class->get_property = trg_destination_combo_get_property;
    object_class->set_property = trg_destination_combo_set_property;
    object_class->constructor = trg_destination_combo_constructor;

    g_object_class_install_property(object_class,
                                    PROP_CLIENT,
                                    g_param_spec_pointer
                                    ("trg-client", "TClient",
                                     "Client",
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));
}

static void trg_destination_combo_init(TrgDestinationCombo * self)
{
}

GtkWidget *trg_destination_combo_new(TrgClient * client)
{
    return GTK_WIDGET(g_object_new(TRG_TYPE_DESTINATION_COMBO,
                                   "trg-client", client, NULL));
}
