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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-client.h"
#include "torrent.h"
#include "trg-torrent-model.h"
#include "trg-destination-combo.h"
#include "util.h"

G_DEFINE_TYPE(TrgDestinationCombo, trg_destination_combo,
        GTK_TYPE_COMBO_BOX)
#define TRG_DESTINATION_COMBO_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_DESTINATION_COMBO, TrgDestinationComboPrivate))
typedef struct _TrgDestinationComboPrivate TrgDestinationComboPrivate;

struct _TrgDestinationComboPrivate {
    TrgClient *client;
};

enum {
    PROP_0, PROP_CLIENT
};

enum {
    DEST_DEFAULT, DEST_LABEL, DEST_EXISTING, DEST_USERADD
};

enum {
    DEST_COLUMN_LABEL, DEST_COLUMN_DIR, DEST_COLUMN_TYPE, N_DEST_COLUMNS
};

static void trg_destination_combo_get_property(GObject * object,
        guint property_id, GValue * value, GParamSpec * pspec) {
    TrgDestinationComboPrivate *priv =
            TRG_DESTINATION_COMBO_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_destination_combo_set_property(GObject * object,
        guint property_id, const GValue * value, GParamSpec * pspec) {
    TrgDestinationComboPrivate *priv =
            TRG_DESTINATION_COMBO_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static gboolean g_slist_str_set_add(GSList ** list, const gchar * string) {
    GSList *li;
    for (li = *list; li; li = g_slist_next(li))
        if (!g_strcmp0((gchar *) li->data, string))
            return FALSE;

    *list = g_slist_insert_sorted(*list, (gpointer) string,
            (GCompareFunc) g_strcmp0);

    return TRUE;
}

/*static void trg_destination_combo_entry_changed(GtkEntry *entry,
 gpointer user_data) {
 GtkComboBox *combo = GTK_COMBO_BOX(user_data);
 GtkTreeIter iter;

 if (gtk_combo_box_get_active_iter(combo, &iter)) {
 GtkTreeModel *model = gtk_combo_box_get_model(combo);
 const gchar *text = gtk_entry_get_text(entry);
 gtk_list_store_set(GTK_LIST_STORE(model), &iter, DEST_COLUMN_DIR, text,
 DEST_COLUMN_LABEL, text, -1);
 }
 }*/

static void gtk_combo_box_entry_active_changed(GtkComboBox *combo_box,
        gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean editableEntry = TRUE;

    if (gtk_combo_box_get_active_iter(combo_box, &iter)) {
        GtkEntry *entry = trg_destination_combo_get_entry(
                TRG_DESTINATION_COMBO(combo_box));

        if (entry) {
            GValue value = { 0, };
            guint type;

            model = gtk_combo_box_get_model(combo_box);

            gtk_tree_model_get_value(model, &iter, DEST_COLUMN_LABEL, &value);
            gtk_tree_model_get(model, &iter, DEST_COLUMN_TYPE, &type, -1);

            g_object_set_property(G_OBJECT (entry), "text", &value);
            g_value_unset(&value);

            if (type == DEST_LABEL)
                editableEntry = FALSE;
        }
    }

    gtk_entry_set_editable(
            trg_destination_combo_get_entry(TRG_DESTINATION_COMBO(combo_box)),
            editableEntry);
}

gboolean trg_destination_combo_has_text(TrgDestinationCombo *combo) {
    const gchar *text = gtk_entry_get_text(
            trg_destination_combo_get_entry(TRG_DESTINATION_COMBO(combo)));
    return strlen(text) > 0;
}

GtkEntry *trg_destination_combo_get_entry(TrgDestinationCombo *combo) {
    return GTK_ENTRY (gtk_bin_get_child (GTK_BIN (combo)));
}

static void add_entry_cb(GtkEntry *entry,
        GtkEntryIconPosition icon_pos,
        GdkEvent            *event,
        gpointer             user_data)
{
    GtkComboBox *combo = GTK_COMBO_BOX(user_data);
    GtkTreeModel *model = gtk_combo_box_get_model(combo);
    GtkTreeIter iter;

    gtk_list_store_insert_with_values(GTK_LIST_STORE(model), &iter, INT_MAX,
            DEST_COLUMN_LABEL, "",
            DEST_COLUMN_DIR, "",
            DEST_COLUMN_TYPE, DEST_USERADD, -1);
    gtk_combo_box_set_active_iter(combo, &iter);
}

static GObject *trg_destination_combo_constructor(GType type,
        guint n_construct_properties, GObjectConstructParam * construct_params) {
    GObject *object = G_OBJECT_CLASS
    (trg_destination_combo_parent_class)->constructor(type,
            n_construct_properties, construct_params);
    TrgDestinationComboPrivate *priv =
            TRG_DESTINATION_COMBO_GET_PRIVATE(object);

    TrgClient *client = priv->client;
    TrgPrefs *prefs = trg_client_get_prefs(client);
    GtkEntry *entry = trg_destination_combo_get_entry(TRG_DESTINATION_COMBO(object));

    GSList *dirs = NULL, *sli;
    GList *li, *list;
    GtkTreeRowReference *rr;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkListStore *comboModel;
    JsonArray *saved_destinations;
    JsonObject *t;
    gchar *defaultDir;

    comboModel = gtk_list_store_new(N_DEST_COLUMNS, G_TYPE_STRING,
            G_TYPE_STRING, G_TYPE_UINT);

    defaultDir = g_strdup(
            session_get_download_dir(trg_client_get_session(client)));
    rm_trailing_slashes(defaultDir);
    gtk_list_store_insert_with_values(comboModel, NULL, INT_MAX,
            DEST_COLUMN_LABEL, defaultDir, DEST_COLUMN_DIR, defaultDir,
            DEST_COLUMN_TYPE, DEST_DEFAULT, -1);

    saved_destinations = trg_prefs_get_array(prefs, TRG_PREFS_KEY_DESTINATIONS,
            TRG_PREFS_CONNECTION);
    if (saved_destinations) {
        list = json_array_get_elements(saved_destinations);
        if (list) {
            for (li = list; li; li = g_list_next(li)) {
                JsonObject *obj = json_node_get_object((JsonNode*) li->data);
                gtk_list_store_insert_with_values(
                        comboModel,
                        NULL,
                        INT_MAX,
                        DEST_COLUMN_LABEL,
                        json_object_get_string_member(obj,
                                TRG_PREFS_SUBKEY_LABEL),
                        DEST_COLUMN_DIR,
                        json_object_get_string_member(obj,
                                TRG_PREFS_KEY_DESTINATIONS_SUBKEY_DIR),
                        DEST_COLUMN_TYPE, DEST_LABEL, -1);
            }
            g_list_free(list);
        }
    }

    trg_client_updatelock(client);
    list = g_hash_table_get_values(trg_client_get_torrent_table(client));
    for (li = list; li; li = g_list_next(li)) {
        rr = (GtkTreeRowReference *) li->data;
        model = gtk_tree_row_reference_get_model(rr);
        path = gtk_tree_row_reference_get_path(rr);

        if (path) {
            GtkTreeIter iter;

            if (gtk_tree_model_get_iter(model, &iter, path)) {
                gchar *dd;

                gtk_tree_model_get(model, &iter, TORRENT_COLUMN_JSON, &t,
                        TORRENT_COLUMN_DOWNLOADDIR, &dd, -1);

                if (dd && g_strcmp0(dd, defaultDir))
                    g_slist_str_set_add(&dirs, dd);
                else
                    g_free(dd);
            }

            gtk_tree_path_free(path);
        }
    }

    trg_client_updateunlock(client);

    g_list_free(list);

    for (sli = dirs; sli; sli = g_slist_next(sli))
        gtk_list_store_insert_with_values(comboModel, NULL, INT_MAX,
                DEST_COLUMN_LABEL, (gchar *) sli->data, DEST_COLUMN_DIR,
                (gchar *) sli->data, DEST_COLUMN_TYPE, DEST_EXISTING, -1);

    gtk_combo_box_set_model(GTK_COMBO_BOX(object), GTK_TREE_MODEL(comboModel));

    gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(object), 0);

    g_object_unref(comboModel);
    g_slist_foreach(dirs, (GFunc) g_free, NULL);
    g_slist_free(dirs);

    g_free(defaultDir);

    g_signal_connect (object, "changed",
            G_CALLBACK (gtk_combo_box_entry_active_changed), NULL);

    gtk_entry_set_icon_from_stock(GTK_ENTRY(entry), GTK_ENTRY_ICON_SECONDARY,
            GTK_STOCK_CLEAR);

    g_signal_connect(entry, "icon-release",
            G_CALLBACK(add_entry_cb), object);

    return object;
}

gchar *trg_destination_combo_get_dir(TrgDestinationCombo *combo) {
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo));
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo), &iter)) {
        gchar *value;
        guint type;

        gtk_tree_model_get(model, &iter, DEST_COLUMN_TYPE, &type, -1);

        if (type == DEST_LABEL) {
            gtk_tree_model_get(model, &iter, DEST_COLUMN_DIR, &value, -1);
            return value;
        }
    }

    return g_strdup(gtk_entry_get_text(trg_destination_combo_get_entry(combo)));
}

static void trg_destination_combo_class_init(TrgDestinationComboClass * klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgDestinationComboPrivate));

    object_class->get_property = trg_destination_combo_get_property;
    object_class->set_property = trg_destination_combo_set_property;
    object_class->constructor = trg_destination_combo_constructor;

    g_object_class_install_property(
            object_class,
            PROP_CLIENT,
            g_param_spec_pointer(
                    "trg-client",
                    "TClient",
                    "Client",
                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
                            | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                            | G_PARAM_STATIC_BLURB));
}

static void trg_destination_combo_init(TrgDestinationCombo * self) {
}

GtkWidget *trg_destination_combo_new(TrgClient * client) {
    return GTK_WIDGET(g_object_new(TRG_TYPE_DESTINATION_COMBO,
                    "trg-client", client, "has-entry", TRUE, NULL));
}
