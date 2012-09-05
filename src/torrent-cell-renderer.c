/*
 * This file Copyright (C) Mnemosyne LLC
 *
 * This file is licensed by the GPL version 2. Works owned by the
 * Transmission project are granted a special exemption to clause 2(b)
 * so that the bulk of its code can remain under the MIT license.
 * This exemption does not extend to derived works not owned by
 * the Transmission project.
 *
 * $Id: torrent-cell-renderer.c 13388 2012-07-14 19:26:55Z jordan $
 */

/* This cell renderer has been modified heavily to work with the
 * TrgTorrentModel instead of libtransmission. */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n.h>

#include "hig.h"
#include "icons.h"
#include "trg-client.h"
#include "torrent.h"
#include "util.h"
#include "torrent-cell-renderer.h"

enum {
    P_STATUS = 1,
    P_CLIENT,
    P_RATIO,
    P_SEEDRATIOLIMIT,
    P_SEEDRATIOMODE,
    P_DOWNLOADED,
    P_HAVEVALID,
    P_HAVEUNCHECKED,
    P_ERROR,
    P_SIZEWHENDONE,
    P_TOTALSIZE,
    P_UPLOADED,
    P_PERCENTCOMPLETE,
    P_METADATAPERCENTCOMPLETE,
    P_UPSPEED,
    P_DOWNSPEED,
    P_PEERSGETTINGFROMUS,
    P_WEBSEEDSTOUS,
    P_PEERSTOUS,
    P_ETA,
    P_JSON,
    P_CONNECTED,
    P_FILECOUNT,
    P_BAR_HEIGHT,
    P_OWNER,
    P_COMPACT
};

#define DEFAULT_BAR_HEIGHT 12
#define SMALL_SCALE 0.9
#define COMPACT_ICON_SIZE GTK_ICON_SIZE_MENU
#define FULL_ICON_SIZE GTK_ICON_SIZE_DND

#if GTK_CHECK_VERSION( 3, 0, 0 )
#define FOREGROUND_COLOR_KEY "foreground-rgba"
typedef GdkRGBA GtrColor;
typedef cairo_t GtrDrawable;
typedef GtkRequisition GtrRequisition;
#else
#define FOREGROUND_COLOR_KEY "foreground-gdk"
typedef GdkColor GtrColor;
typedef GdkWindow GtrDrawable;
typedef GdkRectangle GtrRequisition;
#endif

/***
****
***/

static void
render_compact(TorrentCellRenderer * cell,
               GtrDrawable * window,
               GtkWidget * widget,
               const GdkRectangle * background_area,
               const GdkRectangle * cell_area, GtkCellRendererState flags);

static void
render_full(TorrentCellRenderer * cell,
            GtrDrawable * window,
            GtkWidget * widget,
            const GdkRectangle * background_area,
            const GdkRectangle * cell_area, GtkCellRendererState flags);

struct TorrentCellRendererPrivate {
    GtkCellRenderer *text_renderer;
    GtkCellRenderer *progress_renderer;
    GtkCellRenderer *icon_renderer;
    GString *gstr1;
    GString *gstr2;
    int bar_height;

    guint flags;
    guint fileCount;
    gint64 uploadedEver;
    gint64 sizeWhenDone;
    gint64 totalSize;
    gint64 downloaded;
    gint64 haveValid;
    gint64 haveUnchecked;
    gint64 upSpeed;
    gint64 downSpeed;
    gint64 peersFromUs;
    gint64 webSeedsToUs;
    gint64 peersToUs;
    gint64 connected;
    gint64 eta;
    gint64 error;
    gint64 seedRatioMode;
    gdouble done;
    gdouble metadataPercentComplete;
    gdouble ratio;
    gdouble seedRatioLimit;
    gpointer json;
    TrgClient *client;
    GtkTreeView *owner;
    gboolean compact;
};

static gboolean getSeedRatio(TorrentCellRenderer * r, gdouble * ratio)
{
    struct TorrentCellRendererPrivate *p = r->priv;

    if ((p->seedRatioMode == 0)
        && (trg_client_get_seed_ratio_limited(p->client) == TRUE)) {
        *ratio = trg_client_get_seed_ratio_limit(p->client);
        return TRUE;
    } else if (p->seedRatioMode == 1) {
        *ratio = p->seedRatioLimit;
        return TRUE;
    }

    return FALSE;
}

