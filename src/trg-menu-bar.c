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

#include <gtk/gtk.h>
#include "trg-main-window.h"
#include "trg-menu-bar.h"

enum {
    PROP_0,
    PROP_CONNECT_BUTTON,
    PROP_DISCONNECT_BUTTON,
    PROP_ADD_BUTTON,
    PROP_ADD_URL_BUTTON,
    PROP_REMOVE_BUTTON,
    PROP_DELETE_BUTTON,
    PROP_RESUME_BUTTON,
    PROP_PAUSE_BUTTON,
    PROP_VERIFY_BUTTON,
    PROP_PROPS_BUTTON,
    PROP_MOVE_BUTTON,
    PROP_REMOTE_PREFS_BUTTON,
    PROP_LOCAL_PREFS_BUTTON,
    PROP_ABOUT_BUTTON,
    PROP_VIEW_STATS_BUTTON,
    PROP_VIEW_STATES_BUTTON,
    PROP_VIEW_NOTEBOOK_BUTTON,
    PROP_QUIT
};

G_DEFINE_TYPE(TrgMenuBar, trg_menu_bar, GTK_TYPE_MENU_BAR)
#define TRG_MENU_BAR_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_MENU_BAR, TrgMenuBarPrivate))
typedef struct _TrgMenuBarPrivate TrgMenuBarPrivate;

struct _TrgMenuBarPrivate {
    GtkWidget *mb_connect;
    GtkWidget *mb_disconnect;
    GtkWidget *mb_add;
    GtkWidget *mb_add_url;
    GtkWidget *mb_move;
    GtkWidget *mb_remove;
    GtkWidget *mb_delete;
    GtkWidget *mb_resume;
    GtkWidget *mb_pause;
    GtkWidget *mb_verify;
    GtkWidget *mb_props;
    GtkWidget *mb_local_prefs;
    GtkWidget *mb_remote_prefs;
    GtkWidget *mb_view_states;
    GtkWidget *mb_view_notebook;
    GtkWidget *mb_view_stats;
    GtkWidget *mb_about;
    GtkWidget *mb_quit;
};

void trg_menu_bar_connected_change(TrgMenuBar * mb, gboolean connected)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(mb);

    gtk_widget_set_sensitive(priv->mb_add, connected);
    gtk_widget_set_sensitive(priv->mb_add_url, connected);
    gtk_widget_set_sensitive(priv->mb_connect, !connected);
    gtk_widget_set_sensitive(priv->mb_disconnect, connected);
    gtk_widget_set_sensitive(priv->mb_remote_prefs, connected);
    gtk_widget_set_sensitive(priv->mb_view_stats, connected);
}

void trg_menu_bar_torrent_actions_sensitive(TrgMenuBar * mb,
					    gboolean sensitive)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(mb);

    gtk_widget_set_sensitive(priv->mb_props, sensitive);
    gtk_widget_set_sensitive(priv->mb_remove, sensitive);
    gtk_widget_set_sensitive(priv->mb_delete, sensitive);
    gtk_widget_set_sensitive(priv->mb_resume, sensitive);
    gtk_widget_set_sensitive(priv->mb_pause, sensitive);
    gtk_widget_set_sensitive(priv->mb_verify, sensitive);
    gtk_widget_set_sensitive(priv->mb_move, sensitive);
}

static void
trg_menu_bar_get_property(GObject * object, guint property_id,
			  GValue * value, GParamSpec * pspec)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_CONNECT_BUTTON:
	g_value_set_object(value, priv->mb_connect);
	break;
    case PROP_DISCONNECT_BUTTON:
	g_value_set_object(value, priv->mb_disconnect);
	break;
    case PROP_ADD_BUTTON:
	g_value_set_object(value, priv->mb_add);
	break;
    case PROP_ADD_URL_BUTTON:
	g_value_set_object(value, priv->mb_add_url);
	break;
    case PROP_REMOVE_BUTTON:
	g_value_set_object(value, priv->mb_remove);
	break;
    case PROP_DELETE_BUTTON:
	g_value_set_object(value, priv->mb_delete);
	break;
    case PROP_MOVE_BUTTON:
	g_value_set_object(value, priv->mb_move);
	break;
    case PROP_RESUME_BUTTON:
	g_value_set_object(value, priv->mb_resume);
	break;
    case PROP_PAUSE_BUTTON:
	g_value_set_object(value, priv->mb_pause);
	break;
    case PROP_VERIFY_BUTTON:
	g_value_set_object(value, priv->mb_verify);
	break;
    case PROP_PROPS_BUTTON:
	g_value_set_object(value, priv->mb_props);
	break;
    case PROP_REMOTE_PREFS_BUTTON:
	g_value_set_object(value, priv->mb_remote_prefs);
	break;
    case PROP_LOCAL_PREFS_BUTTON:
	g_value_set_object(value, priv->mb_local_prefs);
	break;
    case PROP_ABOUT_BUTTON:
	g_value_set_object(value, priv->mb_about);
	break;
    case PROP_VIEW_STATES_BUTTON:
	g_value_set_object(value, priv->mb_view_states);
	break;
    case PROP_VIEW_NOTEBOOK_BUTTON:
	g_value_set_object(value, priv->mb_view_notebook);
	break;
    case PROP_VIEW_STATS_BUTTON:
	g_value_set_object(value, priv->mb_view_stats);
	break;
    case PROP_QUIT:
	g_value_set_object(value, priv->mb_quit);
	break;
    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
