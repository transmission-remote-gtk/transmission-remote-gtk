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
#include <gtk/gtk.h>
#include <stdint.h>

#include "protocol-constants.h"
#include "trg-cell-renderer-priority.h"
#include "trg-files-model.h"
#include "util.h"

enum {
    PROP_0,
    PROP_PRIORITY_VALUE
};

G_DEFINE_TYPE(TrgCellRendererPriority, trg_cell_renderer_priority, GTK_TYPE_CELL_RENDERER_TEXT)
#define TRG_CELL_RENDERER_PRIORITY_GET_PRIVATE(o)                                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE((o), TRG_TYPE_CELL_RENDERER_PRIORITY,                             \
                                 TrgCellRendererPriorityPrivate))
typedef struct _TrgCellRendererPriorityPrivate TrgCellRendererPriorityPrivate;

struct _TrgCellRendererPriorityPrivate {
    gint64 priority_value;
};

static void trg_cell_renderer_priority_get_property(GObject *object, guint property_id,
                                                    GValue *value, GParamSpec *pspec)
{
    TrgCellRendererPriorityPrivate *priv = TRG_CELL_RENDERER_PRIORITY_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_PRIORITY_VALUE:
        g_value_set_int64(value, priv->priority_value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_cell_renderer_priority_set_property(GObject *object, guint property_id,
                                                    const GValue *value, GParamSpec *pspec)
{
    TrgCellRendererPriorityPrivate *priv = TRG_CELL_RENDERER_PRIORITY_GET_PRIVATE(object);

    if (property_id == PROP_PRIORITY_VALUE) {
        priv->priority_value = g_value_get_int(value);
        if (priv->priority_value == TR_PRI_LOW) {
            g_object_set(object, "text", _("Low"), NULL);
        } else if (priv->priority_value == TR_PRI_HIGH) {
            g_object_set(object, "text", _("High"), NULL);
        } else if (priv->priority_value == TR_PRI_NORMAL) {
            g_object_set(object, "text", _("Normal"), NULL);
        } else if (priv->priority_value == TR_PRI_MIXED) {
            g_object_set(object, "text", _("Mixed"), NULL);
        } else {
            g_object_set(object, "text", "", NULL);
        }
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_priority_class_init(TrgCellRendererPriorityClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_cell_renderer_priority_get_property;
    object_class->set_property = trg_cell_renderer_priority_set_property;

    g_object_class_install_property(
        object_class, PROP_PRIORITY_VALUE,
        g_param_spec_int("priority-value", "Priority Value", "Priority Value", TR_PRI_UNSET,
                         TR_PRI_HIGH, TR_PRI_NORMAL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                             | G_PARAM_STATIC_BLURB));

    g_type_class_add_private(klass, sizeof(TrgCellRendererPriorityPrivate));
}

static void trg_cell_renderer_priority_init(TrgCellRendererPriority *self)
{
}

GtkCellRenderer *trg_cell_renderer_priority_new(void)
{
    return GTK_CELL_RENDERER(g_object_new(TRG_TYPE_CELL_RENDERER_PRIORITY, NULL));
}