static void getProgressString(GString * gstr, TorrentCellRenderer * r)
{
    struct TorrentCellRendererPrivate *p = r->priv;

    const gint64 haveTotal = p->haveUnchecked + p->haveValid;
    const int isSeed = p->haveValid >= p->totalSize;
    char buf1[32], buf2[32], buf3[32], buf4[32], buf5[32], buf6[32];
    double seedRatio;
    const gboolean hasSeedRatio = getSeedRatio(r, &seedRatio);

    if (p->flags & TORRENT_FLAG_DOWNLOADING) {  /* downloading */
        g_string_append_printf(gstr,
                               /* %1$s is how much we've got,
                                  %2$s is how much we'll have when done,
                                  %3$s%% is a percentage of the two */
                               _("%1$s of %2$s (%3$s)"),
                               tr_strlsize(buf1, haveTotal, sizeof(buf1)),
                               tr_strlsize(buf2, p->sizeWhenDone,
                                           sizeof(buf2)),
                               tr_strlpercent(buf3, p->done,
                                              sizeof(buf3)));
    } else if (!isSeed) {       /* Partial seed */
        if (hasSeedRatio) {
            g_string_append_printf(gstr,
                                   _
                                   ("%1$s of %2$s (%3$s), uploaded %4$s (Ratio: %5$s Goal: %6$s)"),
                                   tr_strlsize(buf1, haveTotal,
                                               sizeof(buf1)),
                                   tr_strlsize(buf2, p->totalSize,
                                               sizeof(buf2)),
                                   tr_strlpercent(buf3, p->done,
                                                  sizeof(buf3)),
                                   tr_strlsize(buf4, p->uploadedEver,
                                               sizeof(buf4)),
                                   tr_strlratio(buf5, p->ratio,
                                                sizeof(buf5)),
                                   tr_strlratio(buf6, seedRatio,
                                                sizeof(buf6)));
        } else {
            g_string_append_printf(gstr,
                                   _
                                   ("%1$s of %2$s (%3$s), uploaded %4$s (Ratio: %5$s)"),
                                   tr_strlsize(buf1, haveTotal,
                                               sizeof(buf1)),
                                   tr_strlsize(buf2, p->totalSize,
                                               sizeof(buf2)),
                                   tr_strlpercent(buf3, p->done,
                                                  sizeof(buf3)),
                                   tr_strlsize(buf4, p->uploadedEver,
                                               sizeof(buf4)),
                                   tr_strlratio(buf5, p->ratio,
                                                sizeof(buf5)));
        }
    } else {                    /* seeding */

        if (hasSeedRatio) {
            g_string_append_printf(gstr,
                                   _
                                   ("%1$s, uploaded %2$s (Ratio: %3$s Goal: %4$s)"),
                                   tr_strlsize(buf1, p->totalSize,
                                               sizeof(buf1)),
                                   tr_strlsize(buf2, p->uploadedEver,
                                               sizeof(buf2)),
                                   tr_strlratio(buf3, p->ratio,
                                                sizeof(buf3)),
                                   tr_strlratio(buf4, seedRatio,
                                                sizeof(buf4)));
        } else {
            g_string_append_printf(gstr,
                                   /* %1$s is the torrent's total size,
                                      %2$s is how much we've uploaded,
                                      %3$s is our upload-to-download ratio */
                                   _("%1$s, uploaded %2$s (Ratio: %3$s)"),
                                   tr_strlsize(buf1, p->sizeWhenDone,
                                               sizeof(buf1)),
                                   tr_strlsize(buf2, p->uploadedEver,
                                               sizeof(buf2)),
                                   tr_strlratio(buf3, p->ratio,
                                                sizeof(buf3)));
        }
    }

    /* add time when downloading */
    if ((p->flags & TORRENT_FLAG_DOWNLOADING)
        || (hasSeedRatio && (p->flags & TORRENT_FLAG_SEEDING))) {
        gint64 eta = p->eta;
        g_string_append(gstr, " - ");
        if (eta < 0)
            g_string_append(gstr, _("Remaining time unknown"));
        else {
            char timestr[128];
            tr_strltime_long(timestr, eta, sizeof(timestr));
            /* time remaining */
            g_string_append_printf(gstr, _("%s remaining"), timestr);
        }
    }
}

static char *getShortTransferString(TorrentCellRenderer * r,
                                    char *buf, size_t buflen)
{
    struct TorrentCellRendererPrivate *priv = r->priv;

    char downStr[32], upStr[32];
    const gboolean haveMeta = priv->fileCount > 0;
    const gboolean haveUp = haveMeta && priv->peersFromUs > 0;
    const gboolean haveDown = haveMeta && priv->peersToUs > 0;

    if (haveDown)
        tr_formatter_speed_KBps(downStr, priv->downSpeed / speed_K,
                                sizeof(downStr));
    if (haveUp)
        tr_formatter_speed_KBps(upStr, priv->upSpeed / speed_K,
                                sizeof(upStr));

    if (haveDown && haveUp)
        /* 1==down arrow, 2==down speed, 3==up arrow, 4==down speed */
        g_snprintf(buf, buflen, _("%1$s %2$s, %3$s %4$s"),
                   GTR_UNICODE_DOWN, downStr, GTR_UNICODE_UP, upStr);
    else if (haveDown)
        /* bandwidth speed + unicode arrow */
        g_snprintf(buf, buflen, _("%1$s %2$s"), GTR_UNICODE_DOWN, downStr);
    else if (haveUp)
        /* bandwidth speed + unicode arrow */
        g_snprintf(buf, buflen, _("%1$s %2$s"), GTR_UNICODE_UP, upStr);
    /*else if( st->isStalled )
       g_strlcpy( buf, _( "Stalled" ), buflen ); */
    else if (haveMeta)
        g_strlcpy(buf, _("Idle"), buflen);
    else
        *buf = '\0';

    return buf;
}

