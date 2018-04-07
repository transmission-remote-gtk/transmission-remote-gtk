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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#ifdef HAVE_LIBSECRET
#include <libsecret/secret.h>
#endif

#include "util.h"
#include "torrent.h"
#include "trg-client.h"
#include "trg-prefs.h"
#ifdef HAVE_LIBSECRET
#include "trg-secret-schema.h"
#endif

/* I replaced GConf with this custom configuration backend for a few reasons.
 * 1) Better windows support. No dependency on DBus.
 * 2) We're including a JSON parser/writer anyway.
 * 3) The GConf API is pretty verbose sometimes, working with lists is awkward.
 * 4) Easily switch between profiles, and scope config to either profiles or global.
 * 5) Transparently use configurables from the current connection, not current profile.
 */

G_DEFINE_TYPE(TrgPrefs, trg_prefs, G_TYPE_OBJECT)

struct _TrgPrefsPrivate {
    JsonObject *defaultsObj;
    JsonNode *user;
    JsonObject *userObj;
    JsonObject *connectionObj;
    JsonObject *profile;
    gchar *file;
#ifdef HAVE_LIBSECRET
    GHashTable *passwords;
    gboolean secret_error;
#endif
};

enum {
    PREF_CHANGE,
    PREF_PROFILE_CHANGE,
#ifdef HAVE_LIBSECRET
    PREF_LOADED,
    PREF_SECRET_ERROR,
#endif
    PREFS_SIGNAL_COUNT
};

static guint signals[PREFS_SIGNAL_COUNT] = { 0 };

void trg_prefs_profile_change_emit_signal(TrgPrefs * p)
{
    g_signal_emit(p, signals[PREF_PROFILE_CHANGE], 0);
}

void trg_prefs_changed_emit_signal(TrgPrefs * p, const gchar * key)
{
    g_signal_emit(p, signals[PREF_CHANGE], 0, key);
}

#ifdef HAVE_LIBSECRET
void trg_prefs_loaded_emit_signal(TrgPrefs * p)
{
    g_signal_emit(p, signals[PREF_LOADED], 0);
}

void trg_prefs_secret_error_emit_signal(TrgPrefs * p, const gchar *message)
{
    g_signal_emit(p, signals[PREF_SECRET_ERROR], 0, message);
}
#endif

static void
trg_prefs_get_property(GObject * object, guint property_id,
                       GValue * value, GParamSpec * pspec)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

static void
trg_prefs_set_property(GObject * object, guint property_id,
                       const GValue * value, GParamSpec * pspec)
{
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

static void trg_prefs_dispose(GObject * object)
{
    G_OBJECT_CLASS(trg_prefs_parent_class)->dispose(object);
}

static void trg_prefs_create_defaults(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
    priv->defaultsObj = json_object_new();

    trg_prefs_add_default_string(p, TRG_PREFS_KEY_PROFILE_NAME,
                                 _(TRG_PROFILE_NAME_DEFAULT));
    trg_prefs_add_default_string(p, TRG_PREFS_KEY_RPC_URL_PATH, "/transmission/rpc");
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_PORT, 9091);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_UPDATE_INTERVAL,
                              TRG_INTERVAL_DEFAULT);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_SESSION_UPDATE_INTERVAL,
                              TRG_SESSION_INTERVAL_DEFAULT);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_MINUPDATE_INTERVAL,
                              TRG_INTERVAL_DEFAULT);
    trg_prefs_add_default_int(p, TRG_PREFS_ACTIVEONLY_FULLSYNC_EVERY, 2);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_STATES_PANED_POS, 120);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_TIMEOUT, 40);
    trg_prefs_add_default_int(p, TRG_PREFS_KEY_RETRIES, 3);

    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_FILTER_DIRS);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_FILTER_TRACKERS);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_DIRECTORIES_FIRST);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_SHOW_GRAPH);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_ADD_OPTIONS_DIALOG);
    trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_SHOW_STATE_SELECTOR);
    //trg_prefs_add_default_bool_true(p, TRG_PREFS_KEY_SHOW_NOTEBOOK);
}

