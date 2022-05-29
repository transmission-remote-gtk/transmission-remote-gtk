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
#include <stdint.h>

#include "trg-cell-renderer-numgteqthan.h"
#include "util.h"

enum {
    PROP_0,
    PROP_VALUE_VALUE,
    PROP_MINVALUE
};

G_DEFINE_TYPE(TrgCellRendererNumGtEqThan, trg_cell_renderer_numgteqthan,
              GTK_TYPE_CELL_RENDERER_TEXT)
#define TRG_CELL_RENDERER_NUMGTEQTHAN_GET_PRIVATE(o)                                               \
    (G_TYPE_INSTANCE_GET_PRIVATE((o), TRG_TYPE_CELL_RENDERER_NUMGTEQTHAN,                          \
                                 TrgCellRendererNumGtEqThanPrivate))
typedef struct _TrgCellRendererNumGtEqThanPrivate TrgCellRendererNumGtEqThanPrivate;

struct _TrgCellRendererNumGtEqThanPrivate {
    gint64 value_value;
    gint64 minvalue;
};

static void trg_cell_renderer_numgteqthan_get_property(GObject *object, guint property_id,
                                                       GValue *value, GParamSpec *pspec)
{
    TrgCellRendererNumGtEqThanPrivate *priv = TRG_CELL_RENDERER_NUMGTEQTHAN_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_VALUE_VALUE:
        g_value_set_int64(value, priv->value_value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_cell_renderer_numgteqthan_set_property(GObject *object, guint property_id,
                                                       const GValue *value, GParamSpec *pspec)
{
    TrgCellRendererNumGtEqThanPrivate *priv = TRG_CELL_RENDERER_NUMGTEQTHAN_GET_PRIVATE(object);
    if (property_id == PROP_VALUE_VALUE) {
        priv->value_value = g_value_get_int64(value);
        if (priv->value_value >= priv->minvalue) {
            gchar size_text[32];
            g_snprintf(size_text, sizeof(size_text), "%" G_GINT64_FORMAT, priv->value_value);
            g_object_set(object, "text", size_text, NULL);
        } else {
            g_object_set(object, "text", "", NULL);
        }
    } else if (property_id == PROP_MINVALUE) {
        priv->minvalue = g_value_get_int64(value);
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_numgteqthan_class_init(TrgCellRendererNumGtEqThanClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_cell_renderer_numgteqthan_get_property;
    object_class->set_property = trg_cell_renderer_numgteqthan_set_property;

    g_object_class_install_property(
        object_class, PROP_VALUE_VALUE,
        g_param_spec_int64("value", "Value", "Value", G_MININT64, G_MAXINT64, 0,
                           G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                               | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(
        object_class, PROP_MINVALUE,
        g_param_spec_int64("minvalue", "Min Value", "Min Value", G_MININT64, G_MAXINT64, 1,
                           G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                               | G_PARAM_STATIC_BLURB));

    g_type_class_add_private(klass, sizeof(TrgCellRendererNumGtEqThanPrivate));
}

static void trg_cell_renderer_numgteqthan_init(TrgCellRendererNumGtEqThan *self)
{
    g_object_set(self, "xalign", 1.0f, NULL);
}

GtkCellRenderer *trg_cell_renderer_numgteqthan_new(gint64 minvalue)
{
    return GTK_CELL_RENDERER(
        g_object_new(TRG_TYPE_CELL_RENDERER_NUMGTEQTHAN, "minvalue", minvalue, NULL));
}
