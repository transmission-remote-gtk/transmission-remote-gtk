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

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <libsoup/soup.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "protocol-constants.h"
#include "requests.h"
#include "trg-client.h"
#include "trg-prefs.h"
#include "util.h"

/* This class manages/does quite a few things, and is passed around a lot. It:
 *
 * 1) Holds/inits the single TrgPrefs object for managing configuration.
 * 3) Holds SoupSession for connection reuse
 * 4) Holds a hash table for looking up a torrent by its ID.
 * 5) Dispatches asyncrhonous requests and tracks failures.
 * 6) Holds connection state, an update serial, and provides signals for
 *    connect/disconnect.
 * 7) Provides a mutex for locking updates.
 * 8) Holds the latest session object sent in a session-get response.
 */
struct _TrgClient {
    GObject parent;

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
    GUri *url;
    char *username;
    char *password;
    GHashTable *headers;
    GHashTable *torrentTable;
    TrgPrefs *prefs;
    SoupSession *rpc_session;
    gint configSerial;
    GMutex configMutex;
    gboolean seedRatioLimited;
    gdouble seedRatioLimit;
};

enum {
    TC_SESSION_UPDATED,
    TC_SIGNAL_COUNT
};

static guint signals[TC_SIGNAL_COUNT] = { 0 };

