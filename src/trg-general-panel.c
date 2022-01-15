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

#include <string.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-client.h"
#include "torrent.h"
#include "util.h"
#include "trg-general-panel.h"
#include "trg-torrent-model.h"
#include "protocol-constants.h"

#define TRG_GENERAL_PANEL_SPACING_X         4
#define TRG_GENERAL_PANEL_SPACING_Y         2
#define TRG_GENERAL_PANEL_COLUMNS           6

static void gtk_label_clear(GtkLabel * l);
static GtkLabel *gen_panel_label_get_key_label(GtkLabel * l);
static GtkLabel *trg_general_panel_add_label(TrgGeneralPanel * gp,
                                             char *key, guint col,
                                             guint row);

G_DEFINE_TYPE(TrgGeneralPanel, trg_general_panel, GTK_TYPE_GRID)
#define TRG_GENERAL_PANEL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_GENERAL_PANEL, TrgGeneralPanelPrivate))
typedef struct _TrgGeneralPanelPrivate TrgGeneralPanelPrivate;

struct _TrgGeneralPanelPrivate {
    GtkLabel *gen_name_label;
    GtkLabel *gen_size_label;
    GtkLabel *gen_completed_label;
    GtkLabel *gen_seeders_label;
    GtkLabel *gen_leechers_label;
    GtkLabel *gen_status_label;
    GtkLabel *gen_priority_label;
    GtkLabel *gen_eta_label;
    GtkLabel *gen_downloaded_label;
    GtkLabel *gen_uploaded_label;
    GtkLabel *gen_corrupted_label;
    GtkLabel *gen_down_rate_label;
    GtkLabel *gen_up_rate_label;
    GtkLabel *gen_ratio_label;
    GtkLabel *gen_limit_label;
    GtkLabel *gen_completedat_label;
    GtkLabel *gen_downloaddir_label;
    GtkLabel *gen_comment_label;
    GtkLabel *gen_hash_label;
    GtkLabel *gen_error_label;
    GtkTreeModel *model;
    TrgClient *tc;
};

void trg_general_panel_clear(TrgGeneralPanel * panel)
{
    TrgGeneralPanelPrivate *priv = TRG_GENERAL_PANEL_GET_PRIVATE(panel);

    gtk_label_clear(priv->gen_name_label);
    gtk_label_clear(priv->gen_size_label);
    gtk_label_clear(priv->gen_completed_label);
    gtk_label_clear(priv->gen_seeders_label);
    gtk_label_clear(priv->gen_leechers_label);
    gtk_label_clear(priv->gen_status_label);
    gtk_label_clear(priv->gen_priority_label);
    gtk_label_clear(priv->gen_eta_label);
    gtk_label_clear(priv->gen_downloaded_label);
    gtk_label_clear(priv->gen_uploaded_label);
    gtk_label_clear(priv->gen_corrupted_label);
    gtk_label_clear(priv->gen_down_rate_label);
    gtk_label_clear(priv->gen_up_rate_label);
    gtk_label_clear(priv->gen_ratio_label);
    gtk_label_clear(priv->gen_limit_label);
    gtk_label_clear(priv->gen_completedat_label);
    gtk_label_clear(priv->gen_downloaddir_label);
    gtk_label_clear(priv->gen_comment_label);
    gtk_label_clear(priv->gen_hash_label);
    gtk_label_clear(priv->gen_error_label);
    gtk_label_clear(gen_panel_label_get_key_label
                    (GTK_LABEL(priv->gen_error_label)));
}

static void gtk_label_clear(GtkLabel * l)
{
    gtk_label_set_text(l, "");
}

static GtkLabel *gen_panel_label_get_key_label(GtkLabel * l)
{
    return GTK_LABEL(g_object_get_data(G_OBJECT(l), "key-label"));
}

static void trg_general_panel_class_init(TrgGeneralPanelClass * klass)
{
    g_type_class_add_private(klass, sizeof(TrgGeneralPanelPrivate));
}

