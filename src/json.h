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

#ifndef JSON_H_
#define JSON_H_

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"

JsonGenerator *trg_json_serializer(JsonNode *req, gboolean pretty);
JsonObject *get_arguments(JsonObject *req);
JsonObject *node_get_arguments(JsonNode *req);
gdouble json_double_to_progress(JsonNode *n);
gdouble json_node_really_get_double(JsonNode *node);

#endif /* JSON_H_ */
