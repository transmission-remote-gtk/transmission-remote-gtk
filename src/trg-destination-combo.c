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
    gchar *last_selection;
    GtkWidget *entry;
    GtkCellRenderer *text_renderer;
};

enum {
    PROP_0, PROP_CLIENT, PROP_LAST_SELECTION
};

enum {
    DEST_DEFAULT, DEST_LABEL, DEST_EXISTING, DEST_USERADD
};

enum {
    DEST_COLUMN_LABEL, DEST_COLUMN_DIR, DEST_COLUMN_TYPE, N_DEST_COLUMNS
};

static void trg_destination_combo_finalize(GObject * object)
{
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(object);
    g_free(priv->last_selection);
}

static void
trg_destination_combo_get_property(GObject * object,
                                   guint property_id,
                                   GValue * value, GParamSpec * pspec)
{
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(object);
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

static void
trg_destination_combo_set_property(GObject * object,
                                   guint property_id,
                                   const GValue * value,
                                   GParamSpec * pspec)
{
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(object);
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

static gboolean g_slist_str_set_add(GSList ** list, const gchar * string)
{
    GSList *li;
    for (li = *list; li; li = g_slist_next(li))
        if (!g_strcmp0((gchar *) li->data, string))
            return FALSE;

    *list = g_slist_insert_sorted(*list, (gpointer) string,
                                  (GCompareFunc) g_strcmp0);

    return TRUE;
}

void trg_destination_combo_save_selection(TrgDestinationCombo * combo_box)
{
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(combo_box);
    GtkTreeIter iter;

    if (priv->last_selection
        && gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo_box), &iter))
    {
        GtkTreeModel *model =
            gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box));
        TrgPrefs *prefs = trg_client_get_prefs(priv->client);
        gchar *text;

        gtk_tree_model_get(model, &iter, DEST_COLUMN_LABEL, &text, -1);
        trg_prefs_set_string(prefs, priv->last_selection, text,
                             TRG_PREFS_CONNECTION);
        g_free(text);
    }
}

static void
gtk_combo_box_entry_active_changed(GtkComboBox * combo_box,
                                   gpointer user_data)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean editableEntry = TRUE;

    if (gtk_combo_box_get_active_iter(combo_box, &iter)) {
        GtkEntry *entry =
            trg_destination_combo_get_entry(TRG_DESTINATION_COMBO
                                            (combo_box));

        if (entry) {
            GValue value = { 0, };
            guint type;

            model = gtk_combo_box_get_model(combo_box);

            gtk_tree_model_get_value(model, &iter, DEST_COLUMN_LABEL,
                                     &value);
            gtk_tree_model_get(model, &iter, DEST_COLUMN_TYPE, &type, -1);

            g_object_set_property(G_OBJECT(entry), "text", &value);
            g_value_unset(&value);

            if (type == DEST_LABEL)
                editableEntry = FALSE;
        }
    }
#if GTK_CHECK_VERSION( 3, 0, 0 )
    gtk_editable_set_editable(GTK_EDITABLE
                              (trg_destination_combo_get_entry
                               (TRG_DESTINATION_COMBO(combo_box))),
                              editableEntry);
#else
    gtk_entry_set_editable(trg_destination_combo_get_entry
                           (TRG_DESTINATION_COMBO(combo_box)),
                           editableEntry);
#endif
}

gboolean trg_destination_combo_has_text(TrgDestinationCombo * combo)
{
    const gchar *text =
        gtk_entry_get_text(trg_destination_combo_get_entry
                           (TRG_DESTINATION_COMBO(combo)));
    return strlen(text) > 0;
}

GtkEntry *trg_destination_combo_get_entry(TrgDestinationCombo * combo)
{
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(combo);
    return GTK_ENTRY(priv->entry);
}

static void
add_entry_cb(GtkEntry * entry,
             GtkEntryIconPosition icon_pos,
             GdkEvent * event, gpointer user_data)
{
    GtkComboBox *combo = GTK_COMBO_BOX(user_data);
    GtkTreeModel *model = gtk_combo_box_get_model(combo);
    GtkTreeIter iter;

    gtk_list_store_insert_with_values(GTK_LIST_STORE(model), &iter,
                                      INT_MAX, DEST_COLUMN_LABEL, "",
                                      DEST_COLUMN_DIR, "",
                                      DEST_COLUMN_TYPE, DEST_USERADD, -1);
    gtk_combo_box_set_active_iter(combo, &iter);
}

