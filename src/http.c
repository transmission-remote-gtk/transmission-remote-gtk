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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"
#include "http.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

static struct http_response *trg_http_perform_inner(trg_client * client,
                                                    gchar * req,
                                                    gboolean recurse);

static size_t http_receive_callback(void *ptr, size_t size, size_t nmemb,
                                    void *data);

static size_t header_callback(void *ptr, size_t size, size_t nmemb,
                              void *data);

void http_response_free(struct http_response *response)
{
    if (response->data != NULL)
        g_free(response->data);

    g_free(response);
}

static struct http_response *trg_http_perform_inner(trg_client * tc,
                                                    gchar * req,
                                                    gboolean recurse)
{
    CURL *handle;
    long httpCode;
    struct http_response *response;
    struct curl_slist *headers = NULL;

    response = g_new(struct http_response, 1);
    response->size = 0;
    response->status = -1;
    response->data = NULL;

    handle = curl_easy_init();

    curl_easy_setopt(handle, CURLOPT_USERAGENT, PACKAGE_NAME);
    curl_easy_setopt(handle, CURLOPT_PASSWORD, tc->password);
    curl_easy_setopt(handle, CURLOPT_USERNAME, tc->username);
    curl_easy_setopt(handle, CURLOPT_URL, tc->url);
    curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION,
                     &http_receive_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *) response);
    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, &header_callback);
    curl_easy_setopt(handle, CURLOPT_WRITEHEADER, (void *) tc);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, req);

    if (tc->ssl)
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);

    if (tc->proxy) {
        curl_easy_setopt(handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
        curl_easy_setopt(handle, CURLOPT_PROXY, tc->proxy);
    }

    if (tc->session_id) {
        headers = curl_slist_append(headers, tc->session_id);
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
    }

    response->status = curl_easy_perform(handle);

    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(handle);

    if (headers != NULL)
        curl_slist_free_all(headers);

    if (response->status == CURLE_OK) {
        if (httpCode == HTTP_CONFLICT && recurse == TRUE) {
            http_response_free(response);
            return trg_http_perform_inner(tc, req, FALSE);
        } else if (httpCode != HTTP_OK) {
            response->status = (-httpCode) - 100;
        }
    }

    return response;
}

struct http_response *trg_http_perform(trg_client * tc, gchar * req)
{
    return trg_http_perform_inner(tc, req, TRUE);
}

static size_t
http_receive_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct http_response *mem = (struct http_response *) data;

    mem->data = realloc(mem->data, mem->size + realsize + 1);
    if (mem->data) {
        memcpy(&(mem->data[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->data[mem->size] = 0;
    }
    return realsize;
}

static size_t header_callback(void *ptr, size_t size, size_t nmemb,
                              void *data)
{
    char *header = (char *) (ptr);
    trg_client *client = (trg_client *) data;

    if (g_str_has_prefix(header, "X-Transmission-Session-Id: ")) {
        char *nl;

        if (client->session_id)
            g_free(client->session_id);

        client->session_id = g_strdup(header);
        nl = strrchr(client->session_id, '\r');
        if (nl)
            *nl = '\0';
    }

    return (nmemb * size);
}
