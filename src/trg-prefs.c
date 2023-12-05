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
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>

#include "json.h"
#include "torrent.h"
#include "trg-client.h"
#include "trg-prefs.h"
#include "util.h"

/* I replaced GConf with this custom configuration backend for a few reasons.
 * 1) Better windows support. No dependency on DBus.
 * 2) We're including a JSON parser/writer anyway.
 * 3) The GConf API is pretty verbose sometimes, working with lists is awkward.
 * 4) Easily switch between profiles, and scope config to either profiles or global.
 * 5) Transparently use configurables from the current connection, not current profile.
 */

struct _TrgPrefs {
    GObject parent;

    JsonObject *defaultsObj;
    JsonNode *user;
    JsonObject *userObj;
    JsonObject *connectionObj;
    JsonObject *profile;
    gchar *file;
};

G_DEFINE_TYPE(TrgPrefs, trg_prefs, G_TYPE_OBJECT)

enum {
    PREF_CHANGE,
    PREF_PROFILE_CHANGE,
    PREFS_SIGNAL_COUNT
};

static guint signals[PREFS_SIGNAL_COUNT] = { 0 };

void trg_prefs_profile_change_emit_signal(TrgPrefs *p)
{
    g_signal_emit(p, signals[PREF_PROFILE_CHANGE], 0);
}

void trg_prefs_changed_emit_signal(TrgPrefs *p, const gchar *key)
{
    g_signal_emit(p, signals[PREF_CHANGE], 0, key);
}

static void trg_prefs_get_property(GObject *object, guint property_id, GValue *value,
                                   GParamSpec *pspec)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

static void trg_prefs_set_property(GObject *object, guint property_id, const GValue *value,
                                   GParamSpec *pspec)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

static void trg_prefs_dispose(GObject *object)
{
    G_OBJECT_CLASS(trg_prefs_parent_class)->dispose(object);
}

static void trg_prefs_create_defaults(TrgPrefs *p)
{
    p->defaultsObj = json_object_new();

    trg_prefs_add_default_string(p, TRG_PREFS_KEY_PROFILE_NAME, _(TRG_PROFILE_NAME_DEFAULT));
    trg_prefs_add_default_string(p, TRG_PREFS_KEY_RPC_URL_PATH, "/transmission/rpc");
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_PORT, 9091);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_UPDATE_INTERVAL, TRG_INTERVAL_DEFAULT);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_SESSION_UPDATE_INTERVAL,
                              TRG_SESSION_INTERVAL_DEFAULT);
    trg_prefs_add_default_int(p, TRG_PREFS_ACTIVEONLY_FULLSYNC_EVERY, 2);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_STATES_PANED_POS, 120);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_TIMEOUT, 40);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_RETRIES, 3);

    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_FILTER_DIRS);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_FILTER_TRACKERS);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_DIRECTORIES_FIRST);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_ADD_OPTIONS_DIALOG);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_SHOW_STATE_SELECTOR);
    // trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_SHOW_NOTEBOOK);
}

static GObject *trg_prefs_constructor(GType type, guint n_construct_properties,
                                      GObjectConstructParam *construct_params)
{
    GObject *object = G_OBJECT_CLASS(trg_prefs_parent_class)
                          ->constructor(type, n_construct_properties, construct_params);
    TrgPrefs *prefs = TRG_PREFS(object);

    trg_prefs_create_defaults(prefs);

    prefs->file = g_build_filename(g_get_user_config_dir(), g_get_application_name(),
                                   TRG_PREFS_FILENAME, NULL);

    return object;
}