static GObject *trg_prefs_constructor(GType type,
                                      guint n_construct_properties,
                                      GObjectConstructParam *
                                      construct_params)
{
    GObject *object = G_OBJECT_CLASS
        (trg_prefs_parent_class)->constructor(type, n_construct_properties,
                                              construct_params);
    TrgPrefs *prefs = TRG_PREFS(object);
    TrgPrefsPrivate *priv = prefs->priv;

    trg_prefs_create_defaults(prefs);

    priv->file = g_build_filename(g_get_user_config_dir(),
                                  g_get_application_name(),
                                  TRG_PREFS_FILENAME, NULL);

    return object;
}

static void trg_prefs_class_init(TrgPrefsClass * klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(TrgPrefsPrivate));

    object_class->get_property = trg_prefs_get_property;
    object_class->set_property = trg_prefs_set_property;
    object_class->dispose = trg_prefs_dispose;
    object_class->constructor = trg_prefs_constructor;

    signals[PREF_CHANGE] =
        g_signal_new("pref-changed",
                     G_TYPE_FROM_CLASS(object_class),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(TrgPrefsClass,
                                     pref_changed), NULL,
                     NULL, g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);

    signals[PREF_PROFILE_CHANGE] =
        g_signal_new("pref-profile-changed",
                     G_TYPE_FROM_CLASS(object_class),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(TrgPrefsClass,
                                     pref_changed), NULL,
                     NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

#ifdef HAVE_LIBSECRET
    signals[PREF_LOADED] =
        g_signal_new("pref-loaded",
                     G_TYPE_FROM_CLASS(object_class),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(TrgPrefsClass,
                                     pref_loaded), NULL,
                     NULL,g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[PREF_SECRET_ERROR] =
        g_signal_new("pref-secret-error",
                     G_TYPE_FROM_CLASS(object_class),
                     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                     G_STRUCT_OFFSET(TrgPrefsClass,
                                     pref_secret_error), NULL,
                     NULL,g_cclosure_marshal_VOID__POINTER,
                     G_TYPE_NONE, 1, G_TYPE_POINTER);
#endif
}

static void trg_prefs_init(TrgPrefs * self)
{
    self->priv =
        G_TYPE_INSTANCE_GET_PRIVATE(self, TRG_TYPE_PREFS, TrgPrefsPrivate);
}

TrgPrefs *trg_prefs_new(void)
{
    return g_object_new(TRG_TYPE_PREFS, NULL);
}

#ifdef HAVE_LIBSECRET
static void
trg_prefs_profile_add_uuid_node(JsonObject *profile)
{
    JsonNode *node = json_node_new(JSON_NODE_VALUE);
    gchar *uuid = g_uuid_string_random();
    json_node_set_string (node, uuid);
    g_free(uuid);
    json_object_add_member(profile, TRG_PREFS_KEY_PROFILE_UUID, node);
}
#endif

static JsonObject *trg_prefs_new_profile_object(void)
{
#ifdef HAVE_LIBSECRET
    /* Add uuid so we can key off it for secrets */
    JsonObject *profile = json_object_new();
    trg_prefs_profile_add_uuid_node(profile);
    return profile;
#else
    return json_object_new();
#endif
}

void trg_prefs_add_default_int(TrgPrefs * p, const gchar * key, int value)
{
    TrgPrefsPrivate *priv = p->priv;

    json_object_set_int_member(priv->defaultsObj, key, value);
}

void
trg_prefs_add_default_string(TrgPrefs * p, const gchar * key,
                             gchar * value)
{
    TrgPrefsPrivate *priv = p->priv;

    json_object_set_string_member(priv->defaultsObj, key, value);
}

void
trg_prefs_add_default_double(TrgPrefs * p, const gchar * key, double value)
{
    TrgPrefsPrivate *priv = p->priv;

    json_object_set_double_member(priv->defaultsObj, key, value);
}

/* Not much point adding a default of FALSE, as that's the fallback */
void trg_prefs_add_default_bool_true(TrgPrefs * p, const gchar * key)
{
    TrgPrefsPrivate *priv = p->priv;
    json_object_set_boolean_member(priv->defaultsObj, key, TRUE);
}

gint trg_prefs_get_profile_id(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
    return (gint) json_object_get_int_member(priv->userObj,
                                             TRG_PREFS_KEY_PROFILE_ID);
}

static JsonNode *trg_prefs_get_value_inner(JsonObject * obj,
                                           const gchar * key, int type,
                                           int flags)
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

JsonNode *trg_prefs_get_value(TrgPrefs * p, const gchar * key, int type,
                              int flags)
{
    TrgPrefsPrivate *priv = p->priv;
    JsonNode *res;

    if (priv->profile && (flags & TRG_PREFS_PROFILE)) {
        if ((res =
             trg_prefs_get_value_inner(priv->profile, key, type, flags)))
            return res;
    } else if (priv->connectionObj && (flags & TRG_PREFS_CONNECTION)) {
        if ((res =
             trg_prefs_get_value_inner(priv->connectionObj, key, type,
                                       flags)))
            return res;
    } else {
        if ((res =
             trg_prefs_get_value_inner(priv->userObj, key, type, flags)))
            return res;
    }

    if (priv->defaultsObj
        && json_object_has_member(priv->defaultsObj, key))
        return json_object_get_member(priv->defaultsObj, key);

    return NULL;
}

void trg_prefs_set_connection(TrgPrefs * p, JsonObject * profile)
{
    TrgPrefsPrivate *priv = p->priv;

    if (priv->connectionObj)
        json_object_unref(priv->connectionObj);

    if (profile)
        json_object_ref(profile);

    priv->connectionObj = profile;
}

gchar *trg_prefs_get_string(TrgPrefs * p, const gchar * key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags);

    if (node)
        return g_strdup(json_node_get_string(node));
    else
        return NULL;
}

