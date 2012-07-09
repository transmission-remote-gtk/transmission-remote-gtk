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

/*
 * This graph drawing code was taken from gnome-system-monitor (load-graph.cpp)
 * Converted the class from C++ to GObject, substituted out some STL (C++)
 * functions, and removed the unecessary parts for memory/cpu.
 *
 * Would be nice if it supported configurable timespans. This could possibly
 * be replaced with ubergraph, which is a graphing widget for GTK (C) also based
 * on this widget but with some improvements I didn't do.
 */

#include "trg-torrent-graph3.h"

#if GTK_CHECK_VERSION( 3, 0, 0 )

#include <math.h>
#include <glib.h>
#include <cairo.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "util.h"

/* damn you freebsd */
#define log2(x) (log(x)/0.69314718055994530942)

#define GRAPH_NUM_POINTS 62
#define GRAPH_MIN_HEIGHT 40
#define GRAPH_NUM_LINES 2
#define GRAPH_NUM_DATA_BLOCK_ELEMENTS GRAPH_NUM_POINTS * GRAPH_NUM_LINES
#define GRAPH_OUT_COLOR "#2D7DB3"
#define GRAPH_IN_COLOR "#844798"
#define GRAPH_LINE_WIDTH 3
#define GRAPH_FRAME_WIDTH 4

G_DEFINE_TYPE(TrgTorrentGraph, trg_torrent_graph, GTK_TYPE_VBOX)
#define TRG_TORRENT_GRAPH_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_TORRENT_GRAPH, TrgTorrentGraphPrivate))
typedef struct _TrgTorrentGraphPrivate TrgTorrentGraphPrivate;

struct _TrgTorrentGraphPrivate {
    double fontsize;
    double rmargin;
    double indent;
    guint speed;
    guint draw_width, draw_height;
    guint render_counter;
    guint frames_per_unit;
    guint graph_dely;
    guint real_draw_height;
    double graph_delx;
    guint graph_buffer_offset;

    GdkColor colors[GRAPH_NUM_LINES];

    float data_block[GRAPH_NUM_POINTS * GRAPH_NUM_LINES];
    GList *points;

    GtkWidget *disp;
    cairo_surface_t *background;
    guint timer_index;
    gboolean draw;
    guint64 out, in;
    unsigned int max;
    unsigned values[GRAPH_NUM_POINTS];
    size_t cur;
    GtkStyle *style;

    GtkWidget *label_in;
    GtkWidget *label_out;
};

static int trg_torrent_graph_update(gpointer user_data);