static void trg_prefs_class_init(TrgPrefsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = trg_prefs_get_property;
    object_class->set_property = trg_prefs_set_property;
    object_class->dispose = trg_prefs_dispose;
    object_class->constructor = trg_prefs_constructor;

    signals[PREF_CHANGE] = g_signal_new(
        "pref-changed", G_TYPE_FROM_CLASS(object_class), G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0,
        NULL, NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);

    signals[PREF_PROFILE_CHANGE]
        = g_signal_new("pref-profile-changed", G_TYPE_FROM_CLASS(object_class),
                       G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, 0, NULL, NULL,
                       g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void trg_prefs_init(TrgPrefs *self)
{
}

TrgPrefs *trg_prefs_new(void)
{
    return g_object_new(TRG_TYPE_PREFS, NULL);
}

static JsonObject *trg_prefs_new_profile_object(void)
{
    return json_object_new();
}

void trg_prefs_add_default_int(TrgPrefs *p, const gchar *key, int value)
{
    json_object_set_int_member(p->defaultsObj, key, value);
}

void trg_prefs_add_default_string(TrgPrefs *p, const gchar *key, gchar *value)
{
    json_object_set_string_member(p->defaultsObj, key, value);
}

void trg_prefs_add_default_double(TrgPrefs *p, const gchar *key, double value)
{

    json_object_set_double_member(p->defaultsObj, key, value);
}

/* Not much point adding a default of FALSE, as that's the fallback */
void trg_prefs_add_default_bool_true(TrgPrefs *p, const gchar *key)
{
    json_object_set_boolean_member(p->defaultsObj, key, TRUE);
}

gint trg_prefs_get_profile_id(TrgPrefs *p)
{
    return (gint)json_object_get_int_member(p->userObj, TRG_PREFS_KEY_PROFILE_ID);
}

static JsonNode *trg_prefs_get_value_inner(JsonObject *obj, const gchar *key, int type, int flags)
{
    if (json_object_has_member(obj, key)) {
        if ((flags & TRG_PREFS_REPLACENODE))
            json_object_remove_member(obj, key);
        else
            return json_object_get_member(obj, key);
    }

    if ((flags & TRG_PREFS_NEWNODE) || (flags & TRG_PREFS_REPLACENODE)) {
        JsonNode *newNode = json_node_new(type);
        json_object_set_member(obj, key, newNode);
        return newNode;
    }

    return NULL;
}

JsonNode *trg_prefs_get_value(TrgPrefs *p, const gchar *key, int type, int flags)
{
    JsonNode *res;

    if (p->profile && (flags & TRG_PREFS_PROFILE)) {
        if ((res = trg_prefs_get_value_inner(p->profile, key, type, flags)))
            return res;
    } else if (p->connectionObj && (flags & TRG_PREFS_CONNECTION)) {
        if ((res = trg_prefs_get_value_inner(p->connectionObj, key, type, flags)))
            return res;
    } else {
        if ((res = trg_prefs_get_value_inner(p->userObj, key, type, flags)))
            return res;
    }

    if (p->defaultsObj && json_object_has_member(p->defaultsObj, key))
        return json_object_get_member(p->defaultsObj, key);

    return NULL;
}

void trg_prefs_set_connection(TrgPrefs *p, JsonObject *profile)
{
    g_clear_pointer(&p->connectionObj, json_object_unref);

    if (profile)
        json_object_ref(profile);

    p->connectionObj = profile;
}

gchar *trg_prefs_get_string(TrgPrefs *p, const gchar *key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags);

    if (node)
        return g_strdup(json_node_get_string(node));
    else
        return NULL;
}

JsonArray *trg_prefs_get_array(TrgPrefs *p, const gchar *key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_ARRAY, flags);

    if (node)
        return json_node_get_array(node);
    else
        return NULL;
}

gint64 trg_prefs_get_int(TrgPrefs *p, const gchar *key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags);

    if (node)
        return json_node_get_int(node);
    else
        return 0;
}

gdouble trg_prefs_get_double(TrgPrefs *p, const gchar *key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags);
    if (node)
        return json_node_get_double(node);
    else
        return 0.0;
}

gboolean trg_prefs_get_bool(TrgPrefs *p, const gchar *key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags);

    if (node)
        return json_node_get_boolean(node);
    else
        return FALSE;
}

void trg_prefs_set_int(TrgPrefs *p, const gchar *key, int value, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags | TRG_PREFS_NEWNODE);

    json_node_set_int(node, (gint64)value);
    trg_prefs_changed_emit_signal(p, key);
}

void trg_prefs_set_string(TrgPrefs *p, const gchar *key, const gchar *value, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags | TRG_PREFS_NEWNODE);
    json_node_set_string(node, value);
    trg_prefs_changed_emit_signal(p, key);
}

void trg_prefs_set_profile(TrgPrefs *p, JsonObject *profile)
{
    GList *profiles = json_array_get_elements(trg_prefs_get_profiles(p));
    gint i = 0;
    GList *li;

    p->profile = profile;

    for (li = profiles; li; li = g_list_next(li)) {
        if (json_node_get_object((JsonNode *)li->data) == profile) {
            trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, i, TRG_PREFS_GLOBAL);
            break;
        }
        i++;
    }

    g_list_free(profiles);

    trg_prefs_changed_emit_signal(p, NULL);
    trg_prefs_profile_change_emit_signal(p);
}

JsonObject *trg_prefs_new_profile(TrgPrefs *p)
{
    JsonArray *profiles = trg_prefs_get_profiles(p);
    JsonObject *newp = trg_prefs_new_profile_object();

    json_array_add_object_element(profiles, newp);

    return newp;
}

void trg_prefs_del_profile(TrgPrefs *p, JsonObject *profile)
{
    JsonArray *profiles = trg_prefs_get_profiles(p);
    GList *profilesList = json_array_get_elements(profiles);

    GList *li;
    JsonNode *node;
    int i = 0;

    for (li = profilesList; li; li = g_list_next(li)) {
        node = (JsonNode *)li->data;
        if (profile == json_node_get_object(node)) {
            json_array_remove_element(profiles, i);
            break;
        }
        i++;
    }

    g_list_free(profilesList);

    trg_prefs_profile_change_emit_signal(p);
}

JsonObject *trg_prefs_get_profile(TrgPrefs *p)
{
    return p->profile;
}

JsonObject *trg_prefs_get_connection(TrgPrefs *p)
{
    return p->connectionObj;
}

JsonArray *trg_prefs_get_profiles(TrgPrefs *p)
{
    return json_object_get_array_member(p->userObj, TRG_PREFS_KEY_PROFILES);
}

