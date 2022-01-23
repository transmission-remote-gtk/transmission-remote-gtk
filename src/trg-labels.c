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

#include "trg-labels.h"
#include "util.h"
#include <glib.h>
#include <gtk/gtk.h>

/* TrgLabelsBox */
struct _TrgLabelsBox {
    GtkBox parent;
    GtkEntry *labels_entry;
    GtkLabel *errors_label;
    GRegex *labels_regex;
};
G_DEFINE_TYPE(TrgLabelsBox, trg_labels_box, GTK_TYPE_BOX);
static void trg_labels_box_init(TrgLabelsBox *tv)
{
    tv->labels_regex = g_regex_new("^([^,]+,?)+?[^,]+$", G_REGEX_EXTENDED, 0, NULL);
    tv->labels_entry = GTK_ENTRY(gtk_entry_new());
    tv->errors_label = GTK_LABEL(gtk_label_new(""));

    gtk_box_pack_start(GTK_BOX(tv), GTK_WIDGET(tv->labels_entry), TRUE, TRUE, 5);
    gtk_box_pack_end(GTK_BOX(tv), GTK_WIDGET(tv->errors_label), TRUE, TRUE, 0);
};

static void trg_labels_box_class_init(TrgLabelsBoxClass *klass) {};

gboolean trg_labels_box_is_valid(TrgLabelsBox *trglb)
{
    return g_strcmp0("", gtk_label_get_label(trglb->errors_label)) == 0;
}

GList *trg_labels_box_get_labels(TrgLabelsBox *trglb)
{
    if (!trg_labels_box_is_valid(trglb))
        return NULL;

    GtkEntryBuffer *buf = gtk_entry_get_buffer(trglb->labels_entry);

    return tr_split_to_list(",", gtk_entry_buffer_get_text(buf));
}

static void check_regex(TrgLabelsBox *labels_box)
{
    const gchar *buf_text;
    GtkEntryBuffer *entry_buf;

    entry_buf = gtk_entry_get_buffer(labels_box->labels_entry);
    buf_text = gtk_entry_buffer_get_text(entry_buf);

    if (g_regex_match(labels_box->labels_regex, buf_text, 0, NULL)) {
        gtk_label_set_label(labels_box->errors_label, "");
        return;
    };

    gtk_label_set_markup(labels_box->errors_label,
                         "<span color=\"red\" weight=\"bold\">"
                         "Labels must be comma separated string"
                         "</span>");
}

GtkWidget *trg_labels_box_new(void)
{
    TrgLabelsBox *self;
    self = g_object_new(TRG_TYPE_LABELS_BOX, "orientation", GTK_ORIENTATION_VERTICAL, NULL);

    /* check regex when leaving focus */
    g_signal_connect_swapped(GTK_WIDGET(self->labels_entry), "focus-out-event",
                             G_CALLBACK(check_regex), self);

    return GTK_WIDGET(self);
}

/* TrgLabelsDialog */
struct _TrgLabelsDialog {
    TrgLabelsBox *labels_box;
    TrgClient *client;
    TrgMainWindow *win;
};
G_DEFINE_TYPE(TrgLabelsDialog, trg_labels_dialog, GTK_TYPE_DIALOG);

static void trg_labels_dialog_init(TrgLabelsDialog *ld)
{
    gtk_widget_init_template(GTK_WIDGET(ld));

    ld->labels_box = TRG_LABELS_BOX(trg_labels_box_new());
}

static void trg_labels_dialog_class_init(TrgLabelsDialogClass *class)
{
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
                                                RESOURCE_PATH "/ui/labels-dialog.ui");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), TrgLabelsDialog, labels_box);
}

static void on_response_cb(TrgLabelsDialog *dlg, gint res_id, gpointer data)
{
}
TrgLabelsDialog *trg_labels_dialog_new(TrgMainWindow *win, TrgClient *client)
{
    TrgLabelsDialog *self;
    self = TRG_LABELS_DIALOG(g_object_new(TRG_TYPE_LABELS_DIALOG, NULL));

    self->client = client;
    self->win = win;

    g_signal_connect(G_OBJECT(self), "response", G_CALLBACK(on_response_cb), win);
    return TRG_LABELS_DIALOG(self);
}