JsonArray *trg_prefs_get_array(TrgPrefs * p, const gchar * key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_ARRAY, flags);

    if (node)
        return json_node_get_array(node);
    else
        return NULL;
}

gint64 trg_prefs_get_int(TrgPrefs * p, const gchar * key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags);

    if (node)
        return json_node_get_int(node);
    else
        return 0;
}

gdouble trg_prefs_get_double(TrgPrefs * p, const gchar * key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags);
    if (node)
        return json_node_get_double(node);
    else
        return 0.0;
}

gboolean trg_prefs_get_bool(TrgPrefs * p, const gchar * key, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE, flags);

    if (node)
        return json_node_get_boolean(node);
    else
        return FALSE;
}

void
trg_prefs_set_int(TrgPrefs * p, const gchar * key, int value, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE,
                                         flags | TRG_PREFS_NEWNODE);

    json_node_set_int(node, (gint64) value);
    trg_prefs_changed_emit_signal(p, key);
}

void
trg_prefs_set_string(TrgPrefs * p, const gchar * key,
                     const gchar * value, int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE,
                                         flags | TRG_PREFS_NEWNODE);
    json_node_set_string(node, value);
    trg_prefs_changed_emit_signal(p, key);
}

void trg_prefs_set_profile(TrgPrefs * p, JsonObject * profile)
{
    TrgPrefsPrivate *priv = p->priv;
    GList *profiles = json_array_get_elements(trg_prefs_get_profiles(p));
    gint i = 0;
    GList *li;

    priv->profile = profile;

    for (li = profiles; li; li = g_list_next(li)) {
        if (json_node_get_object((JsonNode *) li->data) == profile) {
            trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, i,
                              TRG_PREFS_GLOBAL);
            break;
        }
        i++;
    }

    g_list_free(profiles);

    trg_prefs_changed_emit_signal(p, NULL);
    trg_prefs_profile_change_emit_signal(p);
}

JsonObject *trg_prefs_new_profile(TrgPrefs * p)
{
    JsonArray *profiles = trg_prefs_get_profiles(p);
    JsonObject *newp = trg_prefs_new_profile_object();

    json_array_add_object_element(profiles, newp);

    return newp;
}

