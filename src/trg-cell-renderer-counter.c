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

#include <gtk/gtk.h>

#include "trg-cell-renderer-counter.h"

enum {
    PROP_0,
    PROP_STATE_LABEL,
    PROP_STATE_COUNT,
    N_PROPS,
};

struct _TrgCellRendererCounter {
    GtkCellRendererText parent;
};

typedef struct {
    gint count;
    gchar *originalLabel;
} TrgCellRendererCounterPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(TrgCellRendererCounter, trg_cell_renderer_counter,
                           GTK_TYPE_CELL_RENDERER_TEXT)

static void trg_cell_renderer_counter_get_property(GObject *object, guint property_id,
                                                   GValue *value, GParamSpec *pspec)
{
    TrgCellRendererCounterPrivate *priv
        = trg_cell_renderer_counter_get_instance_private(TRG_CELL_RENDERER_COUNTER(object));
    switch (property_id) {
    case PROP_STATE_COUNT:
        g_value_set_int(value, priv->count);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_cell_renderer_counter_refresh(TrgCellRendererCounter *cr)
{
    TrgCellRendererCounterPrivate *priv = trg_cell_renderer_counter_get_instance_private(cr);
    if (priv->originalLabel && priv->count > 0) {
        gchar *counterLabel = g_markup_printf_escaped("%s <span size=\"small\">(%d)</span>",
                                                      priv->originalLabel, priv->count);
        g_object_set(cr, "markup", counterLabel, NULL);
        g_free(counterLabel);
    } else {
        g_object_set(cr, "text", priv->originalLabel, NULL);
    }
}

static void trg_cell_renderer_counter_set_property(GObject *object, guint property_id,
                                                   const GValue *value, GParamSpec *pspec)
{
    TrgCellRendererCounterPrivate *priv
        = trg_cell_renderer_counter_get_instance_private(TRG_CELL_RENDERER_COUNTER(object));

    if (property_id == PROP_STATE_LABEL) {
        g_free(priv->originalLabel);
        priv->originalLabel = g_strdup(g_value_get_string(value));
        trg_cell_renderer_counter_refresh(TRG_CELL_RENDERER_COUNTER(object));
    } else if (property_id == PROP_STATE_COUNT) {
        gint newCount = g_value_get_int(value);
        if (priv->count != newCount) {
            priv->count = newCount;
            trg_cell_renderer_counter_refresh(TRG_CELL_RENDERER_COUNTER(object));
        }
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_counter_dispose(GObject *object)
{
    TrgCellRendererCounterPrivate *priv
        = trg_cell_renderer_counter_get_instance_private(TRG_CELL_RENDERER_COUNTER(object));
    g_free(priv->originalLabel);
    G_OBJECT_CLASS(trg_cell_renderer_counter_parent_class)->dispose(object);
}

static void trg_cell_renderer_counter_class_init(TrgCellRendererCounterClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_cell_renderer_counter_get_property;
    object_class->set_property = trg_cell_renderer_counter_set_property;
    object_class->dispose = trg_cell_renderer_counter_dispose;

    g_object_class_install_property(object_class, PROP_STATE_COUNT,
                                    g_param_spec_int("state-count", "State Count", "State Count",
                                                     -1, INT_MAX, -1,
                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(
        object_class, PROP_STATE_LABEL,
        g_param_spec_string("state-label", "State Label", "State Label", NULL,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void trg_cell_renderer_counter_init(TrgCellRendererCounter *self)
{
}

GtkCellRenderer *trg_cell_renderer_counter_new(void)
{
    return g_object_new(TRG_TYPE_CELL_RENDERER_COUNTER, NULL);
}
