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
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <pango/pango.h>

#include "trg-prefs.h"
#include "trg-main-window.h"
#include "trg-status-bar.h"
#include "trg-torrent-model.h"
#include "session-get.h"
#include "requests.h"
#include "json.h"
#include "util.h"

/* A subclass of GtkHBox which contains a status label on the left.
 * Free space indicator on left-right.
 * Speed (including limits if in use) label on right-right.
 *
 * Status and speed labels should be updated on every torrent-get using
 * trg_status_bar_update. Free space is updated with trg_status_bar_session_update.
 *
 * There's a signal in TrgClient for session updates, connected into the
 * main window, which calls this. Session updates happen every 10 torrent-get updates.
 */

G_DEFINE_TYPE(TrgStatusBar, trg_status_bar, GTK_TYPE_BOX)
#define TRG_STATUS_BAR_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_STATUS_BAR, TrgStatusBarPrivate))
typedef struct _TrgStatusBarPrivate TrgStatusBarPrivate;

struct _TrgStatusBarPrivate {
    GtkWidget *speed_lbl;
    GtkWidget *turtleImage, *turtleEventBox;
    GtkWidget *free_lbl;
    GtkWidget *info_lbl;
    TrgClient *client;
    TrgMainWindow *win;
};

static void trg_status_bar_class_init(TrgStatusBarClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgStatusBarPrivate));
}

void trg_status_bar_clear_indicators(TrgStatusBar * sb)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(sb);
    gtk_label_set_text(GTK_LABEL(priv->free_lbl), "");
    gtk_label_set_text(GTK_LABEL(priv->speed_lbl), "");
}

void trg_status_bar_reset(TrgStatusBar * sb)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(sb);
    trg_status_bar_clear_indicators(sb);
    gtk_label_set_text(GTK_LABEL(priv->info_lbl), _("Disconnected"));
    gtk_widget_set_visible(priv->turtleEventBox, FALSE);
}

static void
turtle_toggle(GtkWidget * w, GdkEventButton * event, gpointer data)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(data);
    JsonNode *req = session_set();
    JsonObject *args = node_get_arguments(req);
    const gchar *iconName;
    gboolean altSpeedOn;

    gtk_image_get_icon_name(GTK_IMAGE(priv->turtleImage), &iconName, NULL);
    altSpeedOn = g_strcmp0(iconName, "alt-speed-on") == 0;

    gtk_image_set_from_icon_name(GTK_IMAGE(priv->turtleImage),
                                 altSpeedOn ? "alt-speed-off" : "alt-speed-on",
                                 GTK_ICON_SIZE_SMALL_TOOLBAR);
    json_object_set_boolean_member(args, SGET_ALT_SPEED_ENABLED,
                                   !altSpeedOn);

    dispatch_async(priv->client, req, on_session_set, priv->win);
}

static void trg_status_bar_init(TrgStatusBar * self)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(self);
    gtk_container_set_border_width(GTK_CONTAINER(self), 2);

    priv->info_lbl = gtk_label_new(_("Disconnected"));
    gtk_label_set_ellipsize(GTK_LABEL(priv->info_lbl),
                            PANGO_ELLIPSIZE_END);
    gtk_box_pack_start(GTK_BOX(self), priv->info_lbl, FALSE, TRUE, 0);

    priv->turtleImage = gtk_image_new();

    priv->turtleEventBox = gtk_event_box_new();
    g_signal_connect(priv->turtleEventBox, "button-press-event",
                     G_CALLBACK(turtle_toggle), self);
    gtk_widget_set_visible(priv->turtleEventBox, FALSE);
    gtk_container_add(GTK_CONTAINER(priv->turtleEventBox),
                      priv->turtleImage);
    gtk_box_pack_end(GTK_BOX(self), priv->turtleEventBox, FALSE, TRUE, 5);

    priv->speed_lbl = gtk_label_new(NULL);
    gtk_label_set_ellipsize(GTK_LABEL(priv->speed_lbl),
                            PANGO_ELLIPSIZE_START);
    gtk_box_pack_end(GTK_BOX(self), priv->speed_lbl, FALSE, TRUE, 5);

    priv->free_lbl = gtk_label_new(NULL);
    gtk_box_pack_end(GTK_BOX(self), priv->free_lbl, FALSE, TRUE, 5);
}

void
trg_status_bar_push_connection_msg(TrgStatusBar * sb, const gchar * msg)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(sb);
    gtk_label_set_text(GTK_LABEL(priv->info_lbl), msg);
}