trg_menu_bar_install_widget_prop(GObjectClass * class, guint propId,
				 const gchar * name, const gchar * nick)
{
    g_object_class_install_property(class,
				    propId,
				    g_param_spec_object(name,
							nick,
							nick,
							GTK_TYPE_WIDGET,
							G_PARAM_READABLE
							|
							G_PARAM_STATIC_NAME
							|
							G_PARAM_STATIC_NICK
							|
							G_PARAM_STATIC_BLURB));
}

static
GtkWidget *trg_menu_bar_item_new(GtkMenuShell * shell, char *text,
				 char *stock_id, gboolean sensitive)
{
    GtkWidget *item = gtk_image_menu_item_new_with_label(stock_id);
    gtk_image_menu_item_set_use_stock(GTK_IMAGE_MENU_ITEM(item), TRUE);

    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM
					      (item), TRUE);
    gtk_menu_item_set_label(GTK_MENU_ITEM(item), text);
    gtk_widget_set_sensitive(item, sensitive);

    gtk_menu_shell_append(shell, item);

    return item;
}

static GtkWidget *trg_menu_bar_view_menu_new(TrgMenuBarPrivate * priv)
{
    GtkWidget *view = gtk_menu_item_new_with_label("View");
    GtkWidget *viewMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), viewMenu);

    priv->mb_view_states =
	gtk_check_menu_item_new_with_label("State selector");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM
				   (priv->mb_view_states), TRUE);

    priv->mb_view_notebook =
	gtk_check_menu_item_new_with_label("Torrent details");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM
				   (priv->mb_view_notebook), TRUE);

    priv->mb_view_stats = gtk_menu_item_new_with_label("Statistics");

    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), priv->mb_view_states);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu),
			  priv->mb_view_notebook);
    gtk_menu_shell_append(GTK_MENU_SHELL(viewMenu), priv->mb_view_stats);

    return view;
}

static
GtkWidget *trg_menu_bar_options_menu_new(TrgMenuBarPrivate * priv)
{
    GtkWidget *opts = gtk_menu_item_new_with_label("Options");
    GtkWidget *optsMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(opts), optsMenu);

    priv->mb_local_prefs =
	trg_menu_bar_item_new(GTK_MENU_SHELL(optsMenu),
			      "Local Preferences",
			      GTK_STOCK_PREFERENCES, TRUE);

    priv->mb_remote_prefs =
	trg_menu_bar_item_new(GTK_MENU_SHELL(optsMenu),
			      "Remote Preferences",
			      GTK_STOCK_NETWORK, FALSE);

    return opts;
}

static
GtkWidget *trg_menu_bar_file_file_menu_new(TrgMenuBarPrivate * priv)
{
    GtkWidget *file = gtk_menu_item_new_with_label("File");
    GtkWidget *fileMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), fileMenu);

    priv->mb_connect =
	trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), "Connect",
			      GTK_STOCK_CONNECT, TRUE);
    priv->mb_disconnect =
	trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), "Disconnect",
			      GTK_STOCK_DISCONNECT, FALSE);
    priv->mb_add =
	trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), "Add",
			      GTK_STOCK_ADD, FALSE);
    priv->mb_add_url =
	trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), "Add from URL",
			      GTK_STOCK_ADD, FALSE);

    priv->mb_quit = trg_menu_bar_item_new(GTK_MENU_SHELL(fileMenu), "Quit",
					  GTK_STOCK_QUIT, TRUE);

    return file;
}

