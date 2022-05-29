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

#include "trg-cell-renderer-eta.h"
#include "util.h"

enum {
    PROP_0,
    PROP_ETA_VALUE
};

G_DEFINE_TYPE(TrgCellRendererEta, trg_cell_renderer_eta, GTK_TYPE_CELL_RENDERER_TEXT)
#define TRG_CELL_RENDERER_ETA_GET_PRIVATE(o)                                                       \
    (G_TYPE_INSTANCE_GET_PRIVATE((o), TRG_TYPE_CELL_RENDERER_ETA, TrgCellRendererEtaPrivate))
typedef struct _TrgCellRendererEtaPrivate TrgCellRendererEtaPrivate;

struct _TrgCellRendererEtaPrivate {
    gdouble eta_value;
};

static void trg_cell_renderer_eta_get_property(GObject *object, guint property_id, GValue *value,
                                               GParamSpec *pspec)
{
    TrgCellRendererEtaPrivate *priv = TRG_CELL_RENDERER_ETA_GET_PRIVATE(object);
    switch (property_id) {
    case PROP_ETA_VALUE:
        g_value_set_int64(value, priv->eta_value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_cell_renderer_eta_set_property(GObject *object, guint property_id,
                                               const GValue *value, GParamSpec *pspec)
{
    TrgCellRendererEtaPrivate *priv = TRG_CELL_RENDERER_ETA_GET_PRIVATE(object);

    if (property_id == PROP_ETA_VALUE) {
        priv->eta_value = g_value_get_int64(value);
        if (priv->eta_value > 0) {
            char etaString[32];
            tr_strltime_short(etaString, priv->eta_value, sizeof(etaString));
            g_object_set(object, "text", etaString, NULL);
        } else if (priv->eta_value == -2) {
            g_object_set(object, "text", "∞", NULL);
        } else {
            g_object_set(object, "text", "", NULL);
        }
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_eta_class_init(TrgCellRendererEtaClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_cell_renderer_eta_get_property;
    object_class->set_property = trg_cell_renderer_eta_set_property;

    g_object_class_install_property(
        object_class, PROP_ETA_VALUE,
        g_param_spec_int64("eta-value", "Eta Value", "Eta Value", G_MININT64, G_MAXINT64, 0,
                           G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                               | G_PARAM_STATIC_BLURB));

    g_type_class_add_private(klass, sizeof(TrgCellRendererEtaPrivate));
}

static void trg_cell_renderer_eta_init(TrgCellRendererEta *self G_GNUC_UNUSED)
{
}

GtkCellRenderer *trg_cell_renderer_eta_new(void)
{
    return GTK_CELL_RENDERER(g_object_new(TRG_TYPE_CELL_RENDERER_ETA, NULL));
}
