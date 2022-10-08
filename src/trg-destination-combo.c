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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "torrent.h"
#include "trg-client.h"
#include "trg-destination-combo.h"
#include "trg-torrent-model.h"
#include "util.h"

struct _TrgDestinationCombo {
    GtkComboBox parent_instance;
};

typedef struct {
    TrgClient *client;
    gchar *last_selection;
} TrgDestinationComboPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(TrgDestinationCombo, trg_destination_combo, GTK_TYPE_COMBO_BOX)

enum {
    PROP_0,
    PROP_CLIENT,
    PROP_LAST_SELECTION
};

enum {
    DEST_DEFAULT,
    DEST_LABEL,
    DEST_EXISTING
};

enum {
    DEST_COLUMN_LABEL,
    DEST_COLUMN_DIR,
    DEST_COLUMN_TYPE,
    N_DEST_COLUMNS
};

static void trg_destination_combo_finalize(GObject *object)
{
    TrgDestinationComboPrivate *priv
        = trg_destination_combo_get_instance_private(TRG_DESTINATION_COMBO(object));
    g_free(priv->last_selection);
}

static void trg_destination_combo_get_property(GObject *object, guint property_id, GValue *value,
                                               GParamSpec *pspec)
{
    TrgDestinationComboPrivate *priv
        = trg_destination_combo_get_instance_private(TRG_DESTINATION_COMBO(object));
    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    case PROP_LAST_SELECTION:
        g_value_set_string(value, priv->last_selection);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_destination_combo_set_property(GObject *object, guint property_id,
                                               const GValue *value, GParamSpec *pspec)
{
    TrgDestinationComboPrivate *priv
        = trg_destination_combo_get_instance_private(TRG_DESTINATION_COMBO(object));
    switch (property_id) {
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    case PROP_LAST_SELECTION:
        priv->last_selection = g_strdup(g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static gboolean g_slist_str_set_add(GSList **list, const gchar *string)
{
    GSList *li;
    for (li = *list; li; li = g_slist_next(li))
        if (!g_strcmp0((gchar *)li->data, string))
            return FALSE;

    *list = g_slist_insert_sorted(*list, (gpointer)string, (GCompareFunc)g_strcmp0);

    return TRUE;
}

void trg_destination_combo_save_selection(TrgDestinationCombo *combo_box)
{
    TrgDestinationComboPrivate *priv = trg_destination_combo_get_instance_private(combo_box);
    GtkTreeIter iter;

    if (priv->last_selection && gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo_box), &iter)) {
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box));
        TrgPrefs *prefs = trg_client_get_prefs(priv->client);
        gchar *text;

        gtk_tree_model_get(model, &iter, DEST_COLUMN_DIR, &text, -1);
        trg_prefs_set_string(prefs, priv->last_selection, text, TRG_PREFS_CONNECTION);
        g_free(text);
    }
}

static inline GtkEntry *trg_destination_combo_get_entry(TrgDestinationCombo *combo)
{
    return GTK_ENTRY(gtk_bin_get_child(GTK_BIN(combo)));
}

gboolean trg_destination_combo_has_text(TrgDestinationCombo *combo)
{
    const gchar *text
        = gtk_entry_get_text(trg_destination_combo_get_entry(TRG_DESTINATION_COMBO(combo)));
    return strlen(text) > 0;
}

struct findDupeArg {
    const gchar *dir;
    gboolean isDupe;
};

static gboolean trg_destination_combo_insert_check_dupe_foreach(GtkTreeModel *model,
                                                                GtkTreePath *path G_GNUC_UNUSED,
                                                                GtkTreeIter *iter,
                                                                struct findDupeArg *args)
{
    gchar *existing;
    gtk_tree_model_get(model, iter, DEST_COLUMN_DIR, &existing, -1);
    args->isDupe = g_strcmp0(existing, args->dir) == 0;
    g_free(existing);
    return args->isDupe;
}

static void trg_destination_combo_insert(GtkComboBox *box, const gchar *label, const gchar *dir,
                                         guint type)
{
    GtkTreeModel *model = gtk_combo_box_get_model(box);
    gchar *comboLabel;

    if (type == DEST_EXISTING) {
        struct findDupeArg args;
        args.isDupe = FALSE;
        args.dir = dir;
        gtk_tree_model_foreach(
            GTK_TREE_MODEL(model),
            (GtkTreeModelForeachFunc)trg_destination_combo_insert_check_dupe_foreach, &args);
        if (args.isDupe)
            return;
    }

    comboLabel = label ? g_strdup_printf("%s (%s)", label, dir) : g_strdup(dir);

    gtk_list_store_insert_with_values(GTK_LIST_STORE(model), NULL, -1, DEST_COLUMN_LABEL,
                                      comboLabel, DEST_COLUMN_DIR, dir, DEST_COLUMN_TYPE, type, -1);
    g_free(comboLabel);
}

