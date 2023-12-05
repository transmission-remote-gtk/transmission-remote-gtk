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
#include <limits.h>

#include "trg-cell-renderer-eta.h"
#include "trg-cell-renderer-ratio.h"
#include "util.h"

enum {
    PROP_0,
    PROP_RATIO_VALUE
};

struct _TrgCellRendererRatio {
    GtkCellRendererText parent;

    gdouble ratio_value;
};

G_DEFINE_TYPE(TrgCellRendererRatio, trg_cell_renderer_ratio, GTK_TYPE_CELL_RENDERER_TEXT)

static void trg_cell_renderer_ratio_get_property(GObject *object, guint property_id, GValue *value,
                                                 GParamSpec *pspec)
{
    TrgCellRendererRatio *self = TRG_CELL_RENDERER_RATIO(object);
    switch (property_id) {
    case PROP_RATIO_VALUE:
        g_value_set_double(value, self->ratio_value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_ratio_set_property(GObject *object, guint property_id,
                                                 const GValue *value, GParamSpec *pspec)
{
    TrgCellRendererRatio *self = TRG_CELL_RENDERER_RATIO(object);
    if (property_id == PROP_RATIO_VALUE) {
        self->ratio_value = g_value_get_double(value);
        if (self->ratio_value > 0) {
            char ratioString[32];
            trg_strlratio(ratioString, self->ratio_value);
            g_object_set(object, "text", ratioString, NULL);
        } else {
            g_object_set(object, "text", "", NULL);
        }
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_ratio_class_init(TrgCellRendererRatioClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_cell_renderer_ratio_get_property;
    object_class->set_property = trg_cell_renderer_ratio_set_property;

    g_object_class_install_property(
        object_class, PROP_RATIO_VALUE,
        g_param_spec_double("ratio-value", "Ratio Value", "Ratio Value", 0, DBL_MAX, 0,
                            G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                                | G_PARAM_STATIC_BLURB));
}

static void trg_cell_renderer_ratio_init(TrgCellRendererRatio *self)
{
    g_object_set(self, "xalign", 1.0f, NULL);
}

GtkCellRenderer *trg_cell_renderer_ratio_new(void)
{
    return GTK_CELL_RENDERER(g_object_new(TRG_TYPE_CELL_RENDERER_RATIO, NULL));
}