static void getShortStatusString(GString * gstr, TorrentCellRenderer * r)
{
    struct TorrentCellRendererPrivate *priv = r->priv;
    guint flags = priv->flags;

    if (flags & TORRENT_FLAG_PAUSED) {
        g_string_append(gstr,
                        (flags & TORRENT_FLAG_COMPLETE) ? _("Finished") :
                        _("Paused"));
    } else if (flags & TORRENT_FLAG_WAITING_CHECK) {
        g_string_append(gstr, _("Queued for verification"));
    } else if (flags & TORRENT_FLAG_DOWNLOADING_WAIT) {
        g_string_append(gstr, _("Queued for download"));
    } else if (flags & TORRENT_FLAG_SEEDING_WAIT) {
        g_string_append(gstr, _("Queued for seeding"));
    } else if (flags & TORRENT_FLAG_CHECKING) {
        char buf1[32];
        g_string_append_printf(gstr, _("Verifying data (%1$s tested)"),
                               tr_strlpercent(buf1, priv->done,
                                              sizeof(buf1)));
    } else if ((flags & TORRENT_FLAG_DOWNLOADING)
               || (flags & TORRENT_FLAG_SEEDING)) {
        char buf[512];
        if (flags & ~TORRENT_FLAG_DOWNLOADING) {
            tr_strlratio(buf, priv->ratio, sizeof(buf));
            g_string_append_printf(gstr, _("Ratio %s"), buf);
            g_string_append(gstr, ", ");
        }
        getShortTransferString(r, buf, sizeof(buf));
        g_string_append(gstr, buf);
    }
}

static void getStatusString(GString * gstr, TorrentCellRenderer * r)
{
    struct TorrentCellRendererPrivate *priv = r->priv;
    char buf[256];

    if (priv->error) {
        const char *fmt[] = { NULL, N_("Tracker gave a warning: \"%s\""),
            N_("Tracker gave an error: \"%s\""),
            N_("Error: %s")
        };
        g_string_append_printf(gstr, _(fmt[priv->error]),
                               torrent_get_errorstr(priv->json));
    } else if ((priv->flags & TORRENT_FLAG_PAUSED)
               || (priv->flags & TORRENT_FLAG_WAITING_CHECK)
               || (priv->flags & TORRENT_FLAG_CHECKING)
               || (priv->flags & TORRENT_FLAG_DOWNLOADING_WAIT)
               || (priv->flags & TORRENT_FLAG_SEEDING_WAIT)) {
        getShortStatusString(gstr, r);
    } else if (priv->flags & TORRENT_FLAG_DOWNLOADING) {
        if (priv->fileCount > 0) {
            g_string_append_printf(gstr,
                                   ngettext
                                   ("Downloading from %1$li of %2$li connected peer",
                                    "Downloading from %1$li of %2$li connected peers",
                                    priv->webSeedsToUs + priv->peersToUs),
                                   priv->webSeedsToUs + priv->peersToUs,
                                   priv->webSeedsToUs + priv->connected);
        } else {
            g_string_append_printf(gstr,
                                   ngettext
                                   ("Downloading metadata from %1$li peer (%2$s done)",
                                    "Downloading metadata from %1$li peers (%2$s done)",
                                    priv->connected + priv->webSeedsToUs),
                                   priv->connected + priv->webSeedsToUs,
                                   tr_strlpercent(buf,
                                                  priv->metadataPercentComplete,
                                                  sizeof(buf)));
        }
    } else if (priv->flags & TORRENT_FLAG_SEEDING) {
        g_string_append_printf(gstr,
                               ngettext
                               ("Seeding to %1$li of %2$li connected peer",
                                "Seeding to %1$li of %2$li connected peers",
                                priv->connected), priv->peersFromUs,
                               priv->connected);
    }

    if ((priv->flags & ~TORRENT_FLAG_WAITING_CHECK) &&
        (priv->flags & ~TORRENT_FLAG_CHECKING) &&
        (priv->flags & ~TORRENT_FLAG_DOWNLOADING_WAIT) &&
        (priv->flags & ~TORRENT_FLAG_SEEDING_WAIT) &&
        (priv->flags & ~TORRENT_FLAG_PAUSED)) {
        getShortTransferString(r, buf, sizeof(buf));
        if (*buf)
            g_string_append_printf(gstr, " - %s", buf);
    }
}

/***
****
***/

static GdkPixbuf *get_icon(TorrentCellRenderer * r, GtkIconSize icon_size,
                           GtkWidget * for_widget)
{
    struct TorrentCellRendererPrivate *p = r->priv;

    const char *mime_type;

    if (p->fileCount == 0)
        mime_type = UNKNOWN_MIME_TYPE;
    else if (p->fileCount > 1)
        mime_type = DIRECTORY_MIME_TYPE;
    /*else if( strchr( info->files[0].name, '/' ) != NULL )
       mime_type = DIRECTORY_MIME_TYPE;
       else
       mime_type = gtr_get_mime_type_from_filename( info->files[0].name ); */
    else
        mime_type = FILE_MIME_TYPE;

    //return NULL;
    return gtr_get_mime_type_icon(mime_type, icon_size, for_widget);
}

/***
****
***/

static void
gtr_cell_renderer_get_preferred_size(GtkCellRenderer * renderer,
                                     GtkWidget * widget,
                                     GtkRequisition * minimum_size,
                                     GtkRequisition * natural_size)
{
#if GTK_CHECK_VERSION( 3, 0, 0 )
    gtk_cell_renderer_get_preferred_size(renderer, widget, minimum_size,
                                         natural_size);
#else
    GtkRequisition r;
    gtk_cell_renderer_get_size(renderer, widget, NULL, NULL, NULL,
                               &r.width, &r.height);
    if (minimum_size)
        *minimum_size = r;
    if (natural_size)
        *natural_size = r;
#endif
}