struct findDupeArg {
    const gchar *dir;
    gboolean isDupe;
};

gboolean
trg_destination_combo_insert_check_dupe_foreach(GtkTreeModel * model,
                                                GtkTreePath *
                                                path G_GNUC_UNUSED,
                                                GtkTreeIter * iter,
                                                struct findDupeArg *args)
{
    gchar *existing;
    gtk_tree_model_get(model, iter, DEST_COLUMN_DIR, &existing, -1);
    args->isDupe = g_strcmp0(existing, args->dir) == 0;
    g_free(existing);
    return args->isDupe;
}

static void
trg_destination_combo_insert(GtkComboBox * box,
                             const gchar * label,
                             const gchar * dir, guint type,
                             const gchar * lastDestination)
{
    GtkTreeModel *model = gtk_combo_box_get_model(box);
    gchar *comboLabel;
    GtkTreeIter iter;

    if (type == DEST_EXISTING) {
        struct findDupeArg args;
        args.isDupe = FALSE;
        args.dir = dir;
        gtk_tree_model_foreach(GTK_TREE_MODEL(model),
                               (GtkTreeModelForeachFunc)
                               trg_destination_combo_insert_check_dupe_foreach,
                               &args);
        if (args.isDupe)
            return;
    }

    comboLabel =
        label ? g_strdup_printf("%s (%s)", label, dir) : g_strdup(dir);

    gtk_list_store_insert_with_values(GTK_LIST_STORE(model), &iter,
                                      INT_MAX, DEST_COLUMN_LABEL,
                                      comboLabel, DEST_COLUMN_DIR, dir,
                                      DEST_COLUMN_TYPE, type, -1);

    if (lastDestination && !g_strcmp0(lastDestination, comboLabel))
        gtk_combo_box_set_active_iter(box, &iter);

    g_free(comboLabel);
}

