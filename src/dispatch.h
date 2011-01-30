/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2010  Alan Fitton

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

#ifndef DISPATCH_H_
#define DISPATCH_H_

#include "trg-client.h"

#define FAIL_JSON_DECODE -2
#define FAIL_RESPONSE_UNSUCCESSFUL -3

struct dispatch_async_args {
    gpointer *data;
    JsonNode *req;
    trg_client *client;
    void (*callback) (JsonObject *, int, gpointer);
};

JsonObject *dispatch(trg_client * client, JsonNode * req, int *status);
GThread *dispatch_async(trg_client * client, JsonNode * req,
			void (*callback) (JsonObject *, int, gpointer),
			gpointer data);

#endif				/* DISPATCH_H_ */