static void
trg_torrent_graph_get_property(GObject * object, guint property_id,
                               GValue * value, GParamSpec * pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_torrent_graph_set_property(GObject * object, guint property_id,
                               const GValue * value, GParamSpec * pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

void trg_torrent_graph_draw_background(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv;
    GtkAllocation allocation;

    double dash[2] = { 1.0, 2.0 };
    cairo_t *cr;
    guint i;
    unsigned num_bars;
    char *caption;
    cairo_text_extents_t extents;
    unsigned rate;
    unsigned total_seconds;

    priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    num_bars = trg_torrent_graph_get_num_bars(g);
    priv->graph_dely = (priv->draw_height - 15) / num_bars;     /* round to int to avoid AA blur */
    priv->real_draw_height = priv->graph_dely * num_bars;
    priv->graph_delx =
        (priv->draw_width - 2.0 - priv->rmargin -
         priv->indent) / (GRAPH_NUM_POINTS - 3);
    priv->graph_buffer_offset =
        (int) (1.5 * priv->graph_delx) + GRAPH_FRAME_WIDTH;

    gtk_widget_get_allocation(priv->disp, &allocation);

    cr = cairo_create(priv->background);

    gdk_cairo_set_source_color(cr, &priv->style->bg[GTK_STATE_NORMAL]);
    cairo_paint(cr);
    cairo_translate(cr, GRAPH_FRAME_WIDTH, GRAPH_FRAME_WIDTH);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_rectangle(cr, priv->rmargin + priv->indent, 0,
                    priv->draw_width - priv->rmargin - priv->indent,
                    priv->real_draw_height);

    cairo_fill(cr);

    cairo_set_line_width(cr, 1.0);
    cairo_set_dash(cr, dash, 2, 0);
    cairo_set_font_size(cr, priv->fontsize);

    for (i = 0; i <= num_bars; ++i) {
        double y;
        gchar caption[32];

        if (i == 0)
            y = 0.5 + priv->fontsize / 2.0;
        else if (i == num_bars)
            y = i * priv->graph_dely + 0.5;
        else
            y = i * priv->graph_dely + priv->fontsize / 2.0;

        gdk_cairo_set_source_color(cr, &priv->style->fg[GTK_STATE_NORMAL]);
        rate = priv->max - (i * priv->max / num_bars);
        trg_strlspeed(caption, (gint64) (rate / 1024));
        cairo_text_extents(cr, caption, &extents);
        cairo_move_to(cr, priv->indent - extents.width + 20, y);
        cairo_show_text(cr, caption);
    }

    cairo_stroke(cr);

    cairo_set_dash(cr, dash, 2, 1.5);

    total_seconds = priv->speed * (GRAPH_NUM_POINTS - 2) / 1000;

    for (i = 0; i < 7; i++) {
        unsigned seconds;
        const char *format;
        double x =
            (i) * (priv->draw_width - priv->rmargin - priv->indent) / 6;
        cairo_set_source_rgba(cr, 0, 0, 0, 0.75);
        cairo_move_to(cr, (ceil(x) + 0.5) + priv->rmargin + priv->indent,
                      0.5);
        cairo_line_to(cr, (ceil(x) + 0.5) + priv->rmargin + priv->indent,
                      priv->real_draw_height + 4.5);
        cairo_stroke(cr);
        seconds = total_seconds - i * total_seconds / 6;
        if (i == 0)
            format = "%u seconds";
        else
            format = "%u";
        caption = g_strdup_printf(format, seconds);
        cairo_text_extents(cr, caption, &extents);
        cairo_move_to(cr,
                      ((ceil(x) + 0.5) + priv->rmargin + priv->indent) -
                      (extents.width / 2), priv->draw_height);
        gdk_cairo_set_source_color(cr, &priv->style->fg[GTK_STATE_NORMAL]);
        cairo_show_text(cr, caption);
        g_free(caption);
    }

    cairo_stroke(cr);
    cairo_destroy(cr);
}

void trg_torrent_graph_set_nothing(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    priv->in = priv->out = 0;
}

void
trg_torrent_graph_set_speed(TrgTorrentGraph * g,
                            trg_torrent_model_update_stats * stats)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    priv->in = (guint64) stats->downRateTotal;
    priv->out = (guint64) stats->upRateTotal;
}

static gboolean
trg_torrent_graph_configure(GtkWidget * widget,
                            GdkEventConfigure * event, gpointer data_ptr)
{
    GtkAllocation allocation;
    TrgTorrentGraph *g = TRG_TORRENT_GRAPH(data_ptr);
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(data_ptr);

    gtk_widget_get_allocation(widget, &allocation);
    priv->draw_width = allocation.width - 2 * GRAPH_FRAME_WIDTH;
    priv->draw_height = allocation.height - 2 * GRAPH_FRAME_WIDTH;

    trg_torrent_graph_clear_background(g);

    trg_torrent_graph_queue_draw(g);

    return TRUE;
}

static gboolean
trg_torrent_graph_draw(GtkWidget * widget,
                         cairo_t *context, gpointer data_ptr)
{
    TrgTorrentGraph *g = TRG_TORRENT_GRAPH(data_ptr);
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(data_ptr);
    GdkWindow *window = gtk_widget_get_window(priv->disp);

    GtkAllocation allocation;
    cairo_t *cr;

    guint i, j;
    float *fp;
    gdouble sample_width, x_offset;

    if (priv->background == NULL) {
    	cairo_pattern_t * pattern = NULL;
        trg_torrent_graph_draw_background(g);
        pattern = cairo_pattern_create_for_surface(priv->background);
        gdk_window_set_background_pattern (window, pattern);
        cairo_pattern_destroy (pattern);
    }

    gtk_widget_get_allocation(priv->disp, &allocation);
    /*gdk_draw_drawable(window,
                      priv->gc,
                      priv->background,
                      0, 0, 0, 0, allocation.width, allocation.height);*/

    sample_width =
        (float) (priv->draw_width - priv->rmargin -
                 priv->indent) / (float) GRAPH_NUM_POINTS;
    x_offset = priv->draw_width - priv->rmargin + (sample_width * 2);
    x_offset +=
        priv->rmargin -
        ((sample_width / priv->frames_per_unit) * priv->render_counter);

    cr = gdk_cairo_create(window);

    cairo_set_line_width(cr, GRAPH_LINE_WIDTH);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_rectangle(cr,
                    priv->rmargin + priv->indent + GRAPH_FRAME_WIDTH + 1,
                    GRAPH_FRAME_WIDTH - 1,
                    priv->draw_width - priv->rmargin - priv->indent - 1,
                    priv->real_draw_height + GRAPH_FRAME_WIDTH - 1);
    cairo_clip(cr);

    for (j = 0; j < GRAPH_NUM_LINES; ++j) {
        GList *li = priv->points;
        fp = (float *) li->data;
        cairo_move_to(cr, x_offset,
                      (1.0f - fp[j]) * priv->real_draw_height);
        gdk_cairo_set_source_color(cr, &(priv->colors[j]));

        i = 0;
        for (li = g_list_next(li); li != NULL; li = g_list_next(li)) {
            GList *lli = g_list_previous(li);
            float *lfp = (float *) lli->data;
            fp = (float *) li->data;

            i++;

            if (fp[j] == -1.0f)
                continue;

            cairo_curve_to(cr,
                           x_offset - ((i - 0.5f) * priv->graph_delx),
                           (1.0f - lfp[j]) * priv->real_draw_height + 3.5f,
                           x_offset - ((i - 0.5f) * priv->graph_delx),
                           (1.0f - fp[j]) * priv->real_draw_height + 3.5f,
                           x_offset - (i * priv->graph_delx),
                           (1.0f - fp[j]) * priv->real_draw_height + 3.5f);
        }
        cairo_stroke(cr);
    }

    cairo_destroy(cr);

    return TRUE;
}

void trg_torrent_graph_stop(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    priv->draw = FALSE;
}

static void trg_torrent_graph_dispose(GObject * object)
{
    TrgTorrentGraph *g = TRG_TORRENT_GRAPH(object);
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    trg_torrent_graph_stop(g);

    if (priv->timer_index)
        g_source_remove(priv->timer_index);

    trg_torrent_graph_clear_background(g);

    G_OBJECT_CLASS(trg_torrent_graph_parent_class)->dispose(object);
}

void trg_torrent_graph_start(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    if (!priv->timer_index) {
        trg_torrent_graph_update(g);

        priv->timer_index =
            g_timeout_add(priv->speed / priv->frames_per_unit,
                          trg_torrent_graph_update, g);
    }

    priv->draw = TRUE;
}

static unsigned get_max_value_element(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);
    unsigned r = 0;
    int i;

    for (i = 0; i < GRAPH_NUM_POINTS; i++)
        if (priv->values[i] > r)
            r = priv->values[i];

    return r;
}