G_DEFINE_TYPE(TrgClient, trg_client, G_TYPE_OBJECT)
static void trg_client_get_property(GObject *object, guint property_id, GValue *value,
                                    GParamSpec *pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_client_set_property(GObject *object, guint property_id, const GValue *value,
                                    GParamSpec *pspec)
{
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_client_dispose(GObject *object)
{
    TrgClient *self = TRG_CLIENT(object);
    soup_session_abort(self->rpc_session);
    g_object_unref(self->rpc_session);
    G_OBJECT_CLASS(trg_client_parent_class)->dispose(object);
}

static void trg_client_finalize(GObject *object)
{
    TrgClient *self = TRG_CLIENT(object);

    g_clear_pointer(&self->session_id, g_free);
    g_clear_pointer(&self->session, json_object_unref);
    g_clear_pointer(&self->url, g_uri_unref);
    g_clear_pointer(&self->username, g_free);
    g_clear_pointer(&self->password, g_free);
    g_clear_pointer(&self->headers, g_hash_table_unref);
    g_clear_pointer(&self->torrentTable, g_hash_table_unref);
    G_OBJECT_CLASS(trg_client_parent_class)->finalize(object);
}

static void trg_client_class_init(TrgClientClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_client_get_property;
    object_class->set_property = trg_client_set_property;
    object_class->dispose = trg_client_dispose;
    object_class->finalize = trg_client_finalize;

    signals[TC_SESSION_UPDATED] = g_signal_new(
        "session-updated", G_TYPE_FROM_CLASS(object_class), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0,
        NULL, NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void trg_client_init(TrgClient *self)
{
}

TrgClient *trg_client_new(void)
{
    TrgClient *self = TRG_CLIENT(g_object_new(TRG_TYPE_CLIENT, NULL));

    TrgPrefs *prefs = self->prefs = trg_prefs_new();
    self->rpc_session = soup_session_new_with_options("user-agent", PACKAGE_NAME, NULL);

    if (g_getenv("TRG_CLIENT_DEBUG") != NULL) {
        g_autoptr(SoupLogger) log = soup_logger_new(SOUP_LOGGER_LOG_BODY);
        soup_session_add_feature(self->rpc_session, SOUP_SESSION_FEATURE(log));
    }

    trg_prefs_load(prefs);

    g_mutex_init(&self->configMutex);
    self->seedRatioLimited = FALSE;
    self->seedRatioLimit = 0.00;

    tr_formatter_size_init(disk_K, _(disk_K_str), _(disk_M_str), _(disk_G_str), _(disk_T_str));
    tr_formatter_speed_init(speed_K, _(speed_K_str), _(speed_M_str), _(speed_G_str),
                            _(speed_T_str));

    return self;
}

const gchar *trg_client_get_version_string(TrgClient *tc)
{
    return session_get_version_string(tc->session);
}

gdouble trg_client_get_version(TrgClient *tc)
{
    return tc->version;
}

gint64 trg_client_get_rpc_version(TrgClient *tc)
{
    return session_get_rpc_version(tc->session);
}

void trg_client_inc_connid(TrgClient *tc)
{
    g_atomic_int_inc(&tc->connid);
}

static gint trg_client_get_connid(TrgClient *tc)
{
    return g_atomic_int_get(&tc->connid);
}

void trg_client_set_session(TrgClient *tc, JsonObject *session)
{

    if (tc->session)
        json_object_unref(tc->session);
    else
        tc->version = session_get_version(session);

    tc->session = session;
    json_object_ref(session);

    tc->seedRatioLimit = session_get_seed_ratio_limit(session);
    tc->seedRatioLimited = session_get_seed_ratio_limited(session);

    g_signal_emit(tc, signals[TC_SESSION_UPDATED], 0, session);
}

TrgPrefs *trg_client_get_prefs(TrgClient *tc)
{
    return tc->prefs;
}

static GHashTable *trg_client_headers_array_to_table(JsonArray *array)
{
    GList *nodes, *nodes_iter;
    const gchar *key, *value;
    JsonNode *header_node;
    GHashTable *headers;

    if (!array)
        return NULL;

    nodes = json_array_get_elements(array);
    if (!nodes)
        return NULL;

    headers = g_hash_table_new(NULL, NULL);
    for (nodes_iter = g_list_first(nodes); nodes_iter; nodes_iter = g_list_next(nodes_iter)) {
        header_node = nodes_iter->data;
        if (!header_node)
            continue;
        JsonObject *header_object;
        header_object = json_node_get_object(header_node);
        if (!header_object)
            continue;

        key = json_object_get_string_member(header_object, TRG_PREFS_KEY_CUSTOM_HEADER_NAME);
        value = json_object_get_string_member(header_object, TRG_PREFS_KEY_CUSTOM_HEADER_VALUE);
        if (key && value)
            g_hash_table_insert(headers, (gpointer)key, (gpointer)value);
    }

    return headers;
}

static void trg_client_inject_custom_header(gpointer key, gpointer value, gpointer user_data)
{
    SoupMessageHeaders *headers = user_data;
    soup_message_headers_replace(headers, (gchar *)key, (gchar *)value);
}

gboolean trg_client_parse_settings(TrgClient *tc, gchar **err_msg)
{
    TrgPrefs *prefs = tc->prefs;
    JsonArray *headers;
    g_autoptr(GError) uri_err = NULL;
    gint port;

    g_mutex_lock(&tc->configMutex);

    trg_prefs_set_connection(prefs, trg_prefs_get_profile(prefs));

    g_clear_pointer(&tc->url, g_uri_unref);
    g_clear_pointer(&tc->username, g_free);
    g_clear_pointer(&tc->password, g_free);
    g_clear_pointer(&tc->headers, g_hash_table_unref);
    tc->headers = NULL;

    port = trg_prefs_get_int(prefs, TRG_PREFS_KEY_PORT, TRG_PREFS_CONNECTION);
    g_autofree gchar *host
        = trg_prefs_get_string(prefs, TRG_PREFS_KEY_HOSTNAME, TRG_PREFS_CONNECTION);
    g_autofree gchar *path
        = trg_prefs_get_string(prefs, TRG_PREFS_KEY_RPC_URL_PATH, TRG_PREFS_CONNECTION);

    if (!host || strlen(host) < 1) {
        g_mutex_unlock(&tc->configMutex);
        *err_msg = g_strdup("Bad hostname.");
        return FALSE;
    }

    tc->ssl = trg_prefs_get_bool(prefs, TRG_PREFS_KEY_SSL, TRG_PREFS_CONNECTION);
    tc->ssl_validate = trg_prefs_get_bool(prefs, TRG_PREFS_KEY_SSL_VALIDATE, TRG_PREFS_CONNECTION);

    g_autofree gchar *uri_str = g_strdup_printf(
        "%s://%s:%d%s", tc->ssl ? HTTPS_URI_PREFIX : HTTP_URI_PREFIX, host, port, path);

    tc->url = g_uri_parse(uri_str, G_URI_FLAGS_NONE, &uri_err);
    if (uri_err) {
        g_mutex_unlock(&tc->configMutex);
        *err_msg = g_strdup(uri_err->message);
        return FALSE;
    }

    tc->username = trg_prefs_get_string(prefs, TRG_PREFS_KEY_USERNAME, TRG_PREFS_CONNECTION);
    tc->password = trg_prefs_get_string(prefs, TRG_PREFS_KEY_PASSWORD, TRG_PREFS_CONNECTION);

    headers = trg_prefs_get_array(prefs, TRG_PREFS_KEY_CUSTOM_HEADERS, TRG_PREFS_CONNECTION);
    if (headers)
        tc->headers = trg_client_headers_array_to_table(headers);

    tc->configSerial++;
    g_mutex_unlock(&tc->configMutex);
    return TRUE;
}

gchar *trg_client_get_password(TrgClient *tc)
{
    return tc->password;
}

gchar *trg_client_get_username(TrgClient *tc)
{
    return tc->username;
}

gchar *trg_client_get_session_id(TrgClient *tc)
{
    gchar *ret;

    g_mutex_lock(&tc->configMutex);

    ret = tc->session_id ? g_strdup(tc->session_id) : NULL;

    g_mutex_unlock(&tc->configMutex);

    return ret;
}

void trg_client_set_session_id(TrgClient *tc, gchar *session_id)
{
    g_mutex_lock(&tc->configMutex);

    g_clear_pointer(&tc->session_id, g_free);
    tc->session_id = session_id;

    g_mutex_unlock(&tc->configMutex);
}

void trg_client_status_change(TrgClient *tc, gboolean connected)
{
    if (!connected) {
        g_clear_pointer(&tc->session, json_object_unref);
        g_mutex_lock(&tc->configMutex);
        trg_prefs_set_connection(tc->prefs, NULL);
        g_mutex_unlock(&tc->configMutex);
    }
}

JsonObject *trg_client_get_session(TrgClient *tc)
{
    return tc->session;
}

void trg_client_inc_serial(TrgClient *tc)
{
    tc->updateSerial++;
}

gint64 trg_client_get_serial(TrgClient *tc)
{
    return tc->updateSerial;
}

gboolean trg_client_get_ssl(TrgClient *tc)
{
    return tc->ssl;
}

gboolean trg_client_get_ssl_validate(TrgClient *tc)
{
    return tc->ssl_validate;
}

void trg_client_set_torrent_table(TrgClient *tc, GHashTable *table)
{
    tc->torrentTable = table;
}

GHashTable *trg_client_get_torrent_table(TrgClient *tc)
{
    return tc->torrentTable;
}

gboolean trg_client_is_connected(TrgClient *tc)
{
    return tc->session != NULL;
}

void trg_client_configlock(TrgClient *tc)
{
    g_mutex_lock(&tc->configMutex);
}

guint trg_client_get_failcount(TrgClient *tc)
{
    return tc->failCount;
}

guint trg_client_inc_failcount(TrgClient *tc)
{
    return ++(tc->failCount);
}

void trg_client_reset_failcount(TrgClient *tc)
{
    tc->failCount = 0;
}

void trg_client_configunlock(TrgClient *tc)
{
    g_mutex_unlock(&tc->configMutex);
}

void trg_response_free(trg_response *response)
{
    if (response) {
        g_clear_pointer(&response->obj, json_object_unref);
        g_clear_pointer(&response->err_msg, g_free);
        g_free(response);
    }
}

void trg_client_update_session(TrgClient *tc, GSourceFunc callback, gpointer data)
{
    dispatch_rpc_async(tc, session_get(), callback, data);
}

gdouble trg_client_get_seed_ratio_limit(TrgClient *tc)
{
    return tc->seedRatioLimit;
}

gboolean trg_client_get_seed_ratio_limited(TrgClient *tc)
{
    return tc->seedRatioLimited;
}

/* rpc_request struct manages state for callbacks */
typedef struct {
    TrgClient *client;
    SoupMessage *msg;
    GCancellable *cancellable;
    gint connid;
    GBytes *body;
    GSourceFunc response_cb;
    gpointer *cb_data;
} trg_request;

/* request handling */
static trg_request *trg_request_new(TrgClient *tc, GBytes *body, GSourceFunc cb, gpointer cb_data)
{
    trg_request *request = (trg_request *)g_new0(trg_request, 1);

    request->msg = NULL;
    request->cancellable = NULL;

    g_object_ref(tc);
    request->client = tc;
    request->connid = trg_client_get_connid(tc);

    request->body = body;
    request->response_cb = cb;
    request->cb_data = cb_data;

    return request;
}

static void session_id_callback(SoupMessage *msg, gpointer user_data);
static gboolean auth_callback(SoupMessage *msg, SoupAuth *auth, gboolean retry, gpointer user_data);
static void rpc_callback(GObject *source, GAsyncResult *result, gpointer user_data);
static gboolean tls_callback(SoupMessage *msg, GTlsCertificate *cert,
                             GTlsCertificateFlags tls_errors, gpointer user_data);

static void trg_request_setup_msg(trg_request *request)
{
    TrgClient *self = request->client;
    SoupMessage *msg;
    SoupMessageHeaders *req_headers;

    /* setup_msg is also called if a previous message failed, so make sure it's clear */
    if (request->msg) {
        g_clear_object(&request->msg);
        g_clear_object(&request->cancellable);
    }

    msg = soup_message_new_from_uri(SOUP_METHOD_POST, self->url);

    request->cancellable = g_cancellable_new();

    req_headers = soup_message_get_request_headers(msg);
    if (self->headers)
        g_hash_table_foreach(self->headers, trg_client_inject_custom_header, req_headers);

    g_autofree gchar *session_id = trg_client_get_session_id(request->client);
    if (session_id) {
        soup_message_headers_replace(req_headers, TRANSMISSION_SESSION_ID_HEADER, session_id);
    }

    g_signal_connect(msg, "accept-certificate", G_CALLBACK(tls_callback), (gpointer)request);
    g_signal_connect(msg, "authenticate", G_CALLBACK(auth_callback), (gpointer)request);
    soup_message_add_status_code_handler(msg, "got-headers", SOUP_STATUS_CONFLICT,
                                         G_CALLBACK(session_id_callback), request);

    request->msg = msg;
}

static void trg_request_set_body(trg_request *request)
{
    /* body must be reset every time auth/redirection is done */
    soup_message_set_request_body_from_bytes(request->msg, "application/json", request->body);
}

static void trg_request_send(trg_request *request)
{
    TrgClient *self = request->client;
    soup_session_send_and_read_async(self->rpc_session, request->msg, G_PRIORITY_DEFAULT,
                                     request->cancellable, rpc_callback, request);
}

static void trg_request_free(trg_request *request)
{
    g_clear_object(&request->client);
    g_clear_object(&request->msg);
    g_clear_object(&request->cancellable);
    g_clear_pointer(&request->body, g_bytes_unref);
    g_clear_pointer(&request, g_free);
}

/* Soup Async callbacks for RPC
 *
 * dispatch_rpc_async() is the entry point for all async RPC requests, and is a standard callback
 * based async function. This sets up a soup message with the right headers. The SoupSession will
 * re-use connection and authentication as long as it is valid. The callbacks then cascade:
 *
 * 1. soup_send_async(): send the message async
 * 2. session_id_callback()/auth_callback()/tls_callback(): Three optional callbacks called if 409
 * is found, auth is needed, or tls_certs have errors.
 * 3. rpc_callback(): called on successful response from server
 * 4. trg_request_callback(): calls the original callback passed to dispatch_rpc_async(), sets
 *    up a trg_response and/or passes back any errors and state
 * 5. response_cb(): original callback passed to dispatch_rpc_async().
 */

static void trg_request_callback(trg_request *request, JsonObject *obj, gint status, gchar *err_msg)
{
    if ((request->connid != trg_client_get_connid(request->client)) || !(request->response_cb)) {
        g_clear_pointer(&err_msg, g_free);
        g_clear_pointer(&obj, json_object_unref);
        trg_request_free(request);
        return;
    }

    GSourceFunc response_cb = request->response_cb;
    gpointer cb_data = request->cb_data;

    trg_response *response = (trg_response *)g_new0(trg_response, 1);

    response->obj = obj;
    response->cb_data = cb_data;
    response->status = status;
    response->err_msg = err_msg;

    trg_request_free(request);
    response_cb(response);
}

static void rpc_callback(GObject *source, GAsyncResult *result, gpointer user_data)
{
    trg_request *request = user_data;
    g_autofree gchar *data = NULL;
    g_autoptr(GError) error = NULL;
    g_autoptr(JsonParser) parser = NULL;
    g_autoptr(JsonNode) root = NULL;
    JsonObject *obj = NULL;
    gint status = SOUP_STATUS_OK;
    gsize len;
    gchar *err_msg = NULL;
    JsonNode *rpc_result;

    GBytes *bytes = soup_session_send_and_read_finish(SOUP_SESSION(source), result, &error);
    if (error) {
        if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            return;

        status = FAIL_HTTP_UNSUCCESSFUL;
        err_msg = g_strdup(error->message);
        goto out;
    }

    status = soup_message_get_status(request->msg);
    if (status != SOUP_STATUS_OK) {
        goto out;
    }

    /* TODO(?): Switch this to json_parser_load_from_stream_async() when libsoup
     * can handle threading better. That function works in a thread underneath
     * the hood so libsoup has trouble with it. See libsoup #307 */
    parser = json_parser_new();
    data = (gchar *)g_bytes_unref_to_data(bytes, &len);

    // Potential Transmission bug, we need to validate utf-8, see #261
    if (!g_utf8_validate(data, len, NULL)) {
        // This may be expensive, but it prevents errors
        gchar *new_data = g_utf8_make_valid(data, len);

        g_warning(
            "Invalid JSON received from Transmission, fixing it, but data may be wrong/corrupted");

        g_free(data);
        data = new_data;
    }

    if (!json_parser_load_from_data(parser, data, len, &error)) {
        status = FAIL_JSON_DECODE;
        err_msg = g_strdup(error->message);
        goto out;
    }

    root = json_parser_steal_root(parser);
    if (!root) {
        status = FAIL_JSON_DECODE;
        goto out;
    }

    obj = json_node_dup_object(root);
    json_object_seal(obj);

    rpc_result = json_object_get_member(obj, FIELD_RESULT);
    if (!rpc_result || g_strcmp0(json_node_get_string(rpc_result), FIELD_SUCCESS))
        status = FAIL_RESULT_UNSUCCESSFUL;

out:
    trg_request_callback(request, obj, status, err_msg);
}

static gboolean tls_callback(SoupMessage *msg, GTlsCertificate *cert,
                             GTlsCertificateFlags tls_errors, gpointer user_data)
{
    trg_request *request = user_data;
    return !trg_client_get_ssl_validate(request->client);
}

static gboolean auth_callback(SoupMessage *msg, SoupAuth *auth, gboolean retry, gpointer user_data)
{
    trg_request *request = user_data;

    if (retry)
        return FALSE;

    soup_auth_authenticate(auth, trg_client_get_username(request->client),
                           trg_client_get_password(request->client));

    trg_request_set_body(request);

    return TRUE;
}

static void session_id_callback(SoupMessage *msg, gpointer data)
{
    SoupMessageHeaders *response_headers;
    trg_request *request = data;
    const gchar *session_id;

    response_headers = soup_message_get_response_headers(msg);
    session_id = soup_message_headers_get_one(response_headers, TRANSMISSION_SESSION_ID_HEADER);

    g_cancellable_cancel(request->cancellable);

    if (!session_id) {
        /* If we get a 409 and there's no session_id header this likely isn't transmission */
        trg_request_callback(request, NULL, FAIL_NO_SESSION_ID, NULL);
        return;
    }

    trg_client_set_session_id(request->client, g_strdup(session_id));

    trg_request_setup_msg(request);
    trg_request_set_body(request);
    trg_request_send(request);
}

void dispatch_rpc_async(TrgClient *tc, JsonNode *req, GSourceFunc callback, gpointer data)
{
    GBytes *req_bytes;
    gchar *req_body;
    gsize len;
    g_autoptr(JsonGenerator) generator = NULL;

    /* Note: ownership of req_body is taken by g_bytes_new_take() and will
     * be freed when req_bytes is freed */
    generator = trg_json_serializer(req, FALSE);
    req_body = json_generator_to_data(generator, &len);
    req_bytes = g_bytes_new_take((gpointer)req_body, len);

    trg_request *request = trg_request_new(tc, req_bytes, callback, data);
    trg_request_setup_msg(request);

    trg_request_set_body(request);
    trg_request_send(request);
}
