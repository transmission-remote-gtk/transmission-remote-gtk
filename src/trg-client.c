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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>

#ifdef HAVE_LIBPROXY
#include <proxy.h>
#endif

#include <curl/curl.h>
#include <curl/easy.h>

#include "json.h"
#include "trg-prefs.h"
#include "protocol-constants.h"
#include "util.h"
#include "requests.h"
#include "trg-client.h"

/* This class manages/does quite a few things, and is passed around a lot. It:
 *
 * 1) Holds/inits the single TrgPrefs object for managing configuration.
 * 2) Manages a thread pool for making requests
 *    (each thread has its own CURL client in thread local storage)
 * 3) Holds current connection details needed by CURL clients.
 *    (session ID, username, password, URL, ssl, proxy)
 * 4) Holds a hash table for looking up a torrent by its ID.
 * 5) Dispatches synchronous/asyncrhonous requests and tracks failures.
 * 6) Holds connection state, an update serial, and provides signals for
 *    connect/disconnect.
 * 7) Provides a mutex for locking updates.
 * 8) Holds the latest session object sent in a session-get response.
 */

G_DEFINE_TYPE(TrgClient, trg_client, G_TYPE_OBJECT)
enum {
    TC_SESSION_UPDATED, TC_SIGNAL_COUNT
};

static guint signals[TC_SIGNAL_COUNT] = { 0 };

struct _TrgClientPrivate {
    char *session_id;
    gint connid;
    guint failCount;
    guint retries;
    guint timeout;
    gint64 updateSerial;
    JsonObject *session;
    gboolean ssl;
    gboolean ssl_validate;
    gdouble version;
    char *url;
    char *username;
    char *password;
    char *proxy;
    GHashTable *torrentTable;
    GThreadPool *pool;
    TrgPrefs *prefs;
    gint configSerial;
    guint http_class;
    GMutex configMutex;
    gboolean seedRatioLimited;
    gdouble seedRatioLimit;
};

static void trg_tls_free(gpointer data);

static GPrivate tls_key = G_PRIVATE_INIT(trg_tls_free);

static void dispatch_async_threadfunc(trg_request * reqrsp,
                                      TrgClient * tc);

static void
trg_client_get_property(GObject * object, guint property_id,
                        GValue * value, GParamSpec * pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
trg_client_set_property(GObject * object, guint property_id,
                        const GValue * value, GParamSpec * pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_client_dispose(GObject * object)
{
    G_OBJECT_CLASS(trg_client_parent_class)->dispose(object);
}

static void trg_client_finalize(GObject * object)
{
    TrgClient *tc = TRG_CLIENT(object);
    TrgClientPrivate *priv = tc->priv;

    g_free(priv->session_id);
    json_object_unref(priv->session);
    g_free(priv->url);
    g_free(priv->username);
    g_free(priv->password);
    g_free(priv->proxy);
    g_hash_table_unref(priv->torrentTable);
    g_thread_pool_free(priv->pool, TRUE, TRUE);

    G_OBJECT_CLASS(trg_client_parent_class)->finalize(object);
}

static void trg_client_class_init(TrgClientClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgClientPrivate));

    object_class->get_property = trg_client_get_property;
    object_class->set_property = trg_client_set_property;
    object_class->dispose = trg_client_dispose;
    object_class->finalize = trg_client_finalize;

    signals[TC_SESSION_UPDATED] = g_signal_new("session-updated",
                                               G_TYPE_FROM_CLASS
                                               (object_class),
                                               G_SIGNAL_RUN_LAST |
                                               G_SIGNAL_ACTION,
                                               G_STRUCT_OFFSET
                                               (TrgClientClass,
                                                session_updated), NULL,
                                               NULL,
                                               g_cclosure_marshal_VOID__POINTER,
                                               G_TYPE_NONE, 1,
                                               G_TYPE_POINTER);
}

static void trg_client_init(TrgClient * self)
{
    self->priv =
        G_TYPE_INSTANCE_GET_PRIVATE(self, TRG_TYPE_CLIENT,
                                    TrgClientPrivate);
}

