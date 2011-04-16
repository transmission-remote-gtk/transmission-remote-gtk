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
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include "trg-status-bar.h"
#include "trg-torrent-model.h"
#include "session-get.h"
#include "util.h"

G_DEFINE_TYPE(TrgStatusBar, trg_status_bar, GTK_TYPE_STATUSBAR)
#define TRG_STATUS_BAR_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_STATUS_BAR, TrgStatusBarPrivate))
typedef struct _TrgStatusBarPrivate TrgStatusBarPrivate;

struct _TrgStatusBarPrivate {
    guint connectionCtx;
    guint countSpeedsCtx;
};

static void trg_status_bar_class_init(TrgStatusBarClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgStatusBarPrivate));
}

static void trg_status_bar_init(TrgStatusBar * self)
{
    TrgStatusBarPrivate *priv = TRG_STATUS_BAR_GET_PRIVATE(self);

    priv->connectionCtx =
        gtk_statusbar_get_context_id(GTK_STATUSBAR(self), "connection");
    priv->countSpeedsCtx =
        gtk_statusbar_get_context_id(GTK_STATUSBAR(self),
                                     "counts and speeds");
}

void trg_status_bar_push_connection_msg(TrgStatusBar * sb,
                                        const gchar * msg)
{
    TrgStatusBarPrivate *priv;

    priv = TRG_STATUS_BAR_GET_PRIVATE(sb);

    gtk_statusbar_pop(GTK_STATUSBAR(sb), priv->connectionCtx);
    gtk_statusbar_push(GTK_STATUSBAR(sb), priv->connectionCtx, msg);
}

void trg_status_bar_connect(TrgStatusBar * sb, JsonObject * session)
{
    gchar *statusMsg;
    float version;

    session_get_version(session, &version);
    statusMsg =
        g_strdup_printf
        (_("Connected to Transmission %g, getting torrents..."), version);
    g_printf("%s\n", statusMsg);
    trg_status_bar_push_connection_msg(sb, statusMsg);
    g_free(statusMsg);
}

void trg_status_bar_update(TrgStatusBar * sb,
                           trg_torrent_model_update_stats * stats,
                           trg_client * client)
{
    TrgStatusBarPrivate *priv;
    gchar *statusBarUpdate;
    gint64 uplimitraw, downlimitraw;
    gchar downRateTotalString[32], upRateTotalString[32];
    gchar uplimit[64], downlimit[64];

    priv = TRG_STATUS_BAR_GET_PRIVATE(sb);

    /* The session should always exist otherwise this function wouldn't be called */
    downlimitraw =
        json_object_get_boolean_member(client->session,
                                       SGET_SPEED_LIMIT_DOWN_ENABLED) ?
        json_object_get_int_member(client->session,
                                   SGET_SPEED_LIMIT_DOWN) : -1;

    uplimitraw =
        json_object_get_boolean_member(client->session,
                                       SGET_SPEED_LIMIT_UP_ENABLED) ?
        json_object_get_int_member(client->session,
                                   SGET_SPEED_LIMIT_UP) : -1;

    trg_strlspeed(downRateTotalString,
                  stats->downRateTotal / KILOBYTE_FACTOR);
    trg_strlspeed(upRateTotalString, stats->upRateTotal / KILOBYTE_FACTOR);

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

    statusBarUpdate =
        g_strdup_printf
        (ngettext
         ("%d torrent ..  Down %s%s, Up %s%s  ..  %d seeding, %d downloading, %d paused",
          "%d torrents ..  Down %s%s, Up %s%s  ..  %d seeding, %d downloading, %d paused",
          stats->count), stats->count, downRateTotalString,
         downlimitraw >= 0 ? downlimit : "", upRateTotalString,
         uplimitraw >= 0 ? uplimit : "", stats->seeding, stats->down,
         stats->paused);
    gtk_statusbar_pop(GTK_STATUSBAR(sb), priv->countSpeedsCtx);
    gtk_statusbar_push(GTK_STATUSBAR(sb),
                       priv->countSpeedsCtx, statusBarUpdate);
    g_free(statusBarUpdate);
}


TrgStatusBar *trg_status_bar_new()
{
    return TRG_STATUS_BAR(g_object_new(TRG_TYPE_STATUS_BAR, NULL));
}
