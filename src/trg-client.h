/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011  Alan Fitton

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

/* trg-client.h */

#ifndef _TRG_CLIENT_H_
#define _TRG_CLIENT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <curl/curl.h>
#include <curl/easy.h>

#include <json-glib/json-glib.h>
#include <glib-object.h>

#include "trg-prefs.h"
#include "session-get.h"

#define TRANSMISSION_MIN_SUPPORTED 2.0
#define X_TRANSMISSION_SESSION_ID_HEADER_PREFIX "X-Transmission-Session-Id: "

#define TORRENT_GET_MODE_FIRST 0
#define TORRENT_GET_MODE_ACTIVE 1
#define TORRENT_GET_MODE_INTERACTION 2
#define TORRENT_GET_MODE_UPDATE 3

#define TORRENT_GET_TAG_MODE_FULL -1
#define TORRENT_GET_TAG_MODE_UPDATE -2

#define TRG_NO_HOSTNAME_SET -2

#define HTTP_URI_PREFIX "http"
#define HTTPS_URI_PREFIX "https"
#define HTTP_OK 200
#define HTTP_CONFLICT 409

#define FAIL_JSON_DECODE -2
#define FAIL_RESPONSE_UNSUCCESSFUL -3
#define DISPATCH_POOL_SIZE 3

typedef struct {
    int status;
    int size;
    char *raw;
    JsonObject *obj;
    gpointer cb_data;
} trg_response;

typedef struct {
    gint connid;
    JsonNode *node;
    gchar *str;
    GSourceFunc callback;
    gpointer cb_data;
} trg_request;

typedef struct _TrgClientPrivate TrgClientPrivate;

G_BEGIN_DECLS
#define TRG_TYPE_CLIENT trg_client_get_type()
#define TRG_CLIENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_CLIENT, TrgClient))
#define TRG_CLIENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_CLIENT, TrgClientClass))
#define TRG_IS_CLIENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_CLIENT))
#define TRG_IS_CLIENT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_CLIENT))
#define TRG_CLIENT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_CLIENT, TrgClientClass))
    typedef struct {
    GObject parent;
    TrgClientPrivate *priv;
} TrgClient;

typedef struct {
    GObjectClass parent_class;
    void (*session_updated) (TrgClient * tc, JsonObject * session,
                             gpointer data);

} TrgClientClass;

/* Thread local storage (TLS).
 * CURL clients can't be used concurrently.
 * So create one instance for each thread in the thread pool.
 */
typedef struct {
    /* Use a serial to figure out when there's been a configuration change
     * by comparing with priv->serial.
     * We lock updating (and checking for updates) with priv->configMutex
     */
    int serial;
    CURL *curl;
} trg_tls;

/* stuff that used to be in http.h */
void trg_response_free(trg_response * response);
int trg_http_perform(TrgClient * client, gchar * reqstr,
                     trg_response * reqrsp);
/* end http.h*/

/* stuff that used to be in dispatch.c */
trg_response *dispatch(TrgClient * client, JsonNode * req);
trg_response *dispatch_str(TrgClient * client, gchar * req);
gboolean dispatch_async(TrgClient * client, JsonNode * req,
                        GSourceFunc callback, gpointer data);
/* end dispatch.c*/

GType trg_client_get_type(void);

TrgClient *trg_client_new(void);
TrgPrefs *trg_client_get_prefs(TrgClient * tc);
int trg_client_populate_with_settings(TrgClient * tc);
void trg_client_set_session(TrgClient * tc, JsonObject * session);
gdouble trg_client_get_version(TrgClient * tc);
const gchar *trg_client_get_version_string(TrgClient * tc);
gint64 trg_client_get_rpc_version(TrgClient * tc);
gchar *trg_client_get_password(TrgClient * tc);
gchar *trg_client_get_username(TrgClient * tc);
gchar *trg_client_get_url(TrgClient * tc);
gchar *trg_client_get_session_id(TrgClient * tc);
void trg_client_set_session_id(TrgClient * tc, gchar * session_id);
#ifndef CURL_NO_SSL
gboolean trg_client_get_ssl(TrgClient * tc);
#endif
gchar *trg_client_get_proxy(TrgClient * tc);
gint64 trg_client_get_serial(TrgClient * tc);
void trg_client_thread_pool_push(TrgClient * tc, gpointer data,
                                 GError ** err);
void trg_client_set_torrent_table(TrgClient * tc, GHashTable * table);
GHashTable *trg_client_get_torrent_table(TrgClient * tc);
JsonObject *trg_client_get_session(TrgClient * tc);
void trg_client_status_change(TrgClient * tc, gboolean connected);
gboolean trg_client_is_connected(TrgClient * tc);
void trg_client_configunlock(TrgClient * tc);
void trg_client_configlock(TrgClient * tc);
guint trg_client_inc_failcount(TrgClient * tc);
guint trg_client_get_failcount(TrgClient * tc);
void trg_client_reset_failcount(TrgClient * tc);
void trg_client_inc_serial(TrgClient * tc);
void trg_client_inc_connid(TrgClient * tc);
gboolean trg_client_update_session(TrgClient * tc, GSourceFunc callback,
                                   gpointer data);
gboolean trg_client_get_seed_ratio_limited(TrgClient * tc);
gdouble trg_client_get_seed_ratio_limit(TrgClient * tc);

G_END_DECLS
#endif                          /* _TRG_CLIENT_H_ */
