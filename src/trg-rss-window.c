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

#if HAVE_RSS

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <rss-glib/rss-glib.h>

#include "trg-rss-window.h"
#include "trg-rss-model.h"
#include "trg-rss-cell-renderer.h"
#include "trg-torrent-add-dialog.h"
#include "trg-preferences-dialog.h"
#include "trg-client.h"
#include "upload.h"
#include "util.h"

G_DEFINE_TYPE(TrgRssWindow, trg_rss_window,
              GTK_TYPE_WINDOW)
#define TRG_RSS_WINDOW_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_RSS_WINDOW, TrgRssWindowPrivate))
enum {
	PROP_0, PROP_PARENT, PROP_CLIENT
};

typedef struct _TrgRssWindowPrivate TrgRssWindowPrivate;

struct _TrgRssWindowPrivate {
    TrgMainWindow *parent;
    TrgClient *client;
    GtkTreeView *tree_view;
    TrgRssModel *tree_model;
};

static GObject *instance = NULL;

static void
trg_rss_window_get_property(GObject * object,
                                     guint property_id,
                                     GValue * value, GParamSpec * pspec)
{
    TrgRssWindowPrivate *priv =
        TRG_RSS_WINDOW_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    case PROP_PARENT:
        g_value_set_object(value, priv->parent);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_rss_window_set_property(GObject * object,
                                     guint property_id,
                                     const GValue * value,
                                     GParamSpec * pspec)
{
    TrgRssWindowPrivate *priv =
        TRG_RSS_WINDOW_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_PARENT:
        priv->parent = g_value_get_object(value);
        break;
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static gboolean upload_complete_searchfunc(GtkTreeModel *model,
                                                         GtkTreePath *path,
                                                         GtkTreeIter *iter,
                                                         gpointer data) {
	trg_upload *upload = (trg_upload*)data;
	gchar *item_guid = NULL;
	gboolean stop = FALSE;

	gtk_tree_model_get(model, iter, RSSCOL_ID, &item_guid, -1);

	if (!g_strcmp0(item_guid, upload->uid)) {
		gtk_list_store_set(GTK_LIST_STORE(model), iter, RSSCOL_UPLOADED, TRUE, -1);
		stop = TRUE;
	}

	g_free(item_guid);

	return stop;
}

static gboolean on_upload_complete(gpointer data) {
	trg_response *response = (trg_response*)data;
	trg_upload *upload = (trg_upload*)response->cb_data;
	TrgRssWindowPrivate *priv = TRG_RSS_WINDOW_GET_PRIVATE(upload->cb_data);

	if (response->status == CURLE_OK)
		gtk_tree_model_foreach(GTK_TREE_MODEL(priv->tree_model), upload_complete_searchfunc, upload);

	return FALSE;
}

static gboolean on_torrent_receive(gpointer data) {
	trg_response *response = (trg_response *) data;
	trg_upload *upload = (trg_upload*)response->cb_data;
	TrgRssWindowPrivate *priv = TRG_RSS_WINDOW_GET_PRIVATE(upload->cb_data);
	TrgClient *client = priv->client;
	TrgPrefs *prefs = trg_client_get_prefs(client);
	TrgMainWindow *main_win = priv->parent;

	upload->upload_response = response;

	if (response->status == CURLE_OK) {
	    if (trg_prefs_get_bool(prefs, TRG_PREFS_KEY_ADD_OPTIONS_DIALOG,
	                           TRG_PREFS_GLOBAL)) {
	        TrgTorrentAddDialog *dialog =
	            trg_torrent_add_dialog_new_from_upload(main_win, client,
	                                       upload);
	        gtk_widget_show_all(GTK_WIDGET(dialog));
	    } else {
	    	trg_do_upload(upload);
	    }
	} else {
		trg_error_dialog(GTK_WINDOW(main_win), response);
		trg_upload_free(upload);
	}

	return FALSE;
}

static void
rss_item_activated(GtkTreeView * treeview,
                          GtkTreePath * path,
                          GtkTreeViewColumn *
                          col G_GNUC_UNUSED, gpointer userdata)
{
	TrgRssWindow *win = TRG_RSS_WINDOW(userdata);
	TrgRssWindowPrivate *priv = TRG_RSS_WINDOW_GET_PRIVATE(win);
	TrgClient *client = priv->client;
	TrgPrefs *prefs = trg_client_get_prefs(client);
	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	trg_upload *upload = g_new0(trg_upload, 1);
	GtkTreeIter iter;
	gchar *link, *uid, *cookie;

    //upload->upload_response = response;
    upload->main_window = priv->parent;
    upload->client = client;
    upload->extra_args = FALSE;
    upload->flags = trg_prefs_get_add_flags(prefs);
    upload->callback = on_upload_complete;
    upload->cb_data = win;

	gtk_tree_model_get_iter(model, &iter, path);

	gtk_tree_model_get(model, &iter, RSSCOL_LINK, &link, RSSCOL_ID, &uid, RSSCOL_COOKIE, &cookie, -1);

	upload->uid = uid;

	async_http_request(priv->client, link, cookie, on_torrent_receive, upload);

	g_free(cookie);
	g_free(link);
}

static void trg_rss_on_get_error(TrgRssModel *model, rss_get_error *error, gpointer data) {
	GtkWindow *win = GTK_WINDOW(data);
	gchar *msg;
	if (error->error_code <= -100) {
		msg = g_strdup_printf(_("Request failed with HTTP code %d"), -(error->error_code + 100));
	} else {
		msg = g_strdup(curl_easy_strerror(error->error_code));
	}
    GtkWidget *dialog = gtk_message_dialog_new(win,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_OK,
                                               "%s", msg);
    g_free(msg);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void trg_rss_on_parse_error(TrgRssModel *model, rss_parse_error *error, gpointer data) {
	GtkWindow *win = GTK_WINDOW(data);
	gchar *msg = g_strdup_printf(_("Error parsing RSS feed \"%s\": %s"), error->feed_id, error->error->message);
    GtkWidget *dialog = gtk_message_dialog_new(win,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_OK,
                                               "%s", msg);
    g_free(msg);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void on_configure(GtkWidget *widget, gpointer data) {
	TrgRssWindowPrivate *priv = TRG_RSS_WINDOW_GET_PRIVATE(data);
	GtkWidget *dlg = trg_preferences_dialog_get_instance(priv->parent, priv->client);
	gtk_widget_show_all(dlg);
	trg_preferences_dialog_set_page(TRG_PREFERENCES_DIALOG(dlg), 5);
}

static void on_refresh(GtkWidget *widget, gpointer data) {
	TrgRssWindowPrivate *priv = TRG_RSS_WINDOW_GET_PRIVATE(data);
	trg_rss_model_update(priv->tree_model);
}

static GObject *trg_rss_window_constructor(GType type,
                                                    guint
                                                    n_construct_properties,
                                                    GObjectConstructParam *
                                                    construct_params)
{
    GObject *object;
    TrgRssWindowPrivate *priv;
    GtkWidget *vbox, *img;
    GtkToolItem *item;
    GtkWidget *toolbar;

    object = G_OBJECT_CLASS
        (trg_rss_window_parent_class)->constructor(type,
                                                            n_construct_properties,
                                                            construct_params);
    priv = TRG_RSS_WINDOW_GET_PRIVATE(object);

    priv->tree_model = trg_rss_model_new(priv->client);

    g_signal_connect(priv->tree_model, "get-error",
                      G_CALLBACK(trg_rss_on_get_error), object);
    g_signal_connect(priv->tree_model, "parse-error",
                      G_CALLBACK(trg_rss_on_parse_error), object);

    trg_rss_model_update(priv->tree_model);

    priv->tree_view = GTK_TREE_VIEW(gtk_tree_view_new());
    gtk_tree_view_set_headers_visible(priv->tree_view, FALSE);
    gtk_tree_view_set_model(priv->tree_view, GTK_TREE_MODEL(priv->tree_model));

    gtk_tree_view_insert_column_with_attributes(priv->tree_view, -1, NULL, trg_rss_cell_renderer_new(), "title", RSSCOL_TITLE, "feed", RSSCOL_FEED, "published", RSSCOL_PUBDATE, "uploaded", RSSCOL_UPLOADED, NULL);

    g_signal_connect(priv->tree_view, "row-activated",
                      G_CALLBACK(rss_item_activated), object);

    gtk_window_set_title(GTK_WINDOW(object), _("RSS Feeds"));

    toolbar = gtk_toolbar_new();


    img = gtk_image_new_from_icon_name("view-refresh",
                                       GTK_ICON_SIZE_LARGE_TOOLBAR);
    item = gtk_tool_button_new(img, "Refresh");
    gtk_widget_set_sensitive(GTK_WIDGET(item), TRUE);
    gtk_tool_item_set_tooltip_text(item, "Refresh");
    g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(on_refresh), object);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, 0);

    img = gtk_image_new_from_icon_name("preferences-system",
                                       GTK_ICON_SIZE_LARGE_TOOLBAR);
    item = gtk_tool_button_new(img, "Refresh");
    gtk_widget_set_sensitive(GTK_WIDGET(item), TRUE);
    gtk_tool_item_set_tooltip_text(item, "Configure Feeds");
    g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(on_configure), object);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, 0);

    vbox = trg_vbox_new(FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(toolbar),
                           FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), my_scrolledwin_new(GTK_WIDGET(priv->tree_view)),
                           TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(object), vbox);

    /*g_signal_connect(object, "response",
                     G_CALLBACK(trg_rss_window_response_cb), NULL);*/

    gtk_widget_set_size_request(GTK_WIDGET(object), 500, 300);

    return object;
}