TrgClient *trg_client_new(void)
{
    TrgClient *tc = g_object_new(TRG_TYPE_CLIENT, NULL);
    TrgClientPrivate *priv = tc->priv;
    TrgPrefs *prefs = priv->prefs = trg_prefs_new();

    trg_prefs_load(prefs);

    g_mutex_init(&priv->configMutex);
    priv->seedRatioLimited = FALSE;
    priv->seedRatioLimit = 0.00;

    priv->pool = g_thread_pool_new((GFunc) dispatch_async_threadfunc, tc,
                                   DISPATCH_POOL_SIZE, TRUE, NULL);

    tr_formatter_size_init(disk_K, _(disk_K_str), _(disk_M_str),
                           _(disk_G_str), _(disk_T_str));
    tr_formatter_speed_init(speed_K, _(speed_K_str), _(speed_M_str),
                            _(speed_G_str), _(speed_T_str));

    return tc;
}

const gchar *trg_client_get_version_string(TrgClient * tc)
{
    return session_get_version_string(tc->priv->session);
}

gdouble trg_client_get_version(TrgClient * tc)
{
    return tc->priv->version;
}

gint64 trg_client_get_rpc_version(TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    return session_get_rpc_version(priv->session);
}

void trg_client_inc_connid(TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    g_atomic_int_inc(&priv->connid);
}

void trg_client_set_session(TrgClient * tc, JsonObject * session)
{
    TrgClientPrivate *priv = tc->priv;

    if (priv->session)
        json_object_unref(priv->session);
    else
        priv->version = session_get_version(session);

    priv->session = session;
    json_object_ref(session);

    priv->seedRatioLimit = session_get_seed_ratio_limit(session);
    priv->seedRatioLimited = session_get_seed_ratio_limited(session);

    g_signal_emit(tc, signals[TC_SESSION_UPDATED], 0, session);
}

TrgPrefs *trg_client_get_prefs(TrgClient * tc)
{
    return tc->priv->prefs;
}