static void
get_size_compact(TorrentCellRenderer * cell,
                 GtkWidget * widget, gint * width, gint * height)
{
    int xpad, ypad;
    GtkRequisition icon_size;
    GtkRequisition name_size;
    GtkRequisition stat_size;
    GdkPixbuf *icon;

    struct TorrentCellRendererPrivate *p = cell->priv;
    GString *gstr_stat = p->gstr1;

    icon = get_icon(cell, COMPACT_ICON_SIZE, widget);
    g_string_truncate(gstr_stat, 0);
    getShortStatusString(gstr_stat, cell);
    gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(cell), &xpad, &ypad);

    /* get the idealized cell dimensions */
    g_object_set(p->icon_renderer, "pixbuf", icon, NULL);
    gtr_cell_renderer_get_preferred_size(p->icon_renderer, widget, NULL,
                                         &icon_size);
    g_object_set(p->text_renderer, "text", torrent_get_name(p->json),
                 "ellipsize", PANGO_ELLIPSIZE_NONE, "scale", 1.0, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &name_size);
    g_object_set(p->text_renderer, "text", gstr_stat->str, "scale",
                 SMALL_SCALE, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &stat_size);

    /**
    *** LAYOUT
    **/

#define BAR_WIDTH 50
    if (width != NULL)
        *width =
            xpad * 2 + icon_size.width + GUI_PAD + name_size.width +
            GUI_PAD + BAR_WIDTH + GUI_PAD + stat_size.width;
    if (height != NULL)
        *height = ypad * 2 + MAX(name_size.height, p->bar_height);

    /* cleanup */
    g_object_unref(icon);
}

#define MAX3(a,b,c) MAX(a,MAX(b,c))

static void
get_size_full(TorrentCellRenderer * cell,
              GtkWidget * widget, gint * width, gint * height)
{
    int xpad, ypad;
    GtkRequisition icon_size;
    GtkRequisition name_size;
    GtkRequisition stat_size;
    GtkRequisition prog_size;
    GdkPixbuf *icon;

    struct TorrentCellRendererPrivate *p = cell->priv;
    GString *gstr_prog = p->gstr1;
    GString *gstr_stat = p->gstr2;

    icon = get_icon(cell, FULL_ICON_SIZE, widget);

    g_string_truncate(gstr_stat, 0);
    getStatusString(gstr_stat, cell);
    g_string_truncate(gstr_prog, 0);
    getProgressString(gstr_prog, cell);
    gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(cell), &xpad, &ypad);

    /* get the idealized cell dimensions */
    g_object_set(p->icon_renderer, "pixbuf", icon, NULL);
    gtr_cell_renderer_get_preferred_size(p->icon_renderer, widget, NULL,
                                         &icon_size);
    g_object_set(p->text_renderer, "text", torrent_get_name(p->json),
                 "weight", PANGO_WEIGHT_BOLD, "scale", 1.0, "ellipsize",
                 PANGO_ELLIPSIZE_NONE, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &name_size);
    g_object_set(p->text_renderer, "text", gstr_prog->str, "weight",
                 PANGO_WEIGHT_NORMAL, "scale", SMALL_SCALE, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &prog_size);
    g_object_set(p->text_renderer, "text", gstr_stat->str, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &stat_size);

    /**
    *** LAYOUT
    **/

    if (width != NULL)
        *width =
            xpad * 2 + icon_size.width + GUI_PAD + MAX3(name_size.width,
                                                        prog_size.width,
                                                        stat_size.width);
    if (height != NULL)
        *height =
            ypad * 2 + name_size.height + prog_size.height +
            GUI_PAD_SMALL + p->bar_height + GUI_PAD_SMALL +
            stat_size.height;

    /* cleanup */
    g_object_unref(icon);
}


static void
torrent_cell_renderer_get_size(GtkCellRenderer * cell, GtkWidget * widget,
#if GTK_CHECK_VERSION( 3,0,0 )
                               const GdkRectangle * cell_area,
#else
                               GdkRectangle * cell_area,
#endif
                               gint * x_offset,
                               gint * y_offset,
                               gint * width, gint * height)
{
    TorrentCellRenderer *self = TORRENT_CELL_RENDERER(cell);

    if (self) {
        int w, h;
        struct TorrentCellRendererPrivate *p = self->priv;

        if (p->compact)
            get_size_compact(TORRENT_CELL_RENDERER(cell), widget, &w, &h);
        else
            get_size_full(TORRENT_CELL_RENDERER(cell), widget, &w, &h);

        if (width)
            *width = w;

        if (height)
            *height = h;

        if (x_offset)
            *x_offset = cell_area ? cell_area->x : 0;

        if (y_offset) {
            int xpad, ypad;
            gtk_cell_renderer_get_padding(cell, &xpad, &ypad);
            *y_offset =
                cell_area ? (int) ((cell_area->height - (ypad * 2 + h)) /
                                   2.0) : 0;
        }
    }
}

