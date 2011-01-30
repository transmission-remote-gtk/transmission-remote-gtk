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

#ifndef HTTP_H_
#define HTTP_H_

#include <stdlib.h>

#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <curl/curl.h>

#include "trg-client.h"

#define HTTP_OK 200
#define HTTP_CONFLICT 409

struct http_response {
    int status;
    char *data;
    int size;
};

void http_response_free(struct http_response *response);
struct http_response *trg_http_perform(trg_client * client, gchar * req);

#endif				/* HTTP_H_ */