void
trg_general_panel_update(TrgGeneralPanel * panel, JsonObject * t,
                         GtkTreeIter * iter)
{
    TrgGeneralPanelPrivate *priv;
    gchar buf[32], buf1[32]; //TODO: do it better
    gchar *statusString, *fullStatusString, *completedAtString, *speed, *comment, *markup;
    const gchar *errorStr;
    gint64 eta, uploaded, corrupted, haveValid, completedAt;
    GtkLabel *keyLabel;
    gint64 seeders = 0, leechers = 0;

    priv = TRG_GENERAL_PANEL_GET_PRIVATE(panel);

    gtk_tree_model_get(GTK_TREE_MODEL(priv->model), iter,
                       TORRENT_COLUMN_SEEDS, &seeders,
                       TORRENT_COLUMN_LEECHERS, &leechers,
                       TORRENT_COLUMN_STATUS, &statusString, -1);

    trg_strlsize(buf, torrent_get_size_when_done(t));
    gtk_label_set_text(GTK_LABEL(priv->gen_size_label), buf);

	trg_strlspeed(buf, torrent_get_rate_down(t) / disk_K);
	if (torrent_get_download_limited(t)){
		trg_strlspeed(buf1, torrent_get_download_limit(t));
		speed = g_strdup_printf("%s [%s]", buf, buf1);
	} else
		speed = g_strdup_printf("%s", buf);
    gtk_label_set_text(GTK_LABEL(priv->gen_down_rate_label), speed);
    g_free(speed);

	trg_strlspeed(buf, torrent_get_rate_up(t) / disk_K);
    if (torrent_get_upload_limited(t)){
		trg_strlspeed(buf1, torrent_get_upload_limit(t));
		speed = g_strdup_printf("%s [%s]", buf, buf1);	
	} else
		speed = g_strdup_printf("%s", buf);
    gtk_label_set_text(GTK_LABEL(priv->gen_up_rate_label), speed);
	g_free(speed);

	corrupted = torrent_get_corrupted(t);
	trg_strlsize(buf, corrupted);
	gtk_label_set_text(GTK_LABEL(priv->gen_corrupted_label), buf);

    uploaded = torrent_get_uploaded(t);
    trg_strlsize(buf, uploaded);
    gtk_label_set_text(GTK_LABEL(priv->gen_uploaded_label), buf);

    gtk_label_set_text(GTK_LABEL(priv->gen_hash_label), torrent_get_hash(t));

    haveValid = torrent_get_have_valid(t);
    trg_strlsize(buf, torrent_get_downloaded(t));
    gtk_label_set_text(GTK_LABEL(priv->gen_downloaded_label), buf);

    if (uploaded > 0 && haveValid > 0) {
        trg_strlratio(buf, (double) uploaded / (double) haveValid);
        gtk_label_set_text(GTK_LABEL(priv->gen_ratio_label), buf);
    } else {
        gtk_label_set_text(GTK_LABEL(priv->gen_ratio_label), _("N/A"));
    }

	trg_strlratio(buf, torrent_get_seed_ratio_limit(t));
	gtk_label_set_text(GTK_LABEL(priv->gen_limit_label), buf);

    completedAt = torrent_get_done_date(t);
    if (completedAt > 0) {
        completedAtString = epoch_to_string(completedAt);
        gtk_label_set_text(GTK_LABEL(priv->gen_completedat_label),
                           completedAtString);
        g_free(completedAtString);
    } else {
        gtk_label_set_text(GTK_LABEL(priv->gen_completedat_label), "");
    }

    fullStatusString = g_strdup_printf("%s %s", statusString,
                                       torrent_get_is_private(t) ?
                                       _("(Private)") : _("(Public)"));
    gtk_label_set_text(GTK_LABEL(priv->gen_status_label),
                       fullStatusString);
    g_free(fullStatusString);
    g_free(statusString);

	switch(torrent_get_bandwidth_priority(t)){
		case TR_PRI_LOW:
			gtk_label_set_text(GTK_LABEL(priv->gen_priority_label), _("Low"));
			break;
		case TR_PRI_NORMAL:
			gtk_label_set_text(GTK_LABEL(priv->gen_priority_label), _("Normal"));
			break;
		case TR_PRI_HIGH:
			gtk_label_set_text(GTK_LABEL(priv->gen_priority_label), _("High"));
			break;
	}

    trg_strlpercent(buf, torrent_get_percent_done(t));
    gtk_label_set_text(GTK_LABEL(priv->gen_completed_label), buf);

    gtk_label_set_text(GTK_LABEL(priv->gen_name_label),
                       torrent_get_name(t));

    gtk_label_set_text(GTK_LABEL(priv->gen_downloaddir_label),
                       torrent_get_download_dir(t));

    comment = add_links_to_text(torrent_get_comment(t));
    gtk_label_set_markup(GTK_LABEL(priv->gen_comment_label), comment);
    g_free(comment);

    errorStr = torrent_get_errorstr(t);
    keyLabel =
        gen_panel_label_get_key_label(GTK_LABEL(priv->gen_error_label));
    if (strlen(errorStr) > 0) {
        markup =
            g_markup_printf_escaped("<span fgcolor=\"red\">%s</span>",
                                    errorStr);
        gtk_label_set_markup(GTK_LABEL(priv->gen_error_label), markup);
        g_free(markup);

        markup =
            g_markup_printf_escaped
            ("<span font_weight=\"bold\" fgcolor=\"red\">%s</span>",
             _("Error"));
        gtk_label_set_markup(keyLabel, markup);
        g_free(markup);
    } else {
        gtk_label_clear(GTK_LABEL(priv->gen_error_label));
        gtk_label_clear(keyLabel);
    }

    if ((eta = torrent_get_eta(t)) > 0) {
        tr_strltime_long(buf, eta, sizeof(buf));
        gtk_label_set_text(GTK_LABEL(priv->gen_eta_label), buf);
    } else {
        gtk_label_set_text(GTK_LABEL(priv->gen_eta_label), _("N/A"));
    }

    g_snprintf(buf, sizeof(buf), "%" G_GINT64_FORMAT,
             seeders >= 0 ? seeders : 0);
    gtk_label_set_text(GTK_LABEL(priv->gen_seeders_label), buf);
    g_snprintf(buf, sizeof(buf), "%" G_GINT64_FORMAT,
             leechers >= 0 ? leechers : 0);
    gtk_label_set_text(GTK_LABEL(priv->gen_leechers_label), buf);
}

