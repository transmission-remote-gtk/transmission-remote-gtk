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

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "protocol-constants.h"
#include "base64.h"
#include "json.h"
#include "requests.h"

static JsonNode *base_request(gchar * method);

JsonNode *generic_request(gchar * method, JsonArray * ids)
{
    JsonNode *root = base_request(method);

    if (ids != NULL)
	json_object_set_array_member(node_get_arguments(root),
				     PARAM_IDS, ids);

    return root;
}

JsonNode *session_get()
{
    return generic_request(METHOD_SESSION_GET, NULL);
}

JsonNode *torrent_start(JsonArray * array)
{
    return generic_request(METHOD_TORRENT_START, array);
}

JsonNode *torrent_pause(JsonArray * array)
{
    return generic_request(METHOD_TORRENT_STOP, array);
}

JsonNode *torrent_verify(JsonArray * array)
{
    return generic_request(METHOD_TORRENT_VERIFY, array);
}

JsonNode *session_set(void)
{
    return generic_request(METHOD_SESSION_SET, NULL);
}

JsonNode *torrent_set(JsonArray * array)
{
    return generic_request(METHOD_TORRENT_SET, array);
}

JsonNode *torrent_remove(JsonArray * array, gboolean removeData)
{
    JsonNode *root = base_request(METHOD_TORRENT_REMOVE);
    JsonObject *args = node_get_arguments(root);
    json_object_set_array_member(args, PARAM_IDS, array);
    json_object_set_boolean_member(args, PARAM_DELETE_LOCAL_DATA,
				   removeData);
    return root;
}

JsonNode *torrent_get()
{
    JsonNode *root = base_request(METHOD_TORRENT_GET);
    JsonArray *fields = json_array_new();
    json_array_add_string_element(fields, FIELD_ETA);
    json_array_add_string_element(fields, FIELD_PEERS);
    json_array_add_string_element(fields, FIELD_FILES);
    json_array_add_string_element(fields, FIELD_HAVEVALID);
    json_array_add_string_element(fields, FIELD_HAVEUNCHECKED);
    json_array_add_string_element(fields, FIELD_RATEUPLOAD);
    json_array_add_string_element(fields, FIELD_RATEDOWNLOAD);
    json_array_add_string_element(fields, FIELD_STATUS);
    json_array_add_string_element(fields, FIELD_UPLOADEDEVER);
    json_array_add_string_element(fields, FIELD_SIZEWHENDONE);
    json_array_add_string_element(fields, FIELD_ID);
    json_array_add_string_element(fields, FIELD_NAME);
    json_array_add_string_element(fields, FIELD_PERCENTDONE);
    json_array_add_string_element(fields, FIELD_COMMENT);
    json_array_add_string_element(fields, FIELD_ADDED_DATE);
    json_array_add_string_element(fields, FIELD_TOTAL_SIZE);
    json_array_add_string_element(fields, FIELD_LEFT_UNTIL_DONE);
    json_array_add_string_element(fields, FIELD_ANNOUNCE_URL);
    json_array_add_string_element(fields, FIELD_ERROR_STRING);
    json_array_add_string_element(fields, FIELD_SWARM_SPEED);
    json_array_add_string_element(fields, FIELD_TRACKERS);
    json_array_add_string_element(fields, FIELD_DOWNLOAD_DIR);
    json_array_add_string_element(fields, FIELD_HASH_STRING);
    json_array_add_string_element(fields, FIELD_DONE_DATE);
    json_array_add_string_element(fields, FIELD_HONORS_SESSION_LIMITS);
    json_array_add_string_element(fields, FIELD_UPLOAD_LIMIT);
    json_array_add_string_element(fields, FIELD_UPLOAD_LIMITED);
    json_array_add_string_element(fields, FIELD_DOWNLOAD_LIMIT);
    json_array_add_string_element(fields, FIELD_DOWNLOAD_LIMITED);
    json_array_add_string_element(fields, FIELD_BANDWIDTH_PRIORITY);
    json_array_add_string_element(fields, FIELD_SEED_RATIO_LIMIT);
    json_array_add_string_element(fields, FIELD_SEED_RATIO_MODE);
    json_array_add_string_element(fields, FIELD_PEER_LIMIT);
    json_array_add_string_element(fields, FIELD_ERRORSTR);
    json_array_add_string_element(fields, FIELD_WANTED);
    json_array_add_string_element(fields, FIELD_PRIORITIES);
    json_object_set_array_member(node_get_arguments(root),
				 PARAM_FIELDS, fields);
    return root;
}

JsonNode *torrent_add_url(const gchar * url, gboolean paused)
{
    JsonNode *root = base_request(METHOD_TORRENT_ADD);
    JsonObject *args = node_get_arguments(root);
    json_object_set_string_member(args, PARAM_FILENAME, url);
    return root;
}

JsonNode *torrent_add(gchar * filename, gboolean paused)
{
    JsonNode *root = base_request(METHOD_TORRENT_ADD);
    JsonObject *args = node_get_arguments(root);
    gchar *encodedFile = base64encode(filename);
    json_object_set_string_member(args, PARAM_METAINFO, encodedFile);
    g_free(encodedFile);
    return root;
}

static JsonNode *base_request(gchar * method)
{
    JsonNode *root = json_node_new(JSON_NODE_OBJECT);
    JsonObject *object = json_object_new();
    JsonObject *args = json_object_new();
    json_object_set_string_member(object, PARAM_METHOD, method);
    json_object_set_object_member(object, PARAM_ARGUMENTS, args);
    json_node_take_object(root, object);
    return root;
}
