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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>

#ifdef HAVE_LIBPROXY
#include <proxy.h>
#endif

#include <curl/curl.h>
#include <curl/easy.h>

#include "json.h"
#include "trg-prefs.h"
#include "protocol-constants.h"
#include "util.h"
#include "trg-client.h"

G_DEFINE_TYPE (TrgClient, trg_client, G_TYPE_OBJECT)

enum {
    TC_SESSION_UPDATED,
    TC_SIGNAL_COUNT
};

static guint signals[TC_SIGNAL_COUNT] = { 0 };

#define TRG_CLIENT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_CLIENT, TrgClientPrivate))

typedef struct _TrgClientPrivate TrgClientPrivate;

struct _TrgClientPrivate {
    char *session_id;
    gint connid;
    gboolean activeOnlyUpdate;
    guint failCount;
    guint interval;
    guint min_interval;
    gint64 updateSerial;
    JsonObject *session;
    gboolean ssl;
    float version;
    char *url;
    char *username;
    char *password;
    char *proxy;
    GHashTable *torrentTable;
    GThreadPool *pool;
    GMutex *updateMutex;
    TrgPrefs *prefs;
    GPrivate *tlsKey;
    gint configSerial;
    GMutex *configMutex;
};

static void dispatch_async_threadfunc(trg_request *reqrsp,
                                      TrgClient * client);

static void
trg_client_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
trg_client_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
trg_client_dispose (GObject *object)
{
  G_OBJECT_CLASS (trg_client_parent_class)->dispose (object);
}

static void
trg_client_class_init (TrgClientClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TrgClientPrivate));

  object_class->get_property = trg_client_get_property;
  object_class->set_property = trg_client_set_property;
  object_class->dispose = trg_client_dispose;

  signals[TC_SESSION_UPDATED] =
      g_signal_new("session-updated",
                   G_TYPE_FROM_CLASS(object_class),
                   G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                   G_STRUCT_OFFSET(TrgClientClass,
                                   session_updated), NULL,
                   NULL, g_cclosure_marshal_VOID__POINTER,
                   G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
trg_client_init (TrgClient *self)
{
}

TrgClient*
trg_client_new (void)
{
    TrgClient *tc = g_object_new (TRG_TYPE_CLIENT, NULL);
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    TrgPrefs *prefs = priv->prefs = trg_prefs_new();

    trg_prefs_load(prefs);

    priv->updateMutex = g_mutex_new();
    priv->configMutex = g_mutex_new();
    priv->tlsKey = g_private_new(NULL);

    priv->pool = g_thread_pool_new((GFunc) dispatch_async_threadfunc, tc,
                                 DISPATCH_POOL_SIZE, TRUE, NULL);

    return tc;
}

const gchar *trg_client_get_version_string(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return session_get_version_string(priv->session);
}

float trg_client_get_version(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->version;
}

gint64 trg_client_get_rpc_version(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return session_get_rpc_version(priv->session);
}

void trg_client_set_session(TrgClient * tc, JsonObject * session)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);

    if (priv->session) {
        json_object_unref(priv->session);
    } else {
        session_get_version(session, &priv->version);
        g_atomic_int_inc(&priv->connid);
    }

    priv->session = session;

    g_signal_emit(tc, signals[TC_SESSION_UPDATED], 0, session);
}

TrgPrefs *trg_client_get_prefs(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->prefs;
}