static void
trg_status_bar_set_connected_label(TrgStatusBar * sb, JsonObject * session,
                                   TrgClient * client)
{
    TrgPrefs *prefs = trg_client_get_prefs(client);

    gchar *profileName = trg_prefs_get_string(prefs,
                                              TRG_PREFS_KEY_PROFILE_NAME,
                                              TRG_PREFS_CONNECTION);
    gchar *statusMsg =
        g_strdup_printf(_("Connected: %s :: %s"),
                        profileName,
                        session_get_version_string(session));

    trg_status_bar_push_connection_msg(sb, statusMsg);

    g_free(profileName);
    g_free(statusMsg);
}

void
trg_status_bar_connect(TrgStatusBar * sb, JsonObject * session,
                       TrgClient * client)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(sb);

    trg_status_bar_set_connected_label(sb, session, client);
    gtk_label_set_text(GTK_LABEL(priv->speed_lbl),
                       _("Updating torrents..."));
}

void trg_status_bar_session_update(TrgStatusBar * sb, JsonObject * session)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(sb);
    gint64 free = session_get_download_dir_free_space(session);
    gboolean altSpeedEnabled = session_get_alt_speed_enabled(session);
    gchar freeSpace[64];

    if (free >= 0) {
        gchar *freeSpaceString;
        trg_strlsize(freeSpace, free);
        freeSpaceString = g_strdup_printf(_("Free space: %s"), freeSpace);
        gtk_label_set_text(GTK_LABEL(priv->free_lbl), freeSpaceString);
        g_free(freeSpaceString);
    } else {
        gtk_label_set_text(GTK_LABEL(priv->free_lbl), "");
    }

    gtk_image_set_from_icon_name(GTK_IMAGE(priv->turtleImage),
                                 altSpeedEnabled ? "alt-speed-on" :
                                 "alt-speed-off", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_set_tooltip_text(priv->turtleImage,
                                altSpeedEnabled ?
                                _("Disable alternate speed limits") :
                                _("Enable alternate speed limits"));
    gtk_widget_set_visible(priv->turtleEventBox, TRUE);
}

void
trg_status_bar_update_speed(TrgStatusBar * sb,
                            trg_torrent_model_update_stats * stats,
                            TrgClient * client)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(sb);
    JsonObject *session = trg_client_get_session(client);
    gboolean altLimits = session_get_speed_limit_alt_enabled(session);
    gchar *speedText;
    gint64 uplimitraw, downlimitraw;
    gchar downRateTotalString[32], upRateTotalString[32];
    gchar uplimit[64], downlimit[64];

    if (altLimits) {
        downlimitraw = session_get_alt_speed_limit_down(session);
        uplimitraw = session_get_alt_speed_limit_up(session);
    } else {
        if (session_get_speed_limit_down_enabled(session))
            downlimitraw = session_get_speed_limit_down(session);
        else
            downlimitraw = -1;
        if (session_get_speed_limit_up_enabled(session))
            uplimitraw = session_get_speed_limit_up(session);
        else
            uplimitraw = -1;
    }

    trg_strlspeed(downRateTotalString, stats->downRateTotal / disk_K);
    trg_strlspeed(upRateTotalString, stats->upRateTotal / disk_K);

    if (uplimitraw >= 0) {
        gchar uplimitstring[32];
        trg_strlspeed(uplimitstring, uplimitraw);
        g_snprintf(uplimit, sizeof(uplimit), _(" (Limit: %s)"),
                   uplimitstring);
    }

    if (downlimitraw >= 0) {
        gchar downlimitstring[32];
        trg_strlspeed(downlimitstring, downlimitraw);
        g_snprintf(downlimit, sizeof(downlimit), _(" (Limit: %s)"),
                   downlimitstring);
    }

    speedText =
        g_strdup_printf(_("Down: %s%s, Up: %s%s"), downRateTotalString,
                        downlimitraw >= 0 ? downlimit : "",
                        upRateTotalString, uplimitraw >= 0 ? uplimit : "");

    gtk_label_set_text(GTK_LABEL(priv->speed_lbl), speedText);

    g_free(speedText);
}

void
trg_status_bar_update(TrgStatusBar * sb,
                      trg_torrent_model_update_stats * stats,
                      TrgClient * client)
{
    trg_status_bar_set_connected_label(sb, trg_client_get_session(client),
                                       client);
    trg_status_bar_update_speed(sb, stats, client);
}

const gchar *trg_status_bar_get_speed_text(TrgStatusBar * s)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(s);
    return gtk_label_get_text(GTK_LABEL(priv->speed_lbl));
}

TrgStatusBar *trg_status_bar_new(TrgMainWindow * win, TrgClient * client)
{
    TrgStatusBar *sb = g_object_new(TRG_TYPE_STATUS_BAR,
                                    "orientation",
                                    GTK_ORIENTATION_HORIZONTAL,
                                    NULL);
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(sb);

    priv->client = client;
    priv->win = win;

    return sb;
}
