/*
 * transmission-remote-gtk - Transmission RPC client for GTK
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
#include <stdio.h>
#include <json-glib/json-glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "util.h"
#include "torrent.h"
#include "trg-client.h"
#include "trg-prefs.h"

G_DEFINE_TYPE (TrgPrefs, trg_prefs, G_TYPE_OBJECT)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TRG_TYPE_PREFS, TrgPrefsPrivate))

typedef struct _TrgPrefsPrivate TrgPrefsPrivate;

struct _TrgPrefsPrivate {
    JsonObject *defaultsObj;
    JsonNode *user;
    JsonObject *userObj;
    JsonObject *profile;
    gchar *file;
};

static void trg_prefs_get_property(GObject *object, guint property_id,
        GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void trg_prefs_set_property(GObject *object, guint property_id,
        const GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void trg_prefs_dispose(GObject *object) {
    G_OBJECT_CLASS (trg_prefs_parent_class)->dispose(object);
}

static void trg_prefs_create_defaults(TrgPrefs *p) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    priv->defaultsObj = json_object_new();

    trg_prefs_add_default_string(p, TRG_PREFS_KEY_PROFILE_NAME,
            _(TRG_PROFILE_NAME_DEFAULT));
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_PORT, 9091);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_UPDATE_INTERVAL, 3);

    trg_prefs_add_default_bool(p, TRG_PREFS_KEY_FILTER_DIRS, TRUE);
    trg_prefs_add_default_bool(p, TRG_PREFS_KEY_FILTER_TRACKERS, TRUE);
    trg_prefs_add_default_bool(p, TRG_PREFS_KEY_AUTO_CONNECT, FALSE);
    trg_prefs_add_default_bool(p, TRG_PREFS_KEY_SHOW_GRAPH, TRUE);
    trg_prefs_add_default_bool(p, TRG_PREFS_KEY_SHOW_GRAPH, TRUE);
    trg_prefs_add_default_bool(p, TRG_PREFS_KEY_ADD_OPTIONS_DIALOG, TRUE);
}

static GObject *trg_prefs_constructor(GType type, guint n_construct_properties,
        GObjectConstructParam * construct_params) {
    GObject *object;
    TrgPrefsPrivate *priv;

    object = G_OBJECT_CLASS
    (trg_prefs_parent_class)->constructor(type, n_construct_properties,
            construct_params);
    priv = GET_PRIVATE(object);

    trg_prefs_create_defaults(TRG_PREFS(object));

    priv->file = g_build_filename(g_get_user_config_dir(),
            g_get_application_name(), TRG_PREFS_FILENAME, NULL);

    return object;
}

static void trg_prefs_class_init(TrgPrefsClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private(klass, sizeof(TrgPrefsPrivate));

    object_class->get_property = trg_prefs_get_property;
    object_class->set_property = trg_prefs_set_property;
    object_class->dispose = trg_prefs_dispose;
    object_class->constructor = trg_prefs_constructor;
}

static void trg_prefs_init(TrgPrefs *self) {
}

TrgPrefs*
trg_prefs_new(void) {
    return g_object_new(TRG_TYPE_PREFS, NULL);
}

static JsonObject *trg_prefs_new_profile_object() {
    JsonObject *obj = json_object_new();
    return obj;
}

void trg_prefs_add_default_int(TrgPrefs *p, gchar *key, int value) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    json_object_set_int_member(priv->defaultsObj, key, value);
}

void trg_prefs_add_default_string(TrgPrefs *p, gchar *key, gchar *value) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    json_object_set_string_member(priv->defaultsObj, key, value);
}

void trg_prefs_add_default_double(TrgPrefs *p, gchar *key, double value) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    json_object_set_double_member(priv->defaultsObj, key, value);
}

void trg_prefs_add_default_bool(TrgPrefs *p, gchar *key, gboolean value) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    json_object_set_boolean_member(priv->defaultsObj, key, value);
}

gint trg_prefs_get_profile_id(TrgPrefs *p) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    return (gint) json_object_get_int_member(priv->userObj,
            TRG_PREFS_KEY_PROFILE_ID);
}

JsonNode *trg_prefs_get_value(TrgPrefs *p, gchar *key, int flags) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);

    if ((flags & TRG_PREFS_PROFILE)) {
        JsonObject *profile = trg_prefs_get_profile(p);
        if (json_object_has_member(profile, key)) {
            return json_object_get_member(profile, key);
        } else if ((flags & TRG_PREFS_NEWNODE)) {
            JsonNode *newNode = json_node_new(JSON_NODE_VALUE);
            json_object_set_member(profile, key, newNode);
            return newNode;
        }
    } else {
        if (json_object_has_member(priv->userObj, key)) {
            return json_object_get_member(priv->userObj, key);
        } else if ((flags & TRG_PREFS_NEWNODE)) {
            JsonNode *newNode = json_node_new(JSON_NODE_VALUE);
            json_object_set_member(priv->userObj, key, newNode);
            return newNode;
        }
    }

    if (priv->defaultsObj && json_object_has_member(priv->defaultsObj, key)) {
        return json_object_get_member(priv->defaultsObj, key);
    }

    return NULL;
}

gchar *trg_prefs_get_string(TrgPrefs *p, gchar *key, int flags) {
    JsonNode *node = trg_prefs_get_value(p, key, flags);
    if (node)
        return g_strdup(json_node_get_string(node));
    else
        return NULL;
}

int trg_prefs_get_int(TrgPrefs *p, gchar *key, int flags) {
    JsonNode *node = trg_prefs_get_value(p, key, flags);
    if (node)
        return json_node_get_int(node);
    else
        return 0;
}

gdouble trg_prefs_get_double(TrgPrefs *p, gchar *key, int flags) {
    JsonNode *node = trg_prefs_get_value(p, key, flags);
    if (node)
        return json_node_get_double(node);
    else
        return 0.0;
}

gboolean trg_prefs_get_bool(TrgPrefs *p, gchar *key, int flags) {
    JsonNode *node = trg_prefs_get_value(p, key, flags);
    if (node)
        return json_node_get_boolean(node);
    else
        return FALSE;
}

void trg_prefs_set_int(TrgPrefs *p, gchar *key, int value, int flags) {
    JsonNode *node = trg_prefs_get_value(p, key, flags | TRG_PREFS_NEWNODE);
    json_node_set_int(node, (gint64) value);
}

void trg_prefs_set_string(TrgPrefs *p, gchar *key, const gchar *value,
        int flags) {
    JsonNode *node = trg_prefs_get_value(p, key, flags | TRG_PREFS_NEWNODE);
    json_node_set_string(node, value);
}

void trg_prefs_set_profile(TrgPrefs *p, JsonObject *profile) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    GList *profiles = json_array_get_elements(trg_prefs_get_profiles(p));
    priv->profile = profile;

    GList *li;
    gint i = 0;

    for (li = profiles; li; li = g_list_next(li)) {
        if (json_node_get_object((JsonNode*) li->data) == profile) {
            trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, i, TRG_PREFS_GLOBAL);
            break;
        }
        i++;
    }

    g_list_free(profiles);
}

JsonObject *trg_prefs_new_profile(TrgPrefs *p) {
    JsonArray *profiles = trg_prefs_get_profiles(p);
    JsonObject *newp = trg_prefs_new_profile_object();
    json_array_add_object_element(profiles, newp);
    return newp;
}

void trg_prefs_del_profile(TrgPrefs *p, JsonObject *profile) {
    JsonArray *profiles = trg_prefs_get_profiles(p);
    GList *profilesList = json_array_get_elements(profiles);

    GList *li;
    JsonNode *node;
    int i = 0;

    for (li = profilesList; li; li = g_list_next(li)) {
        node = (JsonNode*) li->data;
        if (profile == (gpointer) json_node_get_object(node)) {
            json_array_remove_element(profiles, i);
            break;
        }
        i++;
    }

    g_list_free(profilesList);
}

JsonObject* trg_prefs_get_profile(TrgPrefs *p) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    return priv->profile;
}

JsonArray* trg_prefs_get_profiles(TrgPrefs *p) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    return json_object_get_array_member(priv->userObj, TRG_PREFS_KEY_PROFILES);
}

void trg_prefs_set_double(TrgPrefs *p, gchar *key, gdouble value, int flags) {
    JsonNode *node = trg_prefs_get_value(p, key, flags | TRG_PREFS_NEWNODE);
    json_node_set_double(node, value);
}

void trg_prefs_set_bool(TrgPrefs *p, gchar *key, gboolean value, int flags) {
    JsonNode *node = trg_prefs_get_value(p, key, flags | TRG_PREFS_NEWNODE);
    json_node_set_boolean(node, value);
}

gboolean trg_prefs_save(TrgPrefs *p) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    JsonGenerator *gen = json_generator_new();
    gchar *dirName;
    gboolean success = TRUE;

    dirName = g_path_get_dirname(priv->file);
    if (!g_file_test(dirName, G_FILE_TEST_IS_DIR))
        success = g_mkdir_with_parents(dirName, TRG_PREFS_DEFAULT_DIR_MODE)
                == 0;
    g_free(dirName);

    if (!success) {
        g_error("Problem creating parent directory (permissions?) for: %s\n", priv->file);
        return success;
    }

    g_object_set(G_OBJECT(gen), "pretty", TRUE, NULL);
    json_generator_set_root(gen, priv->user);

    success = json_generator_to_file(gen, priv->file, NULL);
    if (!success)
        g_error("Problem writing configuration file (permissions?) to: %s", priv->file);

    g_object_unref(gen);

    return success;
}

JsonObject *trg_prefs_get_root(TrgPrefs *p) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    return priv->userObj;
}

void trg_prefs_empty_init(TrgPrefs *p) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    priv->user = json_node_new(JSON_NODE_OBJECT);
    priv->userObj = json_object_new();
    json_node_take_object(priv->user, priv->userObj);

    priv->profile = trg_prefs_new_profile_object();

    JsonArray *profiles = json_array_new();
    json_array_add_object_element(profiles, priv->profile);
    json_object_set_array_member(priv->userObj, TRG_PREFS_KEY_PROFILES,
            profiles);

    json_object_set_int_member(priv->userObj, TRG_PREFS_KEY_PROFILE_ID, 0);
}

void trg_prefs_load(TrgPrefs *p) {
    TrgPrefsPrivate *priv = GET_PRIVATE(p);
    JsonParser *parser = json_parser_new();
    JsonNode *root;
    guint n_profiles;
    JsonArray *profiles;

    gboolean parsed = json_parser_load_from_file(parser, priv->file, NULL);

    if (!parsed) {
        trg_prefs_empty_init(p);
        g_object_unref(parser);
        return;
    }

    root = json_parser_get_root(parser);
    if (root) {
        priv->user = json_node_copy(root);
        priv->userObj = json_node_get_object(priv->user);
    }

    g_object_unref(parser);

    if (!root) {
        trg_prefs_empty_init(p);
        return;
    }

    if (!json_object_has_member(priv->userObj, TRG_PREFS_KEY_PROFILES)) {
        profiles = json_array_new();
        json_object_set_array_member(priv->userObj, TRG_PREFS_KEY_PROFILES,
                profiles);
    } else {
        profiles = json_object_get_array_member(priv->userObj,
                TRG_PREFS_KEY_PROFILES);
    }

    n_profiles = json_array_get_length(profiles);

    if (n_profiles < 1) {
        priv->profile = trg_prefs_new_profile_object();
        json_array_add_object_element(profiles, priv->profile);
        trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, 0, TRG_PREFS_GLOBAL);
    } else {
        gint profile_id = trg_prefs_get_int(p, TRG_PREFS_KEY_PROFILE_ID,
                TRG_PREFS_GLOBAL);
        if (profile_id >= n_profiles)
            trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, profile_id = 0,
                    TRG_PREFS_GLOBAL);

        priv->profile = json_array_get_object_element(profiles, profile_id);
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
