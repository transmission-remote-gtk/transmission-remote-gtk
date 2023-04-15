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

#include <stdio.h>

#include <glib-object.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>

#include "json.h"
#include "protocol-constants.h"
#include "requests.h"
#include "torrent.h"
#include "util.h"

/* A bunch of functions for creating the various requests, in the form of a
 * JsonNode ready for dispatch.
 */

static JsonNode *base_request(gchar *method);

JsonNode *generic_request(gchar *method, JsonArray *ids)
{
    JsonNode *root = base_request(method);

    if (ids) {
        JsonObject *args = node_get_arguments(root);
        json_object_set_array_member(args, PARAM_IDS, ids);
        request_set_tag_from_ids(root, ids);
    }

    return root;
}

JsonNode *session_stats(void)
{
    return base_request(METHOD_SESSION_STATS);
}

JsonNode *blocklist_update(void)
{
    return base_request(METHOD_BLOCKLIST_UPDATE);
}

JsonNode *port_test(void)
{
    return base_request(METHOD_PORT_TEST);
}

JsonNode *session_get(void)
{
    return base_request(METHOD_SESSION_GET);
}

JsonNode *torrent_set_location(JsonArray *array, gchar *location, gboolean move)
{
    JsonNode *req = generic_request(METHOD_TORRENT_SET_LOCATION, array);
    JsonObject *args = node_get_arguments(req);
    json_object_set_boolean_member(args, FIELD_MOVE, move);
    json_object_set_string_member(args, FIELD_LOCATION, location);
    return req;
}

JsonNode *torrent_rename_path(JsonArray *array, const gchar *path, const gchar *name)
{
    JsonNode *req = generic_request(METHOD_TORRENT_RENAME_PATH, array);
    JsonObject *args = node_get_arguments(req);
    json_object_set_string_member(args, FIELD_PATH, path);
    json_object_set_string_member(args, FIELD_NAME, name);
    return req;
}

JsonNode *torrent_start(JsonArray *array)
{
    return generic_request(METHOD_TORRENT_START, array);
}

JsonNode *torrent_pause(JsonArray *array)
{
    return generic_request(METHOD_TORRENT_STOP, array);
}

JsonNode *torrent_reannounce(JsonArray *array)
{
    return generic_request(METHOD_TORRENT_REANNOUNCE, array);
}

JsonNode *torrent_verify(JsonArray *array)
{
    return generic_request(METHOD_TORRENT_VERIFY, array);
}

JsonNode *torrent_queue_move_up(JsonArray *array)
{
    return generic_request(METHOD_QUEUE_MOVE_UP, array);
}

JsonNode *torrent_queue_move_down(JsonArray *array)
{
    return generic_request(METHOD_QUEUE_MOVE_DOWN, array);
}

JsonNode *torrent_start_now(JsonArray *array)
{
    return generic_request(METHOD_TORRENT_START_NOW, array);
}

JsonNode *torrent_queue_move_bottom(JsonArray *array)
{
    return generic_request(METHOD_QUEUE_MOVE_BOTTOM, array);
}

JsonNode *torrent_queue_move_top(JsonArray *array)
{
    return generic_request(METHOD_QUEUE_MOVE_TOP, array);
}

JsonNode *session_set(void)
{
    return generic_request(METHOD_SESSION_SET, NULL);
}

JsonNode *torrent_set(JsonArray *array)
{
    return generic_request(METHOD_TORRENT_SET, array);
}

JsonNode *torrent_remove(JsonArray *array, gboolean removeData)
{
    JsonNode *root = base_request(METHOD_TORRENT_REMOVE);
    JsonObject *args = node_get_arguments(root);

    json_object_set_array_member(args, PARAM_IDS, array);
    json_object_set_boolean_member(args, PARAM_DELETE_LOCAL_DATA, removeData);

    request_set_tag(root, TORRENT_GET_TAG_MODE_FULL);

    return root;
}