int trg_client_populate_with_settings(TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    TrgPrefs *prefs = priv->prefs;

    gint port;
    gchar *host, *path;
#ifdef HAVE_LIBPROXY
    pxProxyFactory *pf = NULL;
#endif

    g_mutex_lock(&priv->configMutex);

    trg_prefs_set_connection(prefs, trg_prefs_get_profile(prefs));

    g_clear_pointer(&priv->url, g_free);
    g_clear_pointer(&priv->username, g_free);
    g_clear_pointer(&priv->password, g_free);

    port =
        trg_prefs_get_int(prefs, TRG_PREFS_KEY_PORT, TRG_PREFS_CONNECTION);

    host = trg_prefs_get_string(prefs, TRG_PREFS_KEY_HOSTNAME,
                                TRG_PREFS_CONNECTION);
    path = trg_prefs_get_string(prefs, TRG_PREFS_KEY_RPC_URL_PATH, TRG_PREFS_CONNECTION);

    if (!host || strlen(host) < 1) {
        g_free(host);
        g_free(path);
        g_mutex_unlock(&priv->configMutex);
        return TRG_NO_HOSTNAME_SET;
    }
#ifndef CURL_NO_SSL
    priv->ssl = trg_prefs_get_bool(prefs, TRG_PREFS_KEY_SSL,
                                   TRG_PREFS_CONNECTION);
    priv->ssl_validate = trg_prefs_get_bool(prefs, TRG_PREFS_KEY_SSL_VALIDATE,
                                   TRG_PREFS_CONNECTION);

#else
    priv->ssl = FALSE;
#endif


    priv->url = g_strdup_printf("%s://%s:%d%s",
                                priv->ssl ? HTTPS_URI_PREFIX :
                                HTTP_URI_PREFIX, host, port, path);
    g_free(host);
    g_free(path);

    priv->username = trg_prefs_get_string(prefs, TRG_PREFS_KEY_USERNAME,
                                          TRG_PREFS_CONNECTION);

    priv->password = trg_prefs_get_string(prefs, TRG_PREFS_KEY_PASSWORD,
                                          TRG_PREFS_CONNECTION);

    g_clear_pointer(&priv->proxy, g_free);

#ifdef HAVE_LIBPROXY
    if ((pf = px_proxy_factory_new())) {
        char **proxies = px_proxy_factory_get_proxies(pf, priv->url);
        int i;

        for (i = 0; proxies[i]; i++) {
            if (g_str_has_prefix(proxies[i], HTTP_URI_PREFIX)
                || g_str_has_prefix(proxies[i], HTTPS_URI_PREFIX)) {
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
    g_mutex_unlock(&priv->configMutex);
    return 0;
}

gchar *trg_client_get_password(TrgClient * tc)
{
    return tc->priv->password;
}

gchar *trg_client_get_username(TrgClient * tc)
{
    return tc->priv->username;
}

gchar *trg_client_get_url(TrgClient * tc)
{
    return tc->priv->url;
}

gchar *trg_client_get_session_id(TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    gchar *ret;

    g_mutex_lock(&priv->configMutex);

    ret = priv->session_id ? g_strdup(priv->session_id) : NULL;

    g_mutex_unlock(&priv->configMutex);

    return ret;
}

void trg_client_set_session_id(TrgClient * tc, gchar * session_id)
{
    TrgClientPrivate *priv = tc->priv;

    g_mutex_lock(&priv->configMutex);

    if (priv->session_id)
        g_free(priv->session_id);

    priv->session_id = session_id;

    g_mutex_unlock(&priv->configMutex);
}

void trg_client_status_change(TrgClient * tc, gboolean connected)
{
    TrgClientPrivate *priv = tc->priv;

    if (!connected) {
        if (priv->session) {
            json_object_unref(priv->session);
            priv->session = NULL;
        }
        g_mutex_lock(&priv->configMutex);
        trg_prefs_set_connection(priv->prefs, NULL);
        g_mutex_unlock(&priv->configMutex);
    }
}

JsonObject *trg_client_get_session(TrgClient * tc)
{
    return tc->priv->session;
}

void
trg_client_thread_pool_push(TrgClient * tc, gpointer data, GError ** err)
{
    g_thread_pool_push(tc->priv->pool, data, err);
}

void trg_client_inc_serial(TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    priv->updateSerial++;
}

gint64 trg_client_get_serial(TrgClient * tc)
{
    return tc->priv->updateSerial;
}

#ifndef CURL_NO_SSL
gboolean trg_client_get_ssl(TrgClient * tc)
{
    return tc->priv->ssl;
}

gboolean trg_client_get_ssl_validate(TrgClient * tc)
{
    return tc->priv->ssl_validate;
}
#endif

gchar *trg_client_get_proxy(TrgClient * tc)
{
    return tc->priv->proxy;
}

void trg_client_set_torrent_table(TrgClient * tc, GHashTable * table)
{
    TrgClientPrivate *priv = tc->priv;
    priv->torrentTable = table;
}

GHashTable *trg_client_get_torrent_table(TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    return priv->torrentTable;
}

gboolean trg_client_is_connected(TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    return priv->session != NULL;
}

void trg_client_configlock(TrgClient * tc)
{
    g_mutex_lock(&tc->priv->configMutex);
}

guint trg_client_get_failcount(TrgClient * tc)
{
    return tc->priv->failCount;
}

guint trg_client_inc_failcount(TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    return ++(priv->failCount);
}

void trg_client_reset_failcount(TrgClient * tc)
{
    tc->priv->failCount = 0;
}

void trg_client_configunlock(TrgClient * tc)
{
    g_mutex_unlock(&tc->priv->configMutex);
}

/* formerly http.c */

void trg_response_free(trg_response * response)
{
	if (response) {
		if (response->obj)
			json_object_unref(response->obj);

		if (response->raw)
			g_free(response->raw);

		g_free(response);
	}
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

static size_t
header_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    char *header = (char *) (ptr);
    TrgClient *tc = TRG_CLIENT(data);
    gchar *session_id;

    if (g_ascii_strncasecmp(header, X_TRANSMISSION_SESSION_ID_HEADER_PREFIX,
            strlen(X_TRANSMISSION_SESSION_ID_HEADER_PREFIX)) == 0) {
        char *nl;

        session_id = g_strdup(header);
        nl = strrchr(session_id, '\r');
        if (nl)
            *nl = '\0';

        trg_client_set_session_id(tc, session_id);
    }

    return (nmemb * size);
}

static trg_tls *trg_tls_new(TrgClient * tc)
{
    trg_tls *tls = g_new0(trg_tls, 1);

    tls->curl = curl_easy_init();
    tls->serial = -1;

    return tls;
}

static void trg_tls_free(gpointer data)
{
    trg_tls *tls = (trg_tls *)data;
    if (tls) {
        curl_easy_cleanup(tls->curl);
    }
}

static trg_tls *get_tls(TrgClient *tc) {
    trg_tls *tls = (trg_tls *) g_private_get(&tls_key);
    if (!tls) {
        tls = trg_tls_new(tc);
        g_private_set(&tls_key, tls);
    }

    return tls;
}

static CURL* get_curl(TrgClient *tc, guint http_class)
{
	TrgClientPrivate *priv = tc->priv;
	TrgPrefs *prefs = trg_client_get_prefs(tc);
	trg_tls *tls = get_tls(tc);
	CURL *curl = tls->curl;

    g_mutex_lock(&priv->configMutex);

    if (priv->configSerial > tls->serial || http_class != priv->http_class) {
    	gchar *proxy;

        curl_easy_reset(curl);

        curl_easy_setopt(curl, CURLOPT_USERAGENT, PACKAGE_NAME);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                         &http_receive_callback);
#ifdef DEBUG
        if (g_getenv("TRG_CURL_VERBOSE") != NULL)
        	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif

        if (http_class == HTTP_CLASS_TRANSMISSION) {
        	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *) tc);
        	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &header_callback);
            curl_easy_setopt(curl, CURLOPT_PASSWORD,
                             trg_client_get_password(tc));
            curl_easy_setopt(curl, CURLOPT_USERNAME,
                             trg_client_get_username(tc));
            curl_easy_setopt(curl, CURLOPT_URL, trg_client_get_url(tc));
        }

    #ifndef CURL_NO_SSL
        if (trg_client_get_ssl(tc) && !trg_client_get_ssl_validate(tc)) {

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        }
    #endif

        proxy = trg_client_get_proxy(tc);
        if (proxy) {
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
        }

        tls->serial = priv->configSerial;
        priv->http_class = http_class;
    }

    if (http_class == HTTP_CLASS_TRANSMISSION)
    	curl_easy_setopt(curl, CURLOPT_URL, trg_client_get_url(tc));

	curl_easy_setopt(curl, CURLOPT_TIMEOUT,
					 (long) trg_prefs_get_int(prefs, TRG_PREFS_KEY_TIMEOUT,
											  TRG_PREFS_CONNECTION));
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");

    g_mutex_unlock(&priv->configMutex);

    /* Headers are set on each use, then freed, so make sure invalid headers aren't still around. */
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);

    return curl;

}

