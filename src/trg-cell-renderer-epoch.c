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
 * GNU General Public License for more depochils.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <gtk/gtk.h>
#include <stdint.h>
#include <time.h>

#include "trg-cell-renderer-epoch.h"
#include "util.h"

enum {
    PROP_0,
    PROP_EPOCH_VALUE
};

G_DEFINE_TYPE(TrgCellRendererEpoch, trg_cell_renderer_epoch, GTK_TYPE_CELL_RENDERER_TEXT)
#define TRG_CELL_RENDERER_EPOCH_GET_PRIVATE(o)                                                     \
    (G_TYPE_INSTANCE_GET_PRIVATE((o), TRG_TYPE_CELL_RENDERER_EPOCH, TrgCellRendererEpochPrivate))
typedef struct _TrgCellRendererEpochPrivate TrgCellRendererEpochPrivate;

struct _TrgCellRendererEpochPrivate {
    gdouble epoch_value;
};

static void trg_cell_renderer_epoch_get_property(GObject *object, guint property_id, GValue *value,
                                                 GParamSpec *pspec)
{
    TrgCellRendererEpochPrivate *priv = TRG_CELL_RENDERER_EPOCH_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_EPOCH_VALUE:
        g_value_set_int64(value, priv->epoch_value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_cell_renderer_epoch_set_property(GObject *object, guint property_id,
                                                 const GValue *value, GParamSpec *pspec)
{
    TrgCellRendererEpochPrivate *priv = TRG_CELL_RENDERER_EPOCH_GET_PRIVATE(object);

    if (property_id == PROP_EPOCH_VALUE) {
        gint64 new_value = g_value_get_int64(value);
        if (priv->epoch_value != new_value) {
            if (new_value > 0) {
                gchar *timestring = epoch_to_string(new_value);
                g_object_set(object, "text", timestring, NULL);
                g_free(timestring);
            } else {
                g_object_set(object, "text", "", NULL);
            }
            priv->epoch_value = new_value;
        }
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_epoch_class_init(TrgCellRendererEpochClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_cell_renderer_epoch_get_property;
    object_class->set_property = trg_cell_renderer_epoch_set_property;

    g_object_class_install_property(
        object_class, PROP_EPOCH_VALUE,
        g_param_spec_int64("epoch-value", "Epoch Value", "Epoch Value", G_MININT64, G_MAXINT64, 0,
                           G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                               | G_PARAM_STATIC_BLURB));

    g_type_class_add_private(klass, sizeof(TrgCellRendererEpochPrivate));
}

static void trg_cell_renderer_epoch_init(TrgCellRendererEpoch *self G_GNUC_UNUSED)
{
}

GtkCellRenderer *trg_cell_renderer_epoch_new(void)
{
    return GTK_CELL_RENDERER(g_object_new(TRG_TYPE_CELL_RENDERER_EPOCH, NULL));
}