gchar *trg_destination_combo_get_dir(TrgDestinationCombo *combo)
{
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

static void load_directory_model(TrgDestinationCombo *self)
{
    TrgDestinationComboPrivate *priv = trg_destination_combo_get_instance_private(self);
    TrgClient *client = priv->client;
    TrgPrefs *prefs = trg_client_get_prefs(client);

    GSList *dirs = NULL, *sli;
    GList *li, *list;

    JsonArray *savedDestinations;

    /* Add default dir */
    g_autofree gchar *defaultDir = NULL;
    defaultDir = g_strdup(session_get_download_dir(trg_client_get_session(client)));
    rm_trailing_slashes(defaultDir);

    trg_destination_combo_insert(GTK_COMBO_BOX(self), NULL, defaultDir, DEST_DEFAULT);

    /* Add saved dirs */
    savedDestinations
        = trg_prefs_get_array(prefs, TRG_PREFS_KEY_DESTINATIONS, TRG_PREFS_CONNECTION);
    if (savedDestinations) {
        list = json_array_get_elements(savedDestinations);
        if (list) {
            for (li = list; li; li = g_list_next(li)) {
                JsonObject *obj = json_node_get_object((JsonNode *)li->data);
                trg_destination_combo_insert(
                    GTK_COMBO_BOX(self), json_object_get_string_member(obj, TRG_PREFS_SUBKEY_LABEL),
                    json_object_get_string_member(obj, TRG_PREFS_KEY_DESTINATIONS_SUBKEY_DIR),
                    DEST_LABEL);
            }
            g_list_free(list);
        }
    }

    /* Add all previously used download dirs */
    list = g_hash_table_get_values(trg_client_get_torrent_table(client));
    for (li = list; li; li = g_list_next(li)) {
        GtkTreeRowReference *rr;
        GtkTreeModel *model;
        GtkTreePath *path;

        rr = (GtkTreeRowReference *)li->data;
        model = gtk_tree_row_reference_get_model(rr);
        path = gtk_tree_row_reference_get_path(rr);

        if (path) {
            GtkTreeIter iter;

            if (gtk_tree_model_get_iter(model, &iter, path)) {
                gchar *dd;

                gtk_tree_model_get(model, &iter, TORRENT_COLUMN_DOWNLOADDIR, &dd, -1);

                if (dd && g_strcmp0(dd, defaultDir))
                    g_slist_str_set_add(&dirs, dd);
                else
                    g_free(dd);
            }

            gtk_tree_path_free(path);
        }
    }

    for (sli = dirs; sli; sli = g_slist_next(sli))
        trg_destination_combo_insert(GTK_COMBO_BOX(self), NULL, (gchar *)sli->data, DEST_EXISTING);

    g_slist_free_full(dirs, g_free);
    g_list_free(list);
}

static void set_text_column(GtkCellLayout *layout, guint col)
{
    GList *cells = gtk_cell_layout_get_cells(layout);
    g_assert(cells != NULL);
    gtk_cell_layout_set_attributes(layout, GTK_CELL_RENDERER(cells->data), "text", col, NULL);
    g_list_free(cells);
}

static void trg_destination_combo_constructed(GObject *object)
{
    TrgDestinationCombo *self = TRG_DESTINATION_COMBO(object);
    TrgDestinationComboPrivate *priv = trg_destination_combo_get_instance_private(self);
    TrgPrefs *prefs = trg_client_get_prefs(priv->client);

    G_OBJECT_CLASS(trg_destination_combo_parent_class)->constructed(object);

    load_directory_model(self);
    set_text_column(GTK_CELL_LAYOUT(self), DEST_COLUMN_LABEL);

    /* Must be set after constructed */
    if (priv->last_selection) {
        /* Restore any previous selection */
        char *lastDestination
            = trg_prefs_get_string(prefs, priv->last_selection, TRG_PREFS_CONNECTION);
        if (!gtk_combo_box_set_active_id(GTK_COMBO_BOX(object), lastDestination))
            g_warning("Last selection was not a valid ID");
        g_free(lastDestination);
    } else {
        /* DefaultDir is the first item otherwise */
        gtk_combo_box_set_active(GTK_COMBO_BOX(object), 0);
    }
}

static void trg_destination_combo_class_init(TrgDestinationComboClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_destination_combo_get_property;
    object_class->set_property = trg_destination_combo_set_property;
    object_class->finalize = trg_destination_combo_finalize;
    object_class->constructed = trg_destination_combo_constructed;

    g_object_class_install_property(
        object_class, PROP_CLIENT,
        g_param_spec_pointer("trg-client", "TClient", "Client",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(
        object_class, PROP_LAST_SELECTION,
        g_param_spec_string("last-selection-key", "LastSelectionKey", "LastSelectionKey", NULL,
                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
}

static void trg_destination_combo_init(TrgDestinationCombo *self)
{
    GtkListStore *store;

    store = gtk_list_store_new(N_DEST_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);
    gtk_combo_box_set_model(GTK_COMBO_BOX(self), GTK_TREE_MODEL(store));
    gtk_combo_box_set_id_column(GTK_COMBO_BOX(self), DEST_COLUMN_DIR);
    gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(self), DEST_COLUMN_LABEL);
    g_object_unref(store);
}

GtkWidget *trg_destination_combo_new(TrgClient *client, const gchar *lastSelectionKey)
{
    return GTK_WIDGET(g_object_new(TRG_TYPE_DESTINATION_COMBO, "has-entry", TRUE, "trg-client",
                                   client, "last-selection-key", lastSelectionKey, NULL));
}