static void trg_torrent_graph_update_net(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    unsigned max, new_max, bak_max, pow2, base10, coef10, factor10,
        num_bars;

    float scale;
    char speed[32];
    gchar *labelMarkup;
    GList *li;

    float *fp = (float *) (priv->points->data);

    GList *last = g_list_last(priv->points);
    float *lastData = last->data;

    priv->points = g_list_delete_link(priv->points, last);
    priv->points = g_list_prepend(priv->points, lastData);

    fp[0] = 1.0f * priv->out / priv->max;
    fp[1] = 1.0f * priv->in / priv->max;

    trg_strlspeed(speed, (gint64) (priv->out / disk_K));
    labelMarkup =
        g_markup_printf_escaped("<span font_size=\"small\" color=\""
                                GRAPH_OUT_COLOR "\">%s: %s</span>",
                                _("Total Uploading"), speed);
    gtk_label_set_markup(GTK_LABEL(priv->label_out), labelMarkup);
    g_free(labelMarkup);

    trg_strlspeed(speed, (gint64) (priv->in / 1024));
    labelMarkup =
        g_markup_printf_escaped("<span font_size=\"small\" color=\""
                                GRAPH_IN_COLOR "\">%s: %s</span>",
                                _("Total Downloading"), speed);
    gtk_label_set_markup(GTK_LABEL(priv->label_in), labelMarkup);
    g_free(labelMarkup);

    max = MAX(priv->in, priv->out);
    priv->values[priv->cur] = max;
    priv->cur = (priv->cur + 1) % GRAPH_NUM_POINTS;

    if (max >= priv->max)
        new_max = max;
    else
        new_max = get_max_value_element(g);

    bak_max = new_max;
    new_max = 1.1 * new_max;
    new_max = MAX(new_max, 1024U);
    pow2 = floor(log2(new_max));
    base10 = pow2 / 10;
    coef10 = ceil(new_max / (double) (1UL << (base10 * 10)));
    factor10 = pow(10.0, floor(log10(coef10)));
    coef10 = ceil(coef10 / (double) (factor10)) * factor10;

    num_bars = trg_torrent_graph_get_num_bars(g);

    if (coef10 % num_bars != 0)
        coef10 = coef10 + (num_bars - coef10 % num_bars);

    new_max = coef10 * (1UL << (base10 * 10));

    if (bak_max > new_max) {
        new_max = bak_max;
    }

    if ((0.8 * priv->max) < new_max && new_max <= priv->max)
        return;

    scale = 1.0f * priv->max / new_max;

    for (li = priv->points; li != NULL; li = g_list_next(li)) {
        float *fp = (float *) li->data;
        if (fp[0] >= 0.0f) {
            fp[0] *= scale;
            fp[1] *= scale;
        }
    }

    priv->max = new_max;

    trg_torrent_graph_clear_background(g);
}

