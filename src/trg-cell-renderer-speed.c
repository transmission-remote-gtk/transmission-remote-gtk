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

#include <stdint.h>
#include <gtk/gtk.h>

#include "trg-cell-renderer-speed.h"
#include "util.h"

enum {
    PROP_0, PROP_SPEED_VALUE
};

G_DEFINE_TYPE(TrgCellRendererSpeed, trg_cell_renderer_speed,
              GTK_TYPE_CELL_RENDERER_TEXT)
#define TRG_CELL_RENDERER_SPEED_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_CELL_RENDERER_SPEED, TrgCellRendererSpeedPrivate))
typedef struct _TrgCellRendererSpeedPrivate TrgCellRendererSpeedPrivate;

struct _TrgCellRendererSpeedPrivate {
    gint64 speed_value;
};

static void trg_cell_renderer_speed_get_property(GObject * object,
                                                 guint property_id,
                                                 GValue * value,
                                                 GParamSpec * pspec)
{
    TrgCellRendererSpeedPrivate *priv =
        TRG_CELL_RENDERER_SPEED_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_SPEED_VALUE:
        g_value_set_int64(value, priv->speed_value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_cell_renderer_speed_set_property(GObject * object,
                                                 guint property_id,
                                                 const GValue * value,
                                                 GParamSpec * pspec)
{
    TrgCellRendererSpeedPrivate *priv =
        TRG_CELL_RENDERER_SPEED_GET_PRIVATE(object);
    if (property_id == PROP_SPEED_VALUE) {
        gint64 new_value = g_value_get_int64(value);
        if (new_value != priv->speed_value) {
            if (new_value > 0) {
                char speedString[32];
                trg_strlspeed(speedString, new_value / disk_K);
                g_object_set(object, "text", speedString, NULL);
            } else {
                g_object_set(object, "text", "", NULL);
            }
            priv->speed_value = new_value;
        }
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_speed_class_init(TrgCellRendererSpeedClass *
                                               klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_cell_renderer_speed_get_property;
    object_class->set_property = trg_cell_renderer_speed_set_property;

    g_object_class_install_property(object_class,
                                    PROP_SPEED_VALUE,
                                    g_param_spec_int64("speed-value",
                                                       "Speed Value",
                                                       "Speed Value",
                                                       0,
                                                       G_MAXINT64,
                                                       0,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_NAME
                                                       |
                                                       G_PARAM_STATIC_NICK
                                                       |
                                                       G_PARAM_STATIC_BLURB));

    g_type_class_add_private(klass, sizeof(TrgCellRendererSpeedPrivate));
}

static void trg_cell_renderer_speed_init(TrgCellRendererSpeed * self)
{
    g_object_set(self, "xalign", 1.0f, NULL);
}

GtkCellRenderer *trg_cell_renderer_speed_new(void)
{
    return GTK_CELL_RENDERER(g_object_new
                             (TRG_TYPE_CELL_RENDERER_SPEED, NULL));
}