static void
get_text_color(TorrentCellRenderer * r, GtkWidget * widget,
               GtrColor * setme)
{
    struct TorrentCellRendererPrivate *p = r->priv;
#if GTK_CHECK_VERSION( 3,0,0 )
    static const GdkRGBA red = { 1.0, 0, 0, 0 };

    if (p->error)
        *setme = red;
    else if (p->flags & TORRENT_FLAG_PAUSED)
        gtk_style_context_get_color(gtk_widget_get_style_context(widget),
                                    GTK_STATE_FLAG_INSENSITIVE, setme);
    else
        gtk_style_context_get_color(gtk_widget_get_style_context(widget),
                                    GTK_STATE_FLAG_NORMAL, setme);
#else
    static const GdkColor red = { 0, 65535, 0, 0 };

    if (p->error)
        *setme = red;
    else if (p->flags & TORRENT_FLAG_PAUSED)
        *setme = gtk_widget_get_style(widget)->text[GTK_STATE_INSENSITIVE];
    else
        *setme = gtk_widget_get_style(widget)->text[GTK_STATE_NORMAL];
#endif
}

static double get_percent_done(TorrentCellRenderer * r, gboolean * seed)
{
    struct TorrentCellRendererPrivate *priv = r->priv;
    gdouble d;

    if ((priv->flags & TORRENT_FLAG_SEEDING) && getSeedRatio(r, &d)) {
        *seed = TRUE;
        const gint64 baseline =
            priv->downloaded ? priv->downloaded : priv->sizeWhenDone;
        const gint64 goal = baseline * priv->seedRatioLimit;
        const gint64 bytesLeft =
            goal > priv->uploadedEver ? goal - priv->uploadedEver : 0;
        float seedRatioPercentDone = (gdouble) (goal - bytesLeft) / goal;

        d = MAX(0.0, seedRatioPercentDone * 100.0);
    } else {
        *seed = FALSE;
        d = MAX(0.0, priv->done);
    }

    return d;
}

static void
gtr_cell_renderer_render(GtkCellRenderer * renderer,
                         GtrDrawable * drawable,
                         GtkWidget * widget,
                         const GdkRectangle * area,
                         GtkCellRendererState flags)
{
#if GTK_CHECK_VERSION( 3, 0, 0 )
    gtk_cell_renderer_render(renderer, drawable, widget, area, area,
                             flags);
#else
    gtk_cell_renderer_render(renderer, drawable, widget, area, area, area,
                             flags);
#endif
}

static void
torrent_cell_renderer_render(GtkCellRenderer * cell,
                             GtrDrawable * window, GtkWidget * widget,
#if GTK_CHECK_VERSION( 3,0,0 )
                             const GdkRectangle * background_area,
                             const GdkRectangle * cell_area,
#else
                             GdkRectangle * background_area,
                             GdkRectangle * cell_area,
                             GdkRectangle * expose_area,
#endif
                             GtkCellRendererState flags)
{
    TorrentCellRenderer *self = TORRENT_CELL_RENDERER(cell);

    if (self) {
        struct TorrentCellRendererPrivate *p = self->priv;
        if (p->compact)
            render_compact(self, window, widget, background_area,
                           cell_area, flags);
        else
            render_full(self, window, widget, background_area, cell_area,
                        flags);
    }
}

