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

#if HAVE_RSS

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <rss-glib/rss-glib.h>

#include "torrent.h"
#include "trg-client.h"
#include "trg-model.h"
#include "trg-rss-model.h"

enum {
	PROP_0, PROP_CLIENT
};

enum {
	SIGNAL_GET_ERROR, SIGNAL_PARSE_ERROR, SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT] = { 0 };

G_DEFINE_TYPE(TrgRssModel, trg_rss_model, GTK_TYPE_LIST_STORE)
#define TRG_RSS_MODEL_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_RSS_MODEL, TrgRssModelPrivate))
typedef struct _TrgRssModelPrivate TrgRssModelPrivate;

struct _TrgRssModelPrivate {
	TrgClient *client;
	GHashTable *table;
	guint index;
};

typedef struct {
	TrgRssModel *model;
	gchar *feed_id;
	gchar *feed_url;
	gchar *feed_cookie;
	GError *error;
	trg_response *response;
} feed_update;

static void feed_update_free(feed_update *update) {
	if (update->error)
		g_error_free(update->error);

	g_free(update->feed_id);
	g_free(update->feed_url);
	g_free(update->feed_cookie);

	g_free(update);
}

static gboolean on_rss_receive(gpointer data) {
	trg_response *response = (trg_response *) data;
	feed_update *update = (feed_update*) response->cb_data;
	TrgRssModel *model = update->model;
	TrgRssModelPrivate *priv = TRG_RSS_MODEL_GET_PRIVATE(model);

	if (response->status == CURLE_OK) {
		RssParser* parser = rss_parser_new();
		GError *error = NULL;
		if (rss_parser_load_from_data(parser, response->raw,
				response->size, &error)) {
			RssDocument *doc = rss_parser_get_document(parser);
			GtkTreeIter iter;
			GList *list, *tmp;

			list = rss_document_get_items(doc);

			for (tmp = list; tmp != NULL; tmp = tmp->next) {
				RssItem *item = (RssItem*) tmp->data;
				const gchar *guid = rss_item_get_guid(item);
				if (g_hash_table_lookup(priv->table, guid) != (void*) 1) {
					gtk_list_store_append(GTK_LIST_STORE(model), &iter);
					gtk_list_store_set(GTK_LIST_STORE(model), &iter, RSSCOL_ID,
							guid, RSSCOL_TITLE, rss_item_get_title(item),
							RSSCOL_LINK, rss_item_get_link(item), RSSCOL_FEED,
							update->feed_id, RSSCOL_COOKIE, update->feed_cookie, RSSCOL_PUBDATE,
							rss_item_get_pub_date(item), -1);
					g_hash_table_insert(priv->table, g_strdup(guid), (void*) 1);
				}
			}

			g_list_free(list);
			g_object_unref(doc);
			g_object_unref(parser);
		} else {
			rss_parse_error perror;
			perror.error = error;
			perror.feed_id = update->feed_id;

		    g_signal_emit(model, signals[SIGNAL_PARSE_ERROR], 0,
		                  &perror);

		    g_message("parse error: %s", error->message);
			g_error_free(error);
		}
	} else {
		rss_get_error get_error;
		get_error.error_code = response->status;
		get_error.feed_id = update->feed_id;

	    g_signal_emit(model, signals[SIGNAL_GET_ERROR], 0,
	                  &get_error);
	}

	trg_response_free(response);
	feed_update_free(update);

	return FALSE;
}

void trg_rss_model_update(TrgRssModel * model) {
	TrgRssModelPrivate *priv = TRG_RSS_MODEL_GET_PRIVATE(model);
	TrgPrefs *prefs = trg_client_get_prefs(priv->client);
	JsonArray *feeds = trg_prefs_get_rss(prefs);
	GRegex *cookie_regex;
	GList *li;

	if (!feeds)
	    return;

	cookie_regex = g_regex_new("(.*):COOKIE:(.*)", 0, 0, NULL);

	for (li = json_array_get_elements(feeds); li != NULL;
			li = g_list_next(li)) {
		JsonObject *feed = json_node_get_object((JsonNode *) li->data);
		const gchar *feed_url = json_object_get_string_member(feed, "url");
		const gchar *id = json_object_get_string_member(feed, "id");
		feed_update *update;
		GMatchInfo *match;

		if (!feed_url || !id)
			continue;

		update = g_new0(feed_update, 1);
		update->feed_id = g_strdup(id);
		update->model = model;

		if (g_regex_match (cookie_regex, feed_url, 0, &match)) {
			update->feed_url = g_match_info_fetch(match, 1);
			update->feed_cookie = g_match_info_fetch(match, 2);
			g_match_info_free(match);
		} else {
			update->feed_url = g_strdup(feed_url);
		}

		async_http_request(priv->client, update->feed_url, update->feed_cookie, on_rss_receive,
				update);
	}

	g_regex_unref(cookie_regex);

	/*trg_model_remove_removed(GTK_LIST_STORE(model),
	 RSSCOL_UPDATESERIAL, updateSerial);*/
}