void trg_prefs_del_profile(TrgPrefs * p, JsonObject * profile)
{

#ifdef HAVE_LIBSECRET
    /* delete secret along with profile */
    const gchar *uuid = json_object_get_string_member(profile, TRG_PREFS_KEY_PROFILE_UUID);
    secret_password_clear (TRG_SECRET_SCHEMA, NULL, NULL, NULL,
                           TRG_PREFS_KEY_PROFILE_UUID, uuid,
                           NULL);
#endif

    JsonArray *profiles = trg_prefs_get_profiles(p);
    GList *profilesList = json_array_get_elements(profiles);

    GList *li;
    JsonNode *node;
    int i = 0;

    for (li = profilesList; li; li = g_list_next(li)) {
        node = (JsonNode *) li->data;
        if (profile == json_node_get_object(node)) {
            json_array_remove_element(profiles, i);
            break;
        }
        i++;
    }

    g_list_free(profilesList);

    trg_prefs_profile_change_emit_signal(p);
}

JsonObject *trg_prefs_get_profile(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
    return priv->profile;
}

JsonObject *trg_prefs_get_connection(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
    return priv->connectionObj;
}

JsonArray *trg_prefs_get_profiles(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
    return json_object_get_array_member(priv->userObj,
                                        TRG_PREFS_KEY_PROFILES);
}

JsonArray *trg_prefs_get_rss(TrgPrefs *p) {
	TrgPrefsPrivate *priv = p->priv;
	return json_object_get_array_member(priv->userObj,
            TRG_PREFS_KEY_RSS);
}

void
trg_prefs_set_double(TrgPrefs * p, const gchar * key, gdouble value,
                     int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE,
                                         flags | TRG_PREFS_NEWNODE);
    json_node_set_double(node, value);
    trg_prefs_changed_emit_signal(p, key);
}

void
trg_prefs_set_bool(TrgPrefs * p, const gchar * key, gboolean value,
                   int flags)
{
    JsonNode *node = trg_prefs_get_value(p, key, JSON_NODE_VALUE,
                                         flags | TRG_PREFS_NEWNODE);
    json_node_set_boolean(node, value);
    trg_prefs_changed_emit_signal(p, key);
}

#ifdef HAVE_LIBSECRET
static void
trg_prefs_save_get_passwords_cb(JsonArray *array, guint index_, JsonNode *element_node, gpointer userdata)
{
    TrgPrefs *p = (TrgPrefs*)userdata;

    JsonObject *profile = json_node_get_object(element_node);
    const gchar *uuid = json_object_get_string_member(profile, TRG_PREFS_KEY_PROFILE_UUID);
    const gchar *password = json_object_get_string_member(profile, TRG_PREFS_KEY_PASSWORD);
    g_hash_table_insert(p->priv->passwords, g_strdup(uuid), g_strdup(password));
    json_object_remove_member(profile, TRG_PREFS_KEY_PASSWORD);
}

static void
trg_prefs_save_restore_and_save_passwords_cb_cb(GObject *source, GAsyncResult *result, gpointer userdata)
{
    TrgPrefs *p = (TrgPrefs*)userdata;

    /* Only emit secret-error once no matter how many errors */
    if(!secret_password_store_finish (result, NULL) && !p->priv->secret_error) {
        p->priv->secret_error = TRUE;
        trg_prefs_secret_error_emit_signal(p, _("Error saving passwords to keychain"));
    }
}

static void
trg_prefs_save_restore_and_save_passwords_cb(JsonArray *array, guint index_, JsonNode *element_node, gpointer userdata)
{
    TrgPrefs *p = (TrgPrefs*)userdata;

    JsonObject *profile = json_node_get_object(element_node);
    const gchar *uuid = json_object_get_string_member(profile, TRG_PREFS_KEY_PROFILE_UUID);
    const gchar *password = g_hash_table_lookup(p->priv->passwords, uuid);

    json_object_set_string_member (profile, TRG_PREFS_KEY_PASSWORD, password);

    secret_password_store(TRG_SECRET_SCHEMA, NULL, PACKAGE_NAME, password, NULL, 
                          trg_prefs_save_restore_and_save_passwords_cb_cb, p,
                          TRG_PREFS_KEY_PROFILE_UUID, uuid,
                          NULL);
}
#endif /* HAVE_LIBSECRET */