static void torrent_cell_renderer_set_property(GObject * object,
                                               guint property_id,
                                               const GValue * v,
                                               GParamSpec * pspec)
{
    TorrentCellRenderer *self = TORRENT_CELL_RENDERER(object);
    struct TorrentCellRendererPrivate *p = self->priv;

    switch (property_id) {
    case P_JSON:
        p->json = g_value_get_pointer(v);
        break;
    case P_STATUS:
        p->flags = g_value_get_uint(v);
        break;
    case P_TOTALSIZE:
        p->totalSize = g_value_get_int64(v);
        break;
    case P_SIZEWHENDONE:
        p->sizeWhenDone = g_value_get_int64(v);
        break;
    case P_DOWNLOADED:
        p->downloaded = g_value_get_int64(v);
        break;
    case P_HAVEVALID:
        p->haveValid = g_value_get_int64(v);
        break;
    case P_HAVEUNCHECKED:
        p->haveUnchecked = g_value_get_int64(v);
        break;
    case P_UPLOADED:
        p->uploadedEver = g_value_get_int64(v);
        break;
    case P_UPSPEED:
        p->upSpeed = g_value_get_int64(v);
        break;
    case P_DOWNSPEED:
        p->downSpeed = g_value_get_int64(v);
        break;
    case P_PEERSGETTINGFROMUS:
        p->peersFromUs = g_value_get_int64(v);
        break;
    case P_WEBSEEDSTOUS:
        p->webSeedsToUs = g_value_get_int64(v);
        break;
    case P_CONNECTED:
        p->connected = g_value_get_int64(v);
        break;
    case P_FILECOUNT:
        p->fileCount = g_value_get_uint(v);
        break;
    case P_ETA:
        p->eta = g_value_get_int64(v);
        break;
    case P_PEERSTOUS:
        p->peersToUs = g_value_get_int64(v);
        break;
    case P_ERROR:
        p->error = g_value_get_int64(v);
        break;
    case P_RATIO:
        p->ratio = g_value_get_double(v);
        break;
    case P_PERCENTCOMPLETE:
        p->done = g_value_get_double(v);
        break;
    case P_METADATAPERCENTCOMPLETE:
        p->metadataPercentComplete = g_value_get_double(v);
        break;
    case P_BAR_HEIGHT:
        p->bar_height = g_value_get_int(v);
        break;
    case P_COMPACT:
        p->compact = g_value_get_boolean(v);
        break;
    case P_SEEDRATIOMODE:
        p->seedRatioMode = g_value_get_int64(v);
        break;
    case P_SEEDRATIOLIMIT:
        p->seedRatioLimit = g_value_get_double(v);
        break;
    case P_CLIENT:
        p->client = g_value_get_pointer(v);
        break;
    case P_OWNER:
        p->owner = g_value_get_pointer(v);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
torrent_cell_renderer_get_property(GObject * object,
                                   guint property_id,
                                   GValue * v, GParamSpec * pspec)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

G_DEFINE_TYPE(TorrentCellRenderer, torrent_cell_renderer,
              GTK_TYPE_CELL_RENDERER)

static void torrent_cell_renderer_dispose(GObject * o)
{
    TorrentCellRenderer *r = TORRENT_CELL_RENDERER(o);

    if (r && r->priv) {
        g_string_free(r->priv->gstr1, TRUE);
        g_string_free(r->priv->gstr2, TRUE);
        g_object_unref(G_OBJECT(r->priv->text_renderer));
        g_object_unref(G_OBJECT(r->priv->progress_renderer));
        g_object_unref(G_OBJECT(r->priv->icon_renderer));
        r->priv = NULL;
    }

    G_OBJECT_CLASS(torrent_cell_renderer_parent_class)->dispose(o);
}

static void
torrent_cell_renderer_class_init(TorrentCellRendererClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS(klass);

    g_type_class_add_private(klass,
                             sizeof(struct TorrentCellRendererPrivate));

    cell_class->render = torrent_cell_renderer_render;
    cell_class->get_size = torrent_cell_renderer_get_size;
    gobject_class->set_property = torrent_cell_renderer_set_property;
    gobject_class->get_property = torrent_cell_renderer_get_property;
    gobject_class->dispose = torrent_cell_renderer_dispose;

    g_object_class_install_property(gobject_class, P_JSON,
                                    g_param_spec_pointer("json", NULL,
                                                         "json",
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_CLIENT,
                                    g_param_spec_pointer("client", NULL,
                                                         "client",
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_OWNER,
                                    g_param_spec_pointer("owner", NULL,
                                                         "owner",
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_RATIO,
                                    g_param_spec_double("ratio", NULL,
                                                        "ratio",
                                                        0, G_MAXDOUBLE, 0,
                                                        G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_SEEDRATIOLIMIT,
                                    g_param_spec_double("seedRatioLimit",
                                                        NULL,
                                                        "seedRatioLimit",
                                                        0, G_MAXDOUBLE, 0,
                                                        G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_SEEDRATIOMODE,
                                    g_param_spec_int64("seedRatioMode",
                                                       NULL,
                                                       "seedRatioMode", 0,
                                                       2, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_PERCENTCOMPLETE,
                                    g_param_spec_double("percentComplete",
                                                        NULL,
                                                        "percentComplete",
                                                        0, 100.00, 0,
                                                        G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class,
                                    P_METADATAPERCENTCOMPLETE,
                                    g_param_spec_double
                                    ("metadataPercentComplete", NULL,
                                     "metadataPercentComplete", 0, 100.00,
                                     0, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_TOTALSIZE,
                                    g_param_spec_int64("totalSize", NULL,
                                                       "totalSize",
                                                       0, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_SIZEWHENDONE,
                                    g_param_spec_int64("sizeWhenDone",
                                                       NULL,
                                                       "sizeWhenDone", 0,
                                                       G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_STATUS,
                                    g_param_spec_uint("status", NULL,
                                                      "status",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_FILECOUNT,
                                    g_param_spec_uint("fileCount", NULL,
                                                      "fileCount",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_ERROR,
                                    g_param_spec_int64("error", NULL,
                                                       "error",
                                                       0, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_UPSPEED,
                                    g_param_spec_int64("upSpeed", NULL,
                                                       "upSpeed",
                                                       0, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_DOWNSPEED,
                                    g_param_spec_int64("downSpeed", NULL,
                                                       "downSpeed",
                                                       0, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_DOWNLOADED,
                                    g_param_spec_int64("downloaded", NULL,
                                                       "downloaded",
                                                       0, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_HAVEVALID,
                                    g_param_spec_int64("haveValid", NULL,
                                                       "haveValid",
                                                       0, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_HAVEUNCHECKED,
                                    g_param_spec_int64("haveUnchecked",
                                                       NULL,
                                                       "haveUnchecked", 0,
                                                       G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_UPLOADED,
                                    g_param_spec_int64("uploaded", NULL,
                                                       "uploaded",
                                                       0, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_PEERSGETTINGFROMUS,
                                    g_param_spec_int64
                                    ("peersGettingFromUs", NULL,
                                     "peersGettingFromUs", -1, G_MAXINT64,
                                     0, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_PEERSTOUS,
                                    g_param_spec_int64("peersToUs", NULL,
                                                       "peersToUs",
                                                       -1, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_WEBSEEDSTOUS,
                                    g_param_spec_int64("webSeedsToUs",
                                                       NULL,
                                                       "webSeedsToUs", 0,
                                                       G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_ETA,
                                    g_param_spec_int64("eta", NULL,
                                                       "eta",
                                                       -2, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_CONNECTED,
                                    g_param_spec_int64("connected", NULL,
                                                       "connected",
                                                       0, G_MAXINT64, 0,
                                                       G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_BAR_HEIGHT,
                                    g_param_spec_int("bar-height", NULL,
                                                     "Bar Height",
                                                     1, INT_MAX,
                                                     DEFAULT_BAR_HEIGHT,
                                                     G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, P_COMPACT,
                                    g_param_spec_boolean("compact", NULL,
                                                         "Compact Mode",
                                                         FALSE,
                                                         G_PARAM_READWRITE));
}

static void torrent_cell_renderer_init(TorrentCellRenderer * self)
{
    struct TorrentCellRendererPrivate *p;

    p = self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
                                                 TORRENT_CELL_RENDERER_TYPE,
                                                 struct
                                                 TorrentCellRendererPrivate);

    p->gstr1 = g_string_new(NULL);
    p->gstr2 = g_string_new(NULL);
    p->text_renderer = gtk_cell_renderer_text_new();
    g_object_set(p->text_renderer, "xpad", 0, "ypad", 0, NULL);
    p->progress_renderer = gtk_cell_renderer_progress_new();
    p->icon_renderer = gtk_cell_renderer_pixbuf_new();
    g_object_ref_sink(p->text_renderer);
    g_object_ref_sink(p->progress_renderer);
    g_object_ref_sink(p->icon_renderer);

    p->bar_height = DEFAULT_BAR_HEIGHT;
}


GtkCellRenderer *torrent_cell_renderer_new(void)
{
    return (GtkCellRenderer *) g_object_new(TORRENT_CELL_RENDERER_TYPE,
                                            NULL);
}

static void
render_compact(TorrentCellRenderer * cell,
               GtrDrawable * window,
               GtkWidget * widget,
               const GdkRectangle * background_area,
               const GdkRectangle * cell_area, GtkCellRendererState flags)
{
    int xpad, ypad;
    GtkRequisition size;
    GdkRectangle icon_area;
    GdkRectangle name_area;
    GdkRectangle stat_area;
    GdkRectangle prog_area;
    GdkRectangle fill_area;
    GdkPixbuf *icon;
    GtrColor text_color;
    gboolean seed;

    struct TorrentCellRendererPrivate *p = cell->priv;
    const gboolean active = (p->flags & ~TORRENT_FLAG_PAUSED)
        && (p->flags & ~TORRENT_FLAG_DOWNLOADING_WAIT)
        && (p->flags & ~TORRENT_FLAG_SEEDING_WAIT);
    const double percentDone = get_percent_done(cell, &seed);
    const gboolean sensitive = active || p->error;
    GString *gstr_stat = p->gstr1;

    icon = get_icon(cell, COMPACT_ICON_SIZE, widget);

    g_string_truncate(gstr_stat, 0);
    getShortStatusString(gstr_stat, cell);
    gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(cell), &xpad, &ypad);
    get_text_color(cell, widget, &text_color);

    fill_area = *background_area;
    fill_area.x += xpad;
    fill_area.y += ypad;
    fill_area.width -= xpad * 2;
    fill_area.height -= ypad * 2;
    icon_area = name_area = stat_area = prog_area = fill_area;

    g_object_set(p->icon_renderer, "pixbuf", icon, NULL);
    gtr_cell_renderer_get_preferred_size(p->icon_renderer, widget, NULL,
                                         &size);
    icon_area.width = size.width;
    g_object_set(p->text_renderer, "text", torrent_get_name(p->json),
                 "ellipsize", PANGO_ELLIPSIZE_NONE, "scale", 1.0, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &size);
    name_area.width = size.width;
    g_object_set(p->text_renderer, "text", gstr_stat->str, "scale",
                 SMALL_SCALE, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &size);
    stat_area.width = size.width;

    icon_area.x = fill_area.x;
    prog_area.x = fill_area.x + fill_area.width - BAR_WIDTH;
    prog_area.width = BAR_WIDTH;
    stat_area.x = prog_area.x - GUI_PAD - stat_area.width;
    name_area.x = icon_area.x + icon_area.width + GUI_PAD;
    name_area.y = fill_area.y;
    name_area.width = stat_area.x - GUI_PAD - name_area.x;

    /**
    *** RENDER
    **/

    g_object_set(p->icon_renderer, "pixbuf", icon, "sensitive", sensitive,
                 NULL);
    gtr_cell_renderer_render(p->icon_renderer, window, widget, &icon_area,
                             flags);
    g_object_set(p->progress_renderer, "value", (gint) percentDone, "text",
                 NULL, "sensitive", sensitive, NULL);
    gtr_cell_renderer_render(p->progress_renderer, window, widget,
                             &prog_area, flags);
    g_object_set(p->text_renderer, "text", gstr_stat->str, "scale",
                 SMALL_SCALE, "ellipsize", PANGO_ELLIPSIZE_END,
                 FOREGROUND_COLOR_KEY, &text_color, NULL);
    gtr_cell_renderer_render(p->text_renderer, window, widget, &stat_area,
                             flags);
    g_object_set(p->text_renderer, "text", torrent_get_name(p->json),
                 "scale", 1.0, FOREGROUND_COLOR_KEY, &text_color, NULL);
    gtr_cell_renderer_render(p->text_renderer, window, widget, &name_area,
                             flags);

    /* cleanup */
    g_object_unref(icon);
}

static void
render_full(TorrentCellRenderer * cell,
            GtrDrawable * window,
            GtkWidget * widget,
            const GdkRectangle * background_area,
            const GdkRectangle * cell_area, GtkCellRendererState flags)
{
    int xpad, ypad;
    GtkRequisition size;
    GdkRectangle fill_area;
    GdkRectangle icon_area;
    GdkRectangle name_area;
    GdkRectangle stat_area;
    GdkRectangle prog_area;
    GdkRectangle prct_area;
    GdkPixbuf *icon;
    GtrColor text_color;
    gboolean seed;

    struct TorrentCellRendererPrivate *p = cell->priv;
    const gboolean active = (p->flags & ~TORRENT_FLAG_PAUSED)
        && (p->flags & ~TORRENT_FLAG_DOWNLOADING_WAIT)
        && (p->flags & ~TORRENT_FLAG_SEEDING_WAIT);
    const gboolean sensitive = active || p->error;
    const double percentDone = get_percent_done(cell, &seed);
    GString *gstr_prog = p->gstr1;
    GString *gstr_stat = p->gstr2;

    icon = get_icon(cell, FULL_ICON_SIZE, widget);
    g_string_truncate(gstr_prog, 0);
    getProgressString(gstr_prog, cell);
    g_string_truncate(gstr_stat, 0);
    getStatusString(gstr_stat, cell);
    gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(cell), &xpad, &ypad);
    get_text_color(cell, widget, &text_color);

    /* get the idealized cell dimensions */
    g_object_set(p->icon_renderer, "pixbuf", icon, NULL);
    gtr_cell_renderer_get_preferred_size(p->icon_renderer, widget, NULL,
                                         &size);
    icon_area.width = size.width;
    icon_area.height = size.height;
    g_object_set(p->text_renderer, "text", torrent_get_name(p->json),
                 "weight", PANGO_WEIGHT_BOLD, "ellipsize",
                 PANGO_ELLIPSIZE_NONE, "scale", 1.0, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &size);
    name_area.width = size.width;
    name_area.height = size.height;
    g_object_set(p->text_renderer, "text", gstr_prog->str, "weight",
                 PANGO_WEIGHT_NORMAL, "scale", SMALL_SCALE, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &size);
    prog_area.width = size.width;
    prog_area.height = size.height;
    g_object_set(p->text_renderer, "text", gstr_stat->str, NULL);
    gtr_cell_renderer_get_preferred_size(p->text_renderer, widget, NULL,
                                         &size);
    stat_area.width = size.width;
    stat_area.height = size.height;

    /**
    *** LAYOUT
    **/

    fill_area = *background_area;
    fill_area.x += xpad;
    fill_area.y += ypad;
    fill_area.width -= xpad * 2;
    fill_area.height -= ypad * 2;

    /* icon */
    icon_area.x = fill_area.x;
    icon_area.y = fill_area.y + (fill_area.height - icon_area.height) / 2;

    /* name */
    name_area.x = icon_area.x + icon_area.width + GUI_PAD;
    name_area.y = fill_area.y;
    name_area.width =
        fill_area.width - GUI_PAD - icon_area.width - GUI_PAD_SMALL;

    /* prog */
    prog_area.x = name_area.x;
    prog_area.y = name_area.y + name_area.height;
    prog_area.width = name_area.width;

    /* progressbar */
    prct_area.x = prog_area.x;
    prct_area.y = prog_area.y + prog_area.height + GUI_PAD_SMALL;
    prct_area.width = prog_area.width;
    prct_area.height = p->bar_height;

    /* status */
    stat_area.x = prct_area.x;
    stat_area.y = prct_area.y + prct_area.height + GUI_PAD_SMALL;
    stat_area.width = prct_area.width;

    /**
    *** RENDER
    **/

    g_object_set(p->icon_renderer, "pixbuf", icon, "sensitive", sensitive,
                 NULL);
    gtr_cell_renderer_render(p->icon_renderer, window, widget, &icon_area,
                             flags);
    g_object_set(p->text_renderer, "text", torrent_get_name(p->json),
                 "scale", 1.0, FOREGROUND_COLOR_KEY, &text_color,
                 "ellipsize", PANGO_ELLIPSIZE_END, "weight",
                 PANGO_WEIGHT_BOLD, NULL);
    gtr_cell_renderer_render(p->text_renderer, window, widget, &name_area,
                             flags);
    g_object_set(p->text_renderer, "text", gstr_prog->str, "scale",
                 SMALL_SCALE, "weight", PANGO_WEIGHT_NORMAL, NULL);
    gtr_cell_renderer_render(p->text_renderer, window, widget, &prog_area,
                             flags);
    g_object_set(p->progress_renderer, "value", (gint) percentDone,
                 "text", "", "sensitive", sensitive, NULL);
    gtr_cell_renderer_render(p->progress_renderer, window, widget,
                             &prct_area, flags);
    g_object_set(p->text_renderer, "text", gstr_stat->str,
                 FOREGROUND_COLOR_KEY, &text_color, NULL);
    gtr_cell_renderer_render(p->text_renderer, window, widget, &stat_area,
                             flags);

    /* cleanup */
    g_object_unref(icon);
}

GtkTreeView *torrent_cell_renderer_get_owner(TorrentCellRenderer * r)
{
    return r->priv->owner;
}