static GObject *trg_destination_combo_constructor(GType type,
                                                  guint
                                                  n_construct_properties,
                                                  GObjectConstructParam *
                                                  construct_params)
{
    GObject *object = G_OBJECT_CLASS
        (trg_destination_combo_parent_class)->constructor(type,
                                                          n_construct_properties,
                                                          construct_params);
    TrgDestinationComboPrivate *priv =
        TRG_DESTINATION_COMBO_GET_PRIVATE(object);

    TrgClient *client = priv->client;
    TrgPrefs *prefs = trg_client_get_prefs(client);

    GSList *dirs = NULL, *sli;
    GList *li, *list;
    GtkTreeRowReference *rr;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkListStore *comboModel;
    JsonArray *savedDestinations;
    gchar *defaultDir;
    gchar *lastDestination = NULL;

    comboModel = gtk_list_store_new(N_DEST_COLUMNS, G_TYPE_STRING,
                                    G_TYPE_STRING, G_TYPE_UINT);
    gtk_combo_box_set_model(GTK_COMBO_BOX(object),
                            GTK_TREE_MODEL(comboModel));
    g_object_unref(comboModel);

    g_signal_connect(object, "changed",
                     G_CALLBACK(gtk_combo_box_entry_active_changed), NULL);

    priv->entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(object), priv->entry);

    priv->text_renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(object),
                               priv->text_renderer, TRUE);

    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(object),
                                   priv->text_renderer, "text", 0, NULL);

    g_slist_foreach(dirs, (GFunc) g_free, NULL);
    g_slist_free(dirs);

    gtk_entry_set_icon_from_stock(GTK_ENTRY(priv->entry),
                                  GTK_ENTRY_ICON_SECONDARY,
                                  GTK_STOCK_CLEAR);

    g_signal_connect(priv->entry, "icon-release",
                     G_CALLBACK(add_entry_cb), object);

    defaultDir =
        g_strdup(session_get_download_dir(trg_client_get_session(client)));
    rm_trailing_slashes(defaultDir);

    savedDestinations =
        trg_prefs_get_array(prefs, TRG_PREFS_KEY_DESTINATIONS,
                            TRG_PREFS_CONNECTION);

    if (priv->last_selection)
        lastDestination = trg_prefs_get_string(prefs, priv->last_selection,
                                               TRG_PREFS_CONNECTION);

    trg_destination_combo_insert(GTK_COMBO_BOX(object),
                                 NULL,
                                 defaultDir, DEST_DEFAULT,
                                 lastDestination);
    gtk_combo_box_set_active(GTK_COMBO_BOX(object), 0);

    if (savedDestinations) {
        list = json_array_get_elements(savedDestinations);
        if (list) {
            for (li = list; li; li = g_list_next(li)) {
                JsonObject *obj =
                    json_node_get_object((JsonNode *) li->data);
                trg_destination_combo_insert(GTK_COMBO_BOX(object),
                                             json_object_get_string_member
                                             (obj, TRG_PREFS_SUBKEY_LABEL),
                                             json_object_get_string_member
                                             (obj,
                                              TRG_PREFS_KEY_DESTINATIONS_SUBKEY_DIR),
                                             DEST_LABEL, lastDestination);
            }
            g_list_free(list);
        }
    }

    list = g_hash_table_get_values(trg_client_get_torrent_table(client));
    for (li = list; li; li = g_list_next(li)) {
        rr = (GtkTreeRowReference *) li->data;
        model = gtk_tree_row_reference_get_model(rr);
        path = gtk_tree_row_reference_get_path(rr);

        if (path) {
            GtkTreeIter iter;

            if (gtk_tree_model_get_iter(model, &iter, path)) {
                gchar *dd;

                gtk_tree_model_get(model, &iter,
                                   TORRENT_COLUMN_DOWNLOADDIR, &dd, -1);

                if (dd && g_strcmp0(dd, defaultDir))
                    g_slist_str_set_add(&dirs, dd);
                else
                    g_free(dd);
            }

            gtk_tree_path_free(path);
        }
    }

    for (sli = dirs; sli; sli = g_slist_next(sli))
        trg_destination_combo_insert(GTK_COMBO_BOX(object),
                                     NULL,
                                     (gchar *) sli->data,
                                     DEST_EXISTING, lastDestination);

    g_list_free(list);
    g_free(defaultDir);
    g_free(lastDestination);

    return object;
}

gchar *trg_destination_combo_get_dir(TrgDestinationCombo * combo)
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

    return
        g_strdup(gtk_entry_get_text
                 (trg_destination_combo_get_entry(combo)));
}

static void
trg_destination_combo_class_init(TrgDestinationComboClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgDestinationComboPrivate));

    object_class->get_property = trg_destination_combo_get_property;
    object_class->set_property = trg_destination_combo_set_property;
    object_class->finalize = trg_destination_combo_finalize;
    object_class->constructor = trg_destination_combo_constructor;

    g_object_class_install_property(object_class,
                                    PROP_CLIENT,
                                    g_param_spec_pointer("trg-client",
                                                         "TClient",
                                                         "Client",
                                                         G_PARAM_READWRITE
                                                         |
                                                         G_PARAM_CONSTRUCT_ONLY
                                                         |
                                                         G_PARAM_STATIC_NAME
                                                         |
                                                         G_PARAM_STATIC_NICK
                                                         |
                                                         G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class,
                                    PROP_LAST_SELECTION,
                                    g_param_spec_string
                                    ("last-selection-key",
                                     "LastSelectionKey",
                                     "LastSelectionKey", NULL,
                                     G_PARAM_READWRITE |
                                     G_PARAM_CONSTRUCT_ONLY |
                                     G_PARAM_STATIC_NAME |
                                     G_PARAM_STATIC_NICK |
                                     G_PARAM_STATIC_BLURB));
}

static void trg_destination_combo_init(TrgDestinationCombo * self)
{
}

GtkWidget *trg_destination_combo_new(TrgClient * client,
                                     const gchar * lastSelectionKey)
{
    return GTK_WIDGET(g_object_new(TRG_TYPE_DESTINATION_COMBO,
                                   "trg-client", client,
                                   "last-selection-key", lastSelectionKey,
                                   NULL));
}