static GObject *trg_torrent_graph_constructor(GType type,
                                              guint
                                              n_construct_properties,
                                              GObjectConstructParam *
                                              construct_params)
{
    GObject *object;
    TrgTorrentGraphPrivate *priv;
    GtkWidget *hbox;
    int i;

    object =
        G_OBJECT_CLASS
        (trg_torrent_graph_parent_class)->constructor(type,
                                                      n_construct_properties,
                                                      construct_params);
    priv = TRG_TORRENT_GRAPH_GET_PRIVATE(object);

    priv->draw_width = 0;
    priv->draw_height = 0;
    priv->render_counter = 0;
    priv->graph_dely = 0;
    priv->real_draw_height = 0;
    priv->graph_delx = 0.0;
    priv->graph_buffer_offset = 0;
    priv->disp = NULL;
    priv->background = NULL;
    priv->timer_index = 0;
    priv->draw = FALSE;

    priv->fontsize = 8.0;
    priv->frames_per_unit = 10;
    priv->rmargin = 3.5 * priv->fontsize;
    priv->indent = 24.0;
    priv->out = 0;
    priv->in = 0;
    priv->cur = 0;

    priv->speed = 1000;
    priv->max = 1024;

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    priv->label_in = gtk_label_new(NULL);
    priv->label_out = gtk_label_new(NULL);

    gtk_box_pack_start(GTK_BOX(hbox), priv->label_in, FALSE, FALSE, 65);
    gtk_box_pack_start(GTK_BOX(hbox), priv->label_out, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(object), hbox, FALSE, FALSE, 2);

    gdk_color_parse(GRAPH_OUT_COLOR, &priv->colors[0]);
    gdk_color_parse(GRAPH_IN_COLOR, &priv->colors[1]);

    priv->timer_index = 0;
    priv->render_counter = (priv->frames_per_unit - 1);
    priv->draw = FALSE;

    gtk_box_set_homogeneous(GTK_BOX(object), FALSE);

    priv->disp = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(priv->disp), "draw",
                     G_CALLBACK(trg_torrent_graph_draw), object);
    g_signal_connect(G_OBJECT(priv->disp), "configure_event",
                     G_CALLBACK(trg_torrent_graph_configure), object);

    gtk_widget_set_events(priv->disp, GDK_EXPOSURE_MASK);

    gtk_box_pack_start(GTK_BOX(object), priv->disp, TRUE, TRUE, 0);

    priv->points = NULL;
    for (i = 0; i < GRAPH_NUM_DATA_BLOCK_ELEMENTS; i++) {
        priv->data_block[i] = -1.0f;
        if (i % GRAPH_NUM_LINES == 0)
            priv->points =
                g_list_append(priv->points, &priv->data_block[i]);
    }

    return object;
}

static void trg_torrent_graph_class_init(TrgTorrentGraphClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgTorrentGraphPrivate));

    object_class->get_property = trg_torrent_graph_get_property;
    object_class->set_property = trg_torrent_graph_set_property;
    object_class->dispose = trg_torrent_graph_dispose;
    object_class->constructor = trg_torrent_graph_constructor;
}

static void trg_torrent_graph_init(TrgTorrentGraph * self)
{
}

TrgTorrentGraph *trg_torrent_graph_new(GtkStyle * style)
{
    GObject *obj = g_object_new(TRG_TYPE_TORRENT_GRAPH, NULL);
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(obj);

    priv->style = style;

    return TRG_TORRENT_GRAPH(obj);
}

void trg_torrent_graph_queue_draw(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    gtk_widget_queue_draw(priv->disp);
}

void trg_torrent_graph_clear_background(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);

    if (priv->background) {
        cairo_surface_destroy (priv->background);
        priv->background = NULL;
    }
}


static gboolean trg_torrent_graph_update(gpointer user_data)
{
    TrgTorrentGraph *g = TRG_TORRENT_GRAPH(user_data);
    TrgTorrentGraphPrivate *priv =
        TRG_TORRENT_GRAPH_GET_PRIVATE(user_data);

    if (priv->render_counter == priv->frames_per_unit - 1)
        trg_torrent_graph_update_net(g);

    if (priv->draw)
        trg_torrent_graph_queue_draw(g);

    priv->render_counter++;

    if (priv->render_counter >= priv->frames_per_unit)
        priv->render_counter = 0;

    return TRUE;
}

unsigned trg_torrent_graph_get_num_bars(TrgTorrentGraph * g)
{
    TrgTorrentGraphPrivate *priv = TRG_TORRENT_GRAPH_GET_PRIVATE(g);
    unsigned n;

    switch ((int) (priv->draw_height / (priv->fontsize + 14))) {
    case 0:
    case 1:
        n = 1;
        break;
    case 2:
    case 3:
        n = 2;
        break;
    case 4:
        n = 4;
        break;
    default:
        n = 5;
        break;
    }

    return n;
}

#endif