gboolean trg_prefs_save(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
    JsonGenerator *gen = json_generator_new();
    gchar *dirName;
    gboolean success = TRUE;
    gboolean isNew = TRUE;

    dirName = g_path_get_dirname(priv->file);
    if (!g_file_test(dirName, G_FILE_TEST_IS_DIR)) {
        success = g_mkdir_with_parents(dirName, TRG_PREFS_DEFAULT_DIR_MODE)
            == 0;
    } else if (g_file_test(priv->file, G_FILE_TEST_IS_REGULAR)) {
        isNew = FALSE;
    }
    g_free(dirName);

    if (!success) {
        g_error
            ("Problem creating parent directory (permissions?) for: %s\n",
             priv->file);
        return success;
    }

    g_object_set(G_OBJECT(gen), "pretty", TRUE, NULL);

#ifdef HAVE_LIBSECRET
     /* Save passwords from json to hash table keyed on UUID, and */
     /* remove json password nodes so they are not saved to disk  */
    p->priv->passwords = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    JsonArray *profiles = json_object_get_array_member(priv->userObj, TRG_PREFS_KEY_PROFILES); 
    json_array_foreach_element(profiles, trg_prefs_save_get_passwords_cb, p); 
#endif

    json_generator_set_root(gen, priv->user);
    success = json_generator_to_file(gen, priv->file, NULL);

    if (!success)
        g_error("Problem writing configuration file (permissions?) to: %s",
                priv->file);
    else if (isNew)
        g_chmod(priv->file, 384);

    g_object_unref(gen);

#ifdef HAVE_LIBSECRET
    /* Recreate password nodes and save passwords to key chain */
    p->priv->secret_error = FALSE;
    json_array_foreach_element(profiles, trg_prefs_save_restore_and_save_passwords_cb, p);
    g_hash_table_destroy(p->priv->passwords);
    p->priv->passwords = NULL;
#endif

    return success;
}

JsonObject *trg_prefs_get_root(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
    return priv->userObj;
}

static void trg_prefs_empty_init(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
    JsonArray *profiles = json_array_new();

    priv->user = json_node_new(JSON_NODE_OBJECT);
    priv->userObj = json_object_new();
    json_node_take_object(priv->user, priv->userObj);

    priv->profile = trg_prefs_new_profile_object();

    json_array_add_object_element(profiles, priv->profile);
    json_object_set_array_member(priv->userObj, TRG_PREFS_KEY_PROFILES,
                                 profiles);

    json_object_set_int_member(priv->userObj, TRG_PREFS_KEY_PROFILE_ID, 0);
}

#ifdef HAVE_LIBSECRET
static void
trg_prefs_secrets_list_free_func(gpointer data)
{
    if(data) {
        SecretItem *item = (SecretItem*)data;
        g_object_unref(item);
    }
}

static void
trg_prefs_secret_json_add_empty_password_node(JsonArray *array, guint index_, JsonNode *element_node, gpointer userdata)
{
    JsonObject *profile = json_node_get_object(element_node);

    JsonNode *password_node = json_node_new(JSON_NODE_VALUE);
    json_node_set_string(password_node, "");
    json_object_set_member(profile, TRG_PREFS_KEY_PASSWORD, password_node);

    /* Also add uuid if profile does not have one to potentially help upgrades */
    if(!json_object_has_member (profile, TRG_PREFS_KEY_PROFILE_UUID))
        trg_prefs_profile_add_uuid_node(profile);
}