static
GtkWidget *trg_menu_bar_torrent_menu_new(TrgMenuBarPrivate * priv)
{
    GtkWidget *torrent = gtk_menu_item_new_with_label("Torrent");
    GtkWidget *torrentMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(torrent), torrentMenu);

    priv->mb_props =
	trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
			      "Properties", GTK_STOCK_PROPERTIES, FALSE);
    priv->mb_resume =
	trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), "Resume",
			      GTK_STOCK_MEDIA_PLAY, FALSE);
    priv->mb_pause =
	trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), "Pause",
			      GTK_STOCK_MEDIA_PAUSE, FALSE);
    priv->mb_verify =
	trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), "Verify",
			      GTK_STOCK_REFRESH, FALSE);
    priv->mb_move =
	trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), "Move",
			      GTK_STOCK_HARDDISK, FALSE);
    priv->mb_remove =
	trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu), "Remove",
			      GTK_STOCK_REMOVE, FALSE);
    priv->mb_delete =
	trg_menu_bar_item_new(GTK_MENU_SHELL(torrentMenu),
			      "Remove and Delete", GTK_STOCK_DELETE,
			      FALSE);

    return torrent;
}

static
GtkWidget *trg_menu_bar_help_menu_new(TrgMenuBar * menuBar)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(menuBar);

    GtkWidget *help = gtk_menu_item_new_with_label("Help");
    GtkWidget *helpMenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), help);

    priv->mb_about =
	trg_menu_bar_item_new(GTK_MENU_SHELL(helpMenu), "About",
			      GTK_STOCK_ABOUT, TRUE);

    return helpMenu;
}

static void trg_menu_bar_class_init(TrgMenuBarClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = trg_menu_bar_get_property;

    g_type_class_add_private(klass, sizeof(TrgMenuBarPrivate));

    trg_menu_bar_install_widget_prop(object_class, PROP_CONNECT_BUTTON,
				     "connect-button", "Connect Button");
    trg_menu_bar_install_widget_prop(object_class,
				     PROP_DISCONNECT_BUTTON,
				     "disconnect-button",
				     "Disconnect Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_ADD_BUTTON,
				     "add-button", "Add Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_ADD_URL_BUTTON,
				     "add-url-button", "Add URL Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_REMOVE_BUTTON,
				     "remove-button", "Remove Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_MOVE_BUTTON,
				     "move-button", "Move Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_DELETE_BUTTON,
				     "delete-button", "Delete Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_RESUME_BUTTON,
				     "resume-button", "Resume Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_VERIFY_BUTTON,
				     "verify-button", "Verify Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_PAUSE_BUTTON,
				     "pause-button", "Pause Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_PROPS_BUTTON,
				     "props-button", "Props Button");
    trg_menu_bar_install_widget_prop(object_class, PROP_ABOUT_BUTTON,
				     "about-button", "About Button");
    trg_menu_bar_install_widget_prop(object_class,
				     PROP_VIEW_STATS_BUTTON,
				     "view-stats-button",
				     "View stats button");
    trg_menu_bar_install_widget_prop(object_class,
				     PROP_VIEW_STATES_BUTTON,
				     "view-states-button",
				     "View states Button");
    trg_menu_bar_install_widget_prop(object_class,
				     PROP_VIEW_NOTEBOOK_BUTTON,
				     "view-notebook-button",
				     "View notebook Button");
    trg_menu_bar_install_widget_prop(object_class,
				     PROP_REMOTE_PREFS_BUTTON,
				     "remote-prefs-button",
				     "Remote Prefs Button");
    trg_menu_bar_install_widget_prop(object_class,
				     PROP_LOCAL_PREFS_BUTTON,
				     "local-prefs-button",
				     "Local Prefs Button");
    trg_menu_bar_install_widget_prop(object_class,
				     PROP_QUIT,
				     "quit-button", "Quit Button");
}

static void trg_menu_bar_init(TrgMenuBar * self)
{
    TrgMenuBarPrivate *priv = TRG_MENU_BAR_GET_PRIVATE(self);

    gtk_menu_shell_append(GTK_MENU_SHELL(self),
			  trg_menu_bar_file_file_menu_new(priv));
    gtk_menu_shell_append(GTK_MENU_SHELL(self),
			  trg_menu_bar_torrent_menu_new(priv));
    gtk_menu_shell_append(GTK_MENU_SHELL(self),
			  trg_menu_bar_options_menu_new(priv));
    gtk_menu_shell_append(GTK_MENU_SHELL(self),
			  trg_menu_bar_view_menu_new(priv));
    trg_menu_bar_help_menu_new(TRG_MENU_BAR(self));
}

TrgMenuBar *trg_menu_bar_new(TrgMainWindow * win G_GNUC_UNUSED)
{
    GObject *obj = g_object_new(TRG_TYPE_MENU_BAR, NULL);
    return TRG_MENU_BAR(obj);
}