static inline int
trg_http_perform_inner(TrgClient * tc, trg_request * request,
                       trg_response * response, gboolean recurse)
{
    CURL* curl = get_curl(tc, HTTP_CLASS_TRANSMISSION);
	struct curl_slist *headers = NULL;
	gchar *session_id = NULL;
    long httpCode = 0;

    response->size = 0;
    response->raw = NULL;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) response);

	session_id = trg_client_get_session_id(tc);
	if (session_id)
		headers = curl_slist_append(NULL, session_id);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    response->status = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    g_free(session_id);

    if (headers)
    	curl_slist_free_all(headers);

    if (response->status == CURLE_OK) {
        if (httpCode == HTTP_CONFLICT && recurse == TRUE) {
            g_free(response->raw);
            return trg_http_perform_inner(tc, request, response, FALSE);
        } else if (httpCode != HTTP_OK) {
            response->status = (-httpCode) - 100;
        }
    }

    return response->status;
}

int trg_http_perform(TrgClient * tc, trg_request *request, trg_response * rsp)
{
    return trg_http_perform_inner(tc, request, rsp, TRUE);
}

static void trg_request_free(trg_request *req)
{
    if (req) {
        g_free(req->body);
        g_free(req->url);
        g_free(req->cookie);

        if (req->node)
            json_node_free(req->node);

        g_free(req);
    }
}