static void trg_rss_model_set_property(GObject * object, guint prop_id,
		const GValue * value, GParamSpec * pspec G_GNUC_UNUSED) {
	TrgRssModelPrivate *priv = TRG_RSS_MODEL_GET_PRIVATE(object);

	switch (prop_id) {
	case PROP_CLIENT:
		priv->client = g_value_get_pointer(value);
		break;
	}
}

static void trg_rss_model_get_property(GObject * object, guint prop_id,
		GValue * value, GParamSpec * pspec G_GNUC_UNUSED) {
}

static GObject *trg_rss_model_constructor(GType type,
		guint n_construct_properties, GObjectConstructParam * construct_params) {
	GObject *obj = G_OBJECT_CLASS
	(trg_rss_model_parent_class)->constructor(type,
			n_construct_properties, construct_params);
	TrgRssModelPrivate *priv = TRG_RSS_MODEL_GET_PRIVATE(obj);

	priv->table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	return obj;
}

static void trg_rss_model_dispose(GObject * object) {
	TrgRssModelPrivate *priv = TRG_RSS_MODEL_GET_PRIVATE(object);
	g_hash_table_destroy(priv->table);
	G_OBJECT_CLASS(trg_rss_model_parent_class)->dispose(object);
}

static void trg_rss_model_class_init(TrgRssModelClass * klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(TrgRssModelPrivate));

	object_class->set_property = trg_rss_model_set_property;
	object_class->get_property = trg_rss_model_get_property;
	object_class->constructor = trg_rss_model_constructor;
	object_class->dispose = trg_rss_model_dispose;

	g_object_class_install_property(object_class, PROP_CLIENT,
			g_param_spec_pointer("client", "client", "client",
					G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
							| G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK
							| G_PARAM_STATIC_BLURB));

    signals[SIGNAL_GET_ERROR] = g_signal_new("get-error",
                                                     G_TYPE_FROM_CLASS
                                                     (object_class),
                                                     G_SIGNAL_RUN_LAST |
                                                     G_SIGNAL_ACTION,
                                                     G_STRUCT_OFFSET
                                                     (TrgRssModelClass,
                                                      get_error),
                                                     NULL, NULL,
                                                     g_cclosure_marshal_VOID__POINTER,
                                                     G_TYPE_NONE, 1,
                                                     G_TYPE_POINTER);

    signals[SIGNAL_PARSE_ERROR] = g_signal_new("parse-error",
                                                     G_TYPE_FROM_CLASS
                                                     (object_class),
                                                     G_SIGNAL_RUN_LAST |
                                                     G_SIGNAL_ACTION,
                                                     G_STRUCT_OFFSET
                                                     (TrgRssModelClass,
                                                      parse_error),
                                                     NULL, NULL,
                                                     g_cclosure_marshal_VOID__POINTER,
                                                     G_TYPE_NONE, 1,
                                                     G_TYPE_POINTER);
}

static void trg_rss_model_init(TrgRssModel * self) {
	GType column_types[RSSCOL_COLUMNS];

	column_types[RSSCOL_ID] = G_TYPE_STRING;
	column_types[RSSCOL_TITLE] = G_TYPE_STRING;
	column_types[RSSCOL_LINK] = G_TYPE_STRING;
	column_types[RSSCOL_COOKIE] = G_TYPE_STRING;
	column_types[RSSCOL_FEED] = G_TYPE_STRING;
	column_types[RSSCOL_PUBDATE] = G_TYPE_STRING;
	column_types[RSSCOL_UPLOADED] = G_TYPE_BOOLEAN;

	gtk_list_store_set_column_types(GTK_LIST_STORE(self), RSSCOL_COLUMNS,
			column_types);
}

TrgRssModel *trg_rss_model_new(TrgClient *client) {
	return g_object_new(TRG_TYPE_RSS_MODEL, "client", client, NULL);
}

#endif