void trg_prefs_set_double(TrgPrefs *p, const gchar *key, gdouble value, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags | TRG_PREFS_NEWNODE);
    json_node_set_double(node, value);
    trg_prefs_changed_emit_signal(p, key);
}

void trg_prefs_set_bool(TrgPrefs *p, const gchar *key, gboolean value, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags | TRG_PREFS_NEWNODE);
    json_node_set_boolean(node, value);
    trg_prefs_changed_emit_signal(p, key);
}

gboolean trg_prefs_save(TrgPrefs *p)
{
    g_autoptr(JsonGenerator) gen = NULL;
    gchar *dirName;
    gboolean success = TRUE;
    gboolean isNew = TRUE;

    dirName = g_path_get_dirname(p->file);
    if (!g_file_test(dirName, G_FILE_TEST_IS_DIR)) {
        success = g_mkdir_with_parents(dirName, TRG_PREFS_DEFAULT_DIR_MODE) == 0;
    } else if (g_file_test(p->file, G_FILE_TEST_IS_REGULAR)) {
        isNew = FALSE;
    }
    g_free(dirName);

    if (!success) {
        g_warning("Problem creating parent directory (permissions?) for: %s", p->file);
        return success;
    }

    gen = trg_json_serializer(p->user, TRUE);

    if (g_file_test(p->file, G_FILE_TEST_IS_SYMLINK)) {
        gsize len;
        gchar *realFile;
        gchar *fileData;
        gchar *linkPath = realFile = g_file_read_link(p->file, NULL);
        if (!g_path_is_absolute(linkPath)) {
            dirName = g_path_get_dirname(p->file);
            realFile = g_build_filename(dirName, linkPath, NULL);
            g_free(dirName);
            g_free(linkPath);
        }
        fileData = json_generator_to_data(gen, &len);
        success = g_file_set_contents(realFile, fileData, len, NULL);
        g_free(realFile);
        g_free(fileData);
    } else {
        success = json_generator_to_file(gen, p->file, NULL);
    }

    if (!success)
        g_warning("Problem writing configuration file (permissions?) to: %s", p->file);
    else if (isNew)
        g_chmod(p->file, 384);

    return success;
}

JsonObject *trg_prefs_get_root(TrgPrefs *p)
{
    return p->userObj;
}

static void trg_prefs_empty_init(TrgPrefs *p)
{
    JsonArray *profiles = json_array_new();

    p->user = json_node_new(JSON_NODE_OBJECT);
    p->userObj = json_object_new();
    json_node_take_object(p->user, p->userObj);

    p->profile = trg_prefs_new_profile_object();

    json_array_add_object_element(profiles, p->profile);
    json_object_set_array_member(p->userObj, TRG_PREFS_KEY_PROFILES, profiles);

    json_object_set_int_member(p->userObj, TRG_PREFS_KEY_PROFILE_ID, 0);
}

void trg_prefs_load(TrgPrefs *p)
{
    JsonParser *parser = json_parser_new();
    JsonNode *root;
    guint n_profiles;
    JsonArray *profiles;

    gboolean parsed = json_parser_load_from_file(parser, p->file, NULL);

    if (!parsed) {
        trg_prefs_empty_init(p);
        g_object_unref(parser);
        return;
    }

    root = json_parser_get_root(parser);
    if (root) {
        p->user = json_node_copy(root);
        p->userObj = json_node_get_object(p->user);
    }

    g_object_unref(parser);

    if (!root) {
        trg_prefs_empty_init(p);
        return;
    }

    if (!json_object_has_member(p->userObj, TRG_PREFS_KEY_PROFILES)) {
        profiles = json_array_new();
        json_object_set_array_member(p->userObj, TRG_PREFS_KEY_PROFILES, profiles);
    } else {
        profiles = json_object_get_array_member(p->userObj, TRG_PREFS_KEY_PROFILES);
    }

    n_profiles = json_array_get_length(profiles);

    if (n_profiles < 1) {
        p->profile = trg_prefs_new_profile_object();
        json_array_add_object_element(profiles, p->profile);
        trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, 0, TRG_PREFS_GLOBAL);
    } else {
        // Note: profile IDs are strictly positive
        guint profile_id = (guint)trg_prefs_get_int(p, TRG_PREFS_KEY_PROFILE_ID, TRG_PREFS_GLOBAL);
        if (profile_id >= n_profiles)
            trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, profile_id = 0, TRG_PREFS_GLOBAL);

        p->profile = json_array_get_object_element(profiles, profile_id);
    }
}

guint trg_prefs_get_add_flags(TrgPrefs *p)
{
    guint flags = 0x00;

    if (trg_prefs_get_bool(p, TRG_PREFS_KEY_START_PAUSED, TRG_PREFS_GLOBAL))
        flags |= TORRENT_ADD_FLAG_PAUSED;

    if (trg_prefs_get_bool(p, TRG_PREFS_KEY_DELETE_LOCAL_TORRENT, TRG_PREFS_GLOBAL))
        flags |= TORRENT_ADD_FLAG_DELETE;

    return flags;
}