static GtkLabel *trg_general_panel_add_label_with_width(TrgGeneralPanel *
                                                        gp, char *key,
                                                        guint col,
                                                        guint row,
                                                        gint width)
{
    GtkWidget *value, *keyLabel;

    guint startCol = col * 2;
    guint endCol = (guint)(width < 0
                           ? (TRG_GENERAL_PANEL_COLUMNS)
                           : width);

    keyLabel = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(keyLabel), 0.0f);
    gtk_label_set_yalign(GTK_LABEL(keyLabel), 0.0f);
    gtk_label_set_width_chars(GTK_LABEL(keyLabel), 10);

    if (strlen(key) > 0) {
        gchar *keyMarkup =
            g_markup_printf_escaped(strlen(key) > 0 ? "<b>%s:</b>" : "",
                                    key);
        gtk_label_set_markup(GTK_LABEL(keyLabel), keyMarkup);
        g_free(keyMarkup);
    }
    gtk_grid_attach(GTK_GRID(gp), keyLabel,
                    startCol, row,
                    1, 1);

    value = gtk_label_new(NULL);
    gtk_label_set_xalign(GTK_LABEL(value), 0.0f);
    gtk_label_set_yalign(GTK_LABEL(value), 0.0f);
    gtk_label_set_width_chars(GTK_LABEL(value), 10);

    g_object_set_data(G_OBJECT(value), "key-label", keyLabel);
    gtk_label_set_selectable(GTK_LABEL(value), TRUE);
    gtk_grid_attach(GTK_GRID(gp), value,
                    startCol + 1, row,
                    endCol, 1);

    return GTK_LABEL(value);
}