int trg_client_populate_with_settings(TrgClient * tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    TrgPrefs *prefs = priv->prefs;

    gint port;
    gchar *host;
#ifdef HAVE_LIBPROXY
    pxProxyFactory *pf = NULL;
#endif

    g_mutex_lock(priv->configMutex);

    g_free(priv->url);
    priv->url = NULL;

    g_free(priv->username);
    priv->username = NULL;

    g_free(priv->password);
    priv->password = NULL;

    port =
        trg_prefs_get_int(prefs, TRG_PREFS_KEY_PORT, TRG_PREFS_PROFILE);

    host = trg_prefs_get_string(prefs, TRG_PREFS_KEY_HOSTNAME, TRG_PREFS_PROFILE);
    if (!host) {
        g_mutex_unlock(priv->configMutex);
        return TRG_NO_HOSTNAME_SET;
    } else if (strlen(host) < 1) {
        g_free(host);
        g_mutex_unlock(priv->configMutex);
        return TRG_NO_HOSTNAME_SET;
    }

#ifndef CURL_NO_SSL
    priv->ssl = trg_prefs_get_bool(prefs, TRG_PREFS_KEY_SSL, TRG_PREFS_PROFILE);
#else
    priv->ssl = FALSE;
#endif

    priv->url =
        g_strdup_printf("%s://%s:%d/transmission/rpc",
                        priv->ssl ? HTTPS_URI_PREFIX : HTTP_URI_PREFIX, host, port);
    g_free(host);

    priv->interval =
        trg_prefs_get_int(prefs, TRG_PREFS_KEY_UPDATE_INTERVAL, TRG_PREFS_PROFILE);
    if (priv->interval < 1)
        priv->interval = TRG_INTERVAL_DEFAULT;

    priv->min_interval =
        trg_prefs_get_int(prefs, TRG_PREFS_KEY_MINUPDATE_INTERVAL, TRG_PREFS_PROFILE);
    if (priv->min_interval < 1)
        priv->min_interval = TRG_INTERVAL_DEFAULT;

    priv->username =
        trg_prefs_get_string(prefs, TRG_PREFS_KEY_USERNAME, TRG_PREFS_PROFILE);

    priv->password =
        trg_prefs_get_string(prefs, TRG_PREFS_KEY_PASSWORD, TRG_PREFS_PROFILE);

    g_free(priv->proxy);
    priv->proxy = NULL;

    priv->activeOnlyUpdate =
        trg_prefs_get_bool(prefs,
                              TRG_PREFS_KEY_UPDATE_ACTIVE_ONLY, TRG_PREFS_PROFILE);

#ifdef HAVE_LIBPROXY
    if ((pf = px_proxy_factory_new())) {
        char **proxies = px_proxy_factory_get_proxies(pf, priv->url);
        int i;

        for (i = 0; proxies[i]; i++) {
            if (g_str_has_prefix(proxies[i], HTTP_URI_PREFIX)) {
                g_free(priv->proxy);
                priv->proxy = proxies[i];
            } else {
                g_free(proxies[i]);
            }
        }

        g_free(proxies);
        px_proxy_factory_free(pf);
    }
#endif

    priv->configSerial++;
    g_mutex_unlock(priv->configMutex);
    return 0;
}

gchar *trg_client_get_password(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->password;
}

gchar *trg_client_get_username(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->username;
}

gchar *trg_client_get_url(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->url;
}

gchar *trg_client_get_session_id(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->session_id ? g_strdup(priv->session_id) : NULL;
}

void trg_client_set_session_id(TrgClient *tc, gchar *session_id)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);

    g_mutex_lock(priv->configMutex);

    if (priv->session_id)
        g_free(priv->session_id);

    priv->session_id = session_id;

    g_mutex_unlock(priv->configMutex);
}

void trg_client_status_change(TrgClient *tc, gboolean connected)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);

    if (!connected && priv->session) {
        json_object_unref(priv->session);
        priv->session = NULL;
    }
}

JsonObject* trg_client_get_session(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->session;
}

void trg_client_thread_pool_push(TrgClient *tc, gpointer data, GError **err)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    g_thread_pool_push(priv->pool, data, err);
}

void trg_client_inc_serial(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->updateSerial++;
}

gint64 trg_client_get_serial(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->updateSerial;
}

#ifndef CURL_NO_SSL
gboolean trg_client_get_ssl(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->ssl;
}
#endif

gchar *trg_client_get_proxy(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->proxy;
}

void trg_client_set_torrent_table(TrgClient *tc, GHashTable *table)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->torrentTable = table;
}

GHashTable* trg_client_get_torrent_table(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->torrentTable;
}

gboolean trg_client_is_connected(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->session != NULL;
}

void trg_client_updatelock(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    g_mutex_lock(priv->updateMutex);
}

guint trg_client_get_failcount(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->failCount;
}

guint trg_client_inc_failcount(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return ++(priv->failCount);
}