static void trg_rss_window_dispose(GObject * object)
{
	instance = NULL;
    G_OBJECT_CLASS(trg_rss_window_parent_class)->dispose(object);
}

static void
trg_rss_window_class_init(TrgRssWindowClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgRssWindowPrivate));

    object_class->constructor = trg_rss_window_constructor;
    object_class->get_property = trg_rss_window_get_property;
    object_class->set_property = trg_rss_window_set_property;
    object_class->dispose = trg_rss_window_dispose;

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
                                    PROP_PARENT,
                                    g_param_spec_object("parent-window",
                                                        "Parent window",
                                                        "Parent window",
                                                        TRG_TYPE_MAIN_WINDOW,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY
                                                        |
                                                        G_PARAM_STATIC_NAME
                                                        |
                                                        G_PARAM_STATIC_NICK
                                                        |
                                                        G_PARAM_STATIC_BLURB));
}

static void
trg_rss_window_init(TrgRssWindow * self G_GNUC_UNUSED)
{
}

TrgRssWindow *trg_rss_window_get_instance(TrgMainWindow *parent, TrgClient *client)
{
    if (instance == NULL) {
        instance =
            g_object_new(TRG_TYPE_RSS_WINDOW, "parent-window", parent, "trg-client", client, NULL);
    }

    return TRG_RSS_WINDOW(instance);
}

#endif