static GtkLabel *trg_general_panel_add_label(TrgGeneralPanel * gp,
                                             char *key, guint col,
                                             guint row)
{
    return trg_general_panel_add_label_with_width(gp, key, col, row, 1);
}

static void trg_general_panel_init(TrgGeneralPanel * self)
{
    TrgGeneralPanelPrivate *priv = TRG_GENERAL_PANEL_GET_PRIVATE(self);
	priv->gen_name_label =
		trg_general_panel_add_label_with_width(self, _("Name"), 0, 0, -1);

	priv->gen_size_label =
		trg_general_panel_add_label(self, _("Size"), 0, 1);
	priv->gen_down_rate_label =
		trg_general_panel_add_label(self, _("Rate Down"), 1, 1);
	priv->gen_completed_label =
		trg_general_panel_add_label(self, _("Completed"), 2, 1);

	priv->gen_eta_label =
		trg_general_panel_add_label(self, _("ETA"), 0, 2);
	priv->gen_up_rate_label =
		trg_general_panel_add_label(self, _("Rate Up"), 1, 2);
	priv->gen_downloaded_label =
		trg_general_panel_add_label(self, _("Downloaded"), 2, 2);

	priv->gen_seeders_label =
		trg_general_panel_add_label(self, _("Seeders"), 0, 3);
	priv->gen_ratio_label =
		trg_general_panel_add_label(self, _("Ratio"), 1, 3);
	priv->gen_uploaded_label =
		trg_general_panel_add_label(self, _("Uploaded"), 2, 3);

	priv->gen_leechers_label =
		trg_general_panel_add_label(self, _("Leechers"), 0, 4);
	priv->gen_limit_label =
		trg_general_panel_add_label(self, _("Ratio limit"), 1, 4);
	priv->gen_corrupted_label =
		trg_general_panel_add_label(self, _("Corrupted"), 2, 4);

	priv->gen_status_label =
		trg_general_panel_add_label(self, _("Status"), 0, 5);
	priv->gen_priority_label =
		trg_general_panel_add_label(self, _("Priority"), 1, 5);
	priv->gen_completedat_label =
		trg_general_panel_add_label(self, _("Completed At"), 2, 5);

	priv->gen_downloaddir_label =
		trg_general_panel_add_label_with_width(self, _("Location"), 0, 6, -1);

	priv->gen_comment_label =
		trg_general_panel_add_label(self, _("Comment"), 0, 7);

	priv->gen_hash_label =
		trg_general_panel_add_label(self, _("Hash"), 0, 8);

	priv->gen_error_label =
		trg_general_panel_add_label_with_width(self, "", 0, 9, -1);

    gtk_grid_set_row_homogeneous(GTK_GRID(self), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(self), TRG_GENERAL_PANEL_SPACING_X);
    gtk_grid_set_row_spacing(GTK_GRID(self), TRG_GENERAL_PANEL_SPACING_Y);
    gtk_widget_set_sensitive(GTK_WIDGET(self), FALSE);
}

TrgGeneralPanel *trg_general_panel_new(GtkTreeModel * model,
                                       TrgClient * tc)
{
    GObject *obj;
    TrgGeneralPanelPrivate *priv;

    obj = g_object_new(TRG_TYPE_GENERAL_PANEL, NULL);

    priv = TRG_GENERAL_PANEL_GET_PRIVATE(obj);
    priv->model = model;
    priv->tc = tc;

    return TRG_GENERAL_PANEL(obj);
}