static void
trg_prefs_load_secrets_set_password_node_cb(JsonArray *array, guint index_, JsonNode *element_node, gpointer userdata)
{
    GList *li, *secrets = (GList*)userdata;

    JsonObject *profile_object = json_node_get_object(element_node);
    const gchar *json_uuid = json_object_get_string_member(profile_object,
                                                           TRG_PREFS_KEY_PROFILE_UUID);

    for (li = secrets; li; li = g_list_next(li)) {
        SecretItem *item = (SecretItem*)li->data;

        GHashTable *attributes = secret_item_get_attributes(item);
        const gchar *secret_uuid  = g_hash_table_lookup(attributes, TRG_PREFS_KEY_PROFILE_UUID);
        if(g_strcmp0(json_uuid, secret_uuid) == 0) {
            SecretValue *secret_value = secret_item_get_secret(item);
            const gchar *password = secret_value_get_text(secret_value);

            JsonObject *profile_object = json_node_get_object(element_node);
            JsonNode *password_node = json_node_new(JSON_NODE_VALUE);
            json_node_set_string(password_node, password ? password : "");
            json_object_set_member(profile_object, TRG_PREFS_KEY_PASSWORD, password_node);

            secret_value_unref(secret_value);
            g_hash_table_unref(attributes);
        }
        g_hash_table_unref(attributes);
    }
}

static void
trg_prefs_load_secrets_cb(GObject *source, GAsyncResult *result, gpointer userdata)
{
    TrgPrefs *p = (TrgPrefs*)userdata;
    TrgPrefsPrivate *priv = p->priv;
    GError *error = NULL;

    GList *secrets = secret_service_search_finish(NULL, result, &error);

    JsonArray *profiles = json_object_get_array_member(priv->userObj, TRG_PREFS_KEY_PROFILES);
    guint n_profiles = json_array_get_length(profiles);

    if(secrets) {
        json_array_foreach_element(profiles, trg_prefs_load_secrets_set_password_node_cb, secrets);
        trg_prefs_loaded_emit_signal(p);
    } else if(n_profiles > 0 && !error) {
        /* Errors do not seem to be returned from libsecret  ¯\_(ツ)_/¯ */
        /* so determine that passwords have not been returned ourselves */
        trg_prefs_secret_error_emit_signal(p,  _("Error loading passwords from keychain"));
    }

    if(error) {
        trg_prefs_secret_error_emit_signal(p,  _("Error loading passwords from keychain"));
        g_error_free(error);
    }

    g_list_free_full(secrets, trg_prefs_secrets_list_free_func);
}
#endif /* HAVE_LIBSECRET */

void trg_prefs_load(TrgPrefs * p)
{
    TrgPrefsPrivate *priv = p->priv;
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
        trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, 0,
                          TRG_PREFS_GLOBAL);
    } else {
        gint profile_id = trg_prefs_get_int(p, TRG_PREFS_KEY_PROFILE_ID,
                                            TRG_PREFS_GLOBAL);
        if (profile_id >= n_profiles)
            trg_prefs_set_int(p, TRG_PREFS_KEY_PROFILE_ID, profile_id = 0,
                              TRG_PREFS_GLOBAL);

        priv->profile =
            json_array_get_object_element(profiles, profile_id);
    }
#ifdef HAVE_LIBSECRET
    /* Add empty password nodes so we don't crash on failure to get passwords from key chain */
    json_array_foreach_element(profiles, trg_prefs_secret_json_add_empty_password_node, NULL);

    /* Load all the passwords from key chain in one go */
    GHashTable *h = g_hash_table_new(NULL, NULL);
    secret_service_search(NULL, TRG_SECRET_SCHEMA, h,
                          SECRET_SEARCH_ALL | SECRET_SEARCH_LOAD_SECRETS,
                          NULL,
                          trg_prefs_load_secrets_cb, p);
    g_hash_table_destroy(h);
#endif
}

guint trg_prefs_get_add_flags(TrgPrefs * p)
{
    guint flags = 0x00;

    if (trg_prefs_get_bool
        (p, TRG_PREFS_KEY_START_PAUSED, TRG_PREFS_GLOBAL))
        flags |= TORRENT_ADD_FLAG_PAUSED;

    if (trg_prefs_get_bool
        (p, TRG_PREFS_KEY_DELETE_LOCAL_TORRENT, TRG_PREFS_GLOBAL))
        flags |= TORRENT_ADD_FLAG_DELETE;

    return flags;
}