JsonNode *torrent_get(gint64 id)
{
    JsonNode *root = base_request(METHOD_TORRENT_GET);
    JsonObject *args = node_get_arguments(root);
    JsonArray *fields = json_array_new();

    if (id == TORRENT_GET_TAG_MODE_UPDATE) {
        json_object_set_string_member(args, PARAM_IDS, FIELD_RECENTLY_ACTIVE);
    } else if (id >= 0) {
        JsonArray *ids = json_array_new();
        json_array_add_int_element(ids, id);
        json_object_set_array_member(args, PARAM_IDS, ids);
    }

    json_array_add_string_element(fields, FIELD_ETA);
    json_array_add_string_element(fields, FIELD_PEERS);
    json_array_add_string_element(fields, FIELD_PEERSFROM);
    json_array_add_string_element(fields, FIELD_FILES);
    json_array_add_string_element(fields, FIELD_PEERS_SENDING_TO_US);
    json_array_add_string_element(fields, FIELD_PEERS_GETTING_FROM_US);
    json_array_add_string_element(fields, FIELD_WEB_SEEDS_SENDING_TO_US);
    json_array_add_string_element(fields, FIELD_PEERS_CONNECTED);
    json_array_add_string_element(fields, FIELD_HAVEVALID);
    json_array_add_string_element(fields, FIELD_HAVEUNCHECKED);
    json_array_add_string_element(fields, FIELD_RATEUPLOAD);
    json_array_add_string_element(fields, FIELD_RATEDOWNLOAD);
    json_array_add_string_element(fields, FIELD_STATUS);
    json_array_add_string_element(fields, FIELD_ISFINISHED);
    json_array_add_string_element(fields, FIELD_ISPRIVATE);
    json_array_add_string_element(fields, FIELD_ADDED_DATE);
    json_array_add_string_element(fields, FIELD_DOWNLOADEDEVER);
    json_array_add_string_element(fields, FIELD_UPLOADEDEVER);
    json_array_add_string_element(fields, FIELD_CORRUPTEVER);
    json_array_add_string_element(fields, FIELD_SIZEWHENDONE);
    json_array_add_string_element(fields, FIELD_QUEUE_POSITION);
    json_array_add_string_element(fields, FIELD_ID);
    json_array_add_string_element(fields, FIELD_NAME);
    json_array_add_string_element(fields, FIELD_PERCENTDONE);
    json_array_add_string_element(fields, FIELD_COMMENT);
    json_array_add_string_element(fields, FIELD_TOTAL_SIZE);
    json_array_add_string_element(fields, FIELD_METADATAPERCENTCOMPLETE);
    json_array_add_string_element(fields, FIELD_LEFT_UNTIL_DONE);
    json_array_add_string_element(fields, FIELD_ANNOUNCE_URL);
    json_array_add_string_element(fields, FIELD_ERROR_STRING);
    json_array_add_string_element(fields, FIELD_TRACKER_STATS);
    json_array_add_string_element(fields, FIELD_DATE_CREATED);
    json_array_add_string_element(fields, FIELD_DOWNLOAD_DIR);
    json_array_add_string_element(fields, FIELD_CREATOR);
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
    json_array_add_string_element(fields, FIELD_ACTIVITY_DATE);
    json_array_add_string_element(fields, FIELD_MAGNETLINK);
    json_array_add_string_element(fields, FIELD_ERROR);
    json_array_add_string_element(fields, FIELD_ERROR_STRING);
    json_array_add_string_element(fields, FIELD_WANTED);
    json_array_add_string_element(fields, FIELD_PRIORITIES);
    json_array_add_string_element(fields, FIELD_RECHECK_PROGRESS);
    json_object_set_array_member(args, PARAM_FIELDS, fields);
    return root;
}

JsonNode *torrent_add_url(const gchar *url, gboolean paused)
{
    JsonNode *root = base_request(METHOD_TORRENT_ADD);
    JsonObject *args = node_get_arguments(root);

    json_object_set_string_member(args, PARAM_FILENAME, url);
    json_object_set_boolean_member(args, PARAM_PAUSED, paused);
    request_set_tag(root, TORRENT_GET_TAG_MODE_FULL);
    return root;
}

static void trash_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    g_autoptr(GError) error = NULL;

    if (!g_file_trash_finish(G_FILE(source_object), res, &error))
        g_warning("Failed to trash '%s': %s", g_file_peek_path(G_FILE(source_object)),
                  error->message);
}

JsonNode *torrent_add_from_file(gchar *target, gint flags, GError **error)
{
    JsonNode *root;
    JsonObject *args;
    gboolean isMagnet = is_magnet(target);
    gboolean isUri = isMagnet || is_url(target);
    gchar *encodedFile;

    if (!isUri && !g_file_test(target, G_FILE_TEST_IS_REGULAR)) {
        g_message("file \"%s\" does not exist.", target);
        return NULL;
    }

    root = base_request(METHOD_TORRENT_ADD);
    args = node_get_arguments(root);

    if (isUri) {
        json_object_set_string_member(args, PARAM_FILENAME, target);
    } else {
        encodedFile = trg_base64encode(target, error);
        if (!encodedFile)
            return NULL;

        json_object_set_string_member(args, PARAM_METAINFO, encodedFile);
        g_free(encodedFile);
    }

    json_object_set_boolean_member(args, PARAM_PAUSED, (flags & TORRENT_ADD_FLAG_PAUSED));

    if ((flags & TORRENT_ADD_FLAG_DELETE)) {
        g_autoptr(GFile) file = NULL;

        file = g_file_new_for_path(target);
        g_file_trash_async(file, G_PRIORITY_DEFAULT, NULL, trash_cb, NULL);
    }

    return root;
}

static JsonNode *base_request(gchar *method)
{
    JsonNode *root = json_node_new(JSON_NODE_OBJECT);
    JsonObject *object = json_object_new();
    JsonObject *args = json_object_new();

    json_object_set_string_member(object, PARAM_METHOD, method);
    json_object_set_object_member(object, PARAM_ARGUMENTS, args);
    json_node_take_object(root, object);

    return root;
}

void request_set_tag(JsonNode *req, gint64 tag)
{
    json_object_set_int_member(json_node_get_object(req), PARAM_TAG, tag);
}

void request_set_tag_from_ids(JsonNode *req, JsonArray *ids)
{
    gint64 id = json_array_get_length(ids) == 1 ? json_array_get_int_element(ids, 0)
                                                : TORRENT_GET_TAG_MODE_FULL;
    request_set_tag(req, id);
}
