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

#include "trg-cell-renderer-file-icon.h"
#include "util.h"

enum {
    PROP_0,
    PROP_FILE_ID,
    PROP_FILE_NAME
};

struct _TrgCellRendererFileIcon {
    GtkCellRendererPixbuf parent;

    gint64 file_id;
    gchar *text;
};

G_DEFINE_TYPE(TrgCellRendererFileIcon, trg_cell_renderer_file_icon, GTK_TYPE_CELL_RENDERER_PIXBUF)

static void trg_cell_renderer_file_icon_get_property(GObject *object, guint property_id,
                                                     GValue *value, GParamSpec *pspec)
{
    TrgCellRendererFileIcon *self = TRG_CELL_RENDERER_FILE_ICON(object);
    switch (property_id) {
    case PROP_FILE_ID:
        g_value_set_int64(value, self->file_id);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_cell_renderer_file_icon_refresh(TrgCellRendererFileIcon *fi)
{
    if (fi->file_id == -2) {
        return;
    } else if (fi->file_id == -1) {
        g_object_set(fi, "icon-name", "folder", NULL);
    } else if (fi->text) {
#ifndef G_OS_WIN32
        gboolean uncertain;
        gchar *mimetype = g_content_type_guess(fi->text, NULL, 0, &uncertain);
        GIcon *icon = NULL;

        if (!uncertain && mimetype)
            icon = g_content_type_get_icon(mimetype);

        g_free(mimetype);

        if (icon) {
            g_object_set(fi, "gicon", icon, NULL);
            g_object_unref(icon);
        } else {
            g_object_set(fi, "icon-name", "file", NULL);
        }
#else
        g_object_set(fi, "icon-name", "file", NULL);
#endif
    }
}

static void trg_cell_renderer_file_icon_set_property(GObject *object, guint property_id,
                                                     const GValue *value, GParamSpec *pspec)
{
    TrgCellRendererFileIcon *self = TRG_CELL_RENDERER_FILE_ICON(object);
    if (property_id == PROP_FILE_ID) {
        self->file_id = g_value_get_int64(value);
        trg_cell_renderer_file_icon_refresh(TRG_CELL_RENDERER_FILE_ICON(object));
    } else if (property_id == PROP_FILE_NAME) {
        if (self->file_id != -1) {
            g_free(self->text);
            self->text = g_strdup(g_value_get_string(value));
            trg_cell_renderer_file_icon_refresh(TRG_CELL_RENDERER_FILE_ICON(object));
        }
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void trg_cell_renderer_file_icon_dispose(GObject *object)
{
    TrgCellRendererFileIcon *self = TRG_CELL_RENDERER_FILE_ICON(object);
    g_free(self->text);
    G_OBJECT_CLASS(trg_cell_renderer_file_icon_parent_class)->dispose(object);
}

static void trg_cell_renderer_file_icon_class_init(TrgCellRendererFileIconClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_cell_renderer_file_icon_get_property;
    object_class->set_property = trg_cell_renderer_file_icon_set_property;
    object_class->dispose = trg_cell_renderer_file_icon_dispose;

    g_object_class_install_property(
        object_class, PROP_FILE_ID,
        g_param_spec_int64("file-id", "File ID", "File ID", -2, G_MAXINT64, -2,
                           G_PARAM_READWRITE | G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
                               | G_PARAM_STATIC_BLURB));

    g_object_class_install_property(object_class, PROP_FILE_NAME,
                                    g_param_spec_string("file-name", "Filename", "Filename", NULL,
                                                        G_PARAM_READWRITE | G_PARAM_STATIC_NAME
                                                            | G_PARAM_STATIC_NICK
                                                            | G_PARAM_STATIC_BLURB));
}

static void trg_cell_renderer_file_icon_init(TrgCellRendererFileIcon *self)
{
}

GtkCellRenderer *trg_cell_renderer_file_icon_new(void)
{
    return GTK_CELL_RENDERER(g_object_new(TRG_TYPE_CELL_RENDERER_FILE_ICON, NULL));
}
