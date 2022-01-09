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

#ifndef TRG_RSS_MODEL_H_
#define TRG_RSS_MODEL_H_

#if HAVE_RSS

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"
#include "trg-model.h"

G_BEGIN_DECLS
#define TRG_TYPE_RSS_MODEL trg_rss_model_get_type()
#define TRG_RSS_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_RSS_MODEL, TrgRssModel))
#define TRG_RSS_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_RSS_MODEL, TrgRssModelClass))
#define TRG_IS_RSS_MODEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_RSS_MODEL))
#define TRG_IS_RSS_MODEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_RSS_MODEL))
#define TRG_RSS_MODEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_RSS_MODEL, TrgRssModelClass))
    typedef struct {
    GtkListStore parent;
} TrgRssModel;

typedef struct {
	gchar *feed_id;
	gint error_code;
} rss_get_error;

typedef struct {
	GError *error;
	gchar *feed_id;
} rss_parse_error;

typedef struct {
    GtkListStoreClass parent_class;
    void (*get_error) (TrgRssModel * model,
                               rss_get_error *error);
    void (*parse_error) (TrgRssModel * model,
                               rss_parse_error *error);
} TrgRssModelClass;

GType trg_rss_model_get_type(void);

TrgRssModel *trg_rss_model_new(TrgClient *client);

G_END_DECLS
void trg_rss_model_update(TrgRssModel * model);

enum {
    RSSCOL_ID,
    RSSCOL_TITLE,
    RSSCOL_LINK,
    RSSCOL_FEED,
    RSSCOL_COOKIE,
    RSSCOL_PUBDATE,
    RSSCOL_UPLOADED,
    RSSCOL_COLUMNS
};

#endif

#endif                          /* TRG_RSS_MODEL_H_ */