void trg_client_reset_failcount(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->failCount = 0;
}

void trg_client_updateunlock(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    g_mutex_unlock(priv->updateMutex);
}

gboolean trg_client_get_activeonlyupdate(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->activeOnlyUpdate;
}

void trg_client_set_activeonlyupdate(TrgClient *tc, gboolean activeOnlyUpdate)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->activeOnlyUpdate = activeOnlyUpdate;
}

guint trg_client_get_minimised_interval(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->min_interval;
}

guint trg_client_get_interval(TrgClient *tc)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    return priv->interval;
}

void trg_client_set_interval(TrgClient *tc, guint interval)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->interval = interval;
}

void trg_client_set_minimised_interval(TrgClient *tc, guint interval)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    priv->min_interval = interval;
}

/* formerly http.c */

void trg_response_free(trg_response *response)
{
	if (response->obj)
		json_object_unref(response->obj);
    g_free(response);
}

static size_t
http_receive_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    trg_response *mem = (trg_response *) data;

    mem->raw = g_realloc(mem->raw, mem->size + realsize + 1);
    if (mem->raw) {
        memcpy(&(mem->raw[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->raw[mem->size] = 0;
    }

    return realsize;
}

static size_t header_callback(void *ptr, size_t size, size_t nmemb,
                              void *data)
{
    char *header = (char *) (ptr);
    TrgClient *tc = TRG_CLIENT(data);
    gchar *session_id;

    if (g_str_has_prefix(header, X_TRANSMISSION_SESSION_ID_HEADER_PREFIX)) {
        char *nl;

        session_id = g_strdup(header);
        nl = strrchr(session_id, '\r');
        if (nl)
            *nl = '\0';

        trg_client_set_session_id(tc, session_id);
    }

    return (nmemb * size);
}

static void trg_tls_update(TrgClient *tc, trg_tls *tls, gint serial)
{
    gchar *proxy;

    curl_easy_setopt(tls->curl, CURLOPT_PASSWORD, trg_client_get_password(tc));
    curl_easy_setopt(tls->curl, CURLOPT_USERNAME, trg_client_get_username(tc));
    curl_easy_setopt(tls->curl, CURLOPT_URL, trg_client_get_url(tc));

#ifndef CURL_NO_SSL
    if (trg_client_get_ssl(tc))
        curl_easy_setopt(tls->curl, CURLOPT_SSL_VERIFYPEER, 0);
#endif

    proxy = trg_client_get_proxy(tc);
    if (proxy) {
        curl_easy_setopt(tls->curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
        curl_easy_setopt(tls->curl, CURLOPT_PROXY, proxy);
    }

    tls->serial = serial;
}

trg_tls *trg_tls_new(TrgClient *tc)
{
    trg_tls *tls = g_new0(trg_tls, 1);

    tls->curl = curl_easy_init();
    curl_easy_setopt(tls->curl, CURLOPT_USERAGENT, PACKAGE_NAME);
    curl_easy_setopt(tls->curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(tls->curl, CURLOPT_WRITEFUNCTION,
                     &http_receive_callback);
    curl_easy_setopt(tls->curl, CURLOPT_HEADERFUNCTION, &header_callback);
    curl_easy_setopt(tls->curl, CURLOPT_WRITEHEADER, (void *) tc);

    tls->serial = -1;

    return tls;
}

static int trg_http_perform_inner(TrgClient * tc,
                                                    gchar * reqstr,
                                                    trg_response *response,
                                                    gboolean recurse)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(tc);
    gpointer threadLocalStorage = g_private_get(priv->tlsKey);
    trg_tls *tls;
    long httpCode = 0;
    gchar *session_id;
    struct curl_slist *headers = NULL;

    if (!threadLocalStorage) {
        tls = trg_tls_new(tc);
        g_private_set(priv->tlsKey, tls);
    } else {
        tls = (trg_tls*)threadLocalStorage;
    }

    g_mutex_lock(priv->configMutex);

    if (priv->configSerial > tls->serial)
        trg_tls_update(tc, tls, priv->configSerial);

    session_id = trg_client_get_session_id(tc);
    if (session_id) {
        headers = curl_slist_append(NULL, session_id);
        curl_easy_setopt(tls->curl, CURLOPT_HTTPHEADER, headers);
    }

    g_mutex_unlock(priv->configMutex);

    response->size = 0;
    response->raw = NULL;

    curl_easy_setopt(tls->curl, CURLOPT_POSTFIELDS, reqstr);
    curl_easy_setopt(tls->curl, CURLOPT_WRITEDATA, (void *)response);
    response->status = curl_easy_perform(tls->curl);

    if (session_id) {
        g_free(session_id);
        curl_slist_free_all(headers);
    }

    curl_easy_getinfo(tls->curl, CURLINFO_RESPONSE_CODE, &httpCode);

    if (response->status == CURLE_OK) {
        if (httpCode == HTTP_CONFLICT && recurse == TRUE)
            return trg_http_perform_inner(tc, reqstr, response, FALSE);
        else if (httpCode != HTTP_OK)
            response->status = (-httpCode) - 100;
    }

    return response->status;
}

int trg_http_perform(TrgClient * tc, gchar * reqstr, trg_response *reqrsp)
{
    return trg_http_perform_inner(tc, reqstr, reqrsp, TRUE);
}

/* formerly dispatch.c */

trg_response *dispatch(TrgClient * client, JsonNode * req)
{
    gchar *serialized = trg_serialize(req);
    json_node_free(req);
#ifdef DEBUG
    if (g_getenv("TRG_SHOW_OUTGOING"))
        g_debug("=>(OUTgoing)=>: %s", serialized);
#endif
    return dispatch_str(client, serialized);
}

trg_response *dispatch_str(TrgClient * client, gchar *req)
{
    trg_response *response = g_new0(trg_response, 1);
    GError *decode_error = NULL;
    JsonNode *result;

    trg_http_perform(client, req, response);
    g_free(req);

    if (response->status != CURLE_OK)
    	return response;

    response->obj = trg_deserialize(response, &decode_error);
    g_free(response->raw);
    response->raw = NULL;

    if (decode_error) {
        g_error("JSON decoding error: %s", decode_error->message);
        g_error_free(decode_error);
        response->status = FAIL_JSON_DECODE;
        return response;
    }

    result = json_object_get_member(response->obj, FIELD_RESULT);
    if (!result || g_strcmp0(json_node_get_string(result), FIELD_SUCCESS))
        response->status = FAIL_RESPONSE_UNSUCCESSFUL;

    return response;
}

static void dispatch_async_threadfunc(trg_request *req,
                                      TrgClient * client)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(client);

    trg_response *rsp;

    if (req->str)
        rsp = dispatch_str(client, req->str);
    else
        rsp = dispatch(client, req->node);

    rsp->cb_data = req->cb_data;

    if (req->callback && req->connid == g_atomic_int_get(&priv->connid))
        g_idle_add(req->callback, rsp);
    else
        trg_response_free(rsp);

    g_free(req);
}

static gboolean dispatch_async_common(TrgClient * client, trg_request *trg_req,
                        GSourceFunc callback,
                        gpointer data)
{
    TrgClientPrivate *priv = TRG_CLIENT_GET_PRIVATE(client);
    GError *error = NULL;

    trg_req->callback = callback;
    trg_req->cb_data = data;
    trg_req->connid = priv->connid;

    trg_client_thread_pool_push(client, trg_req, &error);
    if (error) {
        g_error("thread creation error: %s\n", error->message);
        g_error_free(error);
        g_free(trg_req);
        return FALSE;
    } else {
        return TRUE;
    }
}

gboolean dispatch_async(TrgClient * client, JsonNode *req,
        GSourceFunc callback,
        gpointer data)
{
    trg_request *trg_req = g_new0(trg_request, 1);
    trg_req->node = req;

    return dispatch_async_common(client, trg_req, callback, data);
}

gboolean dispatch_async_str(TrgClient * client, gchar *req,
        GSourceFunc callback,
        gpointer data)
{
    trg_request *trg_req = g_new0(trg_request, 1);
    trg_req->str = req;

    return dispatch_async_common(client, trg_req, callback, data);
}
