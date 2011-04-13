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

#include <glib.h>
#include <glib/gprintf.h>
#include <json-glib/json-glib.h>
#include <curl/curl.h>

#include "config.h"
#include "dispatch.h"
#include "http.h"
#include "json.h"

static void dispatch_async_threadfunc(gpointer task, gpointer user_data);

JsonObject *dispatch(trg_client * client, JsonNode * req, int *status)
{
    gchar *serialized;
    struct http_response *response;
    JsonObject *deserialized;
    JsonNode *result;
    GError *decode_error = NULL;

    serialized = trg_serialize(req);
    json_node_free(req);
#ifdef DEBUG
    if (g_getenv("TRG_SHOW_OUTGOING"))
        g_printf("=>(outgoing)=> %s\n", serialized);
#endif
    response = trg_http_perform(client, serialized);
    g_free(serialized);

    if (status)
        *status = response->status;

    if (response->status != CURLE_OK) {
        http_response_free(response);
        return NULL;
    }

    deserialized = trg_deserialize(response, &decode_error);
    http_response_free(response);

    if (decode_error) {
        g_printf("JSON decoding error: %s\n", decode_error->message);
        g_error_free(decode_error);
        if (status)
            *status = FAIL_JSON_DECODE;
        return NULL;
    }

    result = json_object_get_member(deserialized, "result");
    if (status
        && (!result
            || g_strcmp0(json_node_get_string(result), "success")))
        *status = FAIL_RESPONSE_UNSUCCESSFUL;

    return deserialized;
}

static void dispatch_async_threadfunc(gpointer task, gpointer user_data)
{
    trg_client *client = (trg_client*)user_data;
    struct DispatchAsyncData *args = (struct DispatchAsyncData*)task;

    int status;
    JsonObject *result = dispatch(client, args->req, &status);
    if (args->callback)
        args->callback(result, status, args->data);
    g_free(args);
}

GThreadPool *dispatch_init_pool(trg_client *client)
{
    GThreadPool *pool = g_thread_pool_new((GFunc)dispatch_async_threadfunc, client, DISPATCH_POOL_SIZE, FALSE, NULL);
    return pool;
}

gboolean dispatch_async(trg_client * client, JsonNode * req,
                        void (*callback) (JsonObject *, int, gpointer),
                        gpointer data)
{
    GError *error = NULL;
    struct DispatchAsyncData *args =
        g_new(struct DispatchAsyncData, 1);

    args->callback = callback;
    args->data = data;
    args->req = req;

    g_thread_pool_push(client->pool, args, &error);
    if (error) {
        g_printf("thread creation error: %s\n", error->message);
        g_error_free(error);
        g_free(args);
        return FALSE;
    } else {
        return TRUE;
    }
}