/* formerly dispatch.c */

trg_response *dispatch(TrgClient * tc, trg_request *req)
{
    trg_response *response = g_new0(trg_response, 1);
    GError *decode_error = NULL;
    JsonNode *result;

	if (req->node && !req->body)
		req->body = trg_serialize(req->node);

#ifdef DEBUG
    if (g_getenv("TRG_SHOW_OUTGOING"))
        g_message("=>(OUTgoing)=>: %s", req->body);
#endif

    trg_http_perform(tc, req, response);

    if (response->status == CURLE_OK)
        response->obj = trg_deserialize(response, &decode_error);

    g_free(response->raw);
    response->raw = NULL;

    if (response->status != CURLE_OK)
        return response;

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

trg_response *dispatch_public_http(TrgClient *tc, trg_request *req) {
	trg_response *response = g_new0(trg_response, 1);
    CURL* curl = get_curl(tc, HTTP_CLASS_PUBLIC);
    struct curl_slist *headers = NULL;
    long httpCode = 0;
    gchar *cookie_header = NULL;

    response->size = 0;
    response->raw = NULL;

	curl_easy_setopt(curl, CURLOPT_URL, req->url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) response);

	if (req->cookie) {
		cookie_header = g_strdup_printf("Cookie: %s", req->cookie);
		headers = curl_slist_append(NULL, cookie_header);
	}

	if (headers)
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    response->status = curl_easy_perform(curl);

    trg_request_free(req);

    g_free(cookie_header);

    if (headers)
    	curl_slist_free_all(headers);

    //g_message(response->raw);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    if (response->status == CURLE_OK && httpCode != HTTP_OK) {
      response->status = (-httpCode) - 100;
    }

	return response;
}

static void dispatch_async_threadfunc(trg_request * req, TrgClient * tc)
{
    TrgClientPrivate *priv = tc->priv;
    trg_response *rsp;

    if (req->url)
    	rsp = dispatch_public_http(tc, req);
    else
        rsp = dispatch(tc, req);

    rsp->cb_data = req->cb_data;

    if (req->callback && req->connid == g_atomic_int_get(&priv->connid))
        g_idle_add(req->callback, rsp);
    else
        trg_response_free(rsp);

    trg_request_free(req);
}

static gboolean
dispatch_async_common(TrgClient * tc,
                      trg_request * trg_req,
                      GSourceFunc callback, gpointer data)
{
    TrgClientPrivate *priv = tc->priv;
    GError *error = NULL;

    trg_req->callback = callback;
    trg_req->cb_data = data;
    trg_req->connid = g_atomic_int_get(&priv->connid);

    trg_client_thread_pool_push(tc, trg_req, &error);
    if (error) {
        g_error("thread creation error: %s\n", error->message);
        g_error_free(error);
        trg_request_free(trg_req);
        return FALSE;
    } else {
        return TRUE;
    }
}

gboolean
dispatch_async(TrgClient * tc, JsonNode * req,
               GSourceFunc callback, gpointer data)
{
    trg_request *trg_req = g_new0(trg_request, 1);
    trg_req->node = req;

    return dispatch_async_common(tc, trg_req, callback, data);
}

gboolean async_http_request(TrgClient *tc, gchar *url, const gchar *cookie, GSourceFunc callback, gpointer data) {
	trg_request *trg_req = g_new0(trg_request, 1);
	trg_req->url = g_strdup(url);

	if (cookie)
		trg_req->cookie = g_strdup(cookie);

	return dispatch_async_common(tc, trg_req, callback, data);
}

gboolean trg_client_update_session(TrgClient * tc, GSourceFunc callback,
                                   gpointer data)
{
    return dispatch_async(tc, session_get(), callback, data);
}

gdouble trg_client_get_seed_ratio_limit(TrgClient * tc)
{
    return tc->priv->seedRatioLimit;
}

gboolean trg_client_get_seed_ratio_limited(TrgClient * tc)
{
    return tc->priv->seedRatioLimited;
}
