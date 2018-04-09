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

#include <libsecret/secret.h>

#include "trg-secret.h"
#include "trg-prefs.h"

/*
 * Suppress: "warning: missing initializer for field ‘reserved’ of ‘SecretSchema 
 * {aka const struct <anonymous>}’ [-Wmissing-field-initializers]" which is harmless
 */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

typedef struct {
	GAsyncResult *result;
	GMainContext *context;
	GMainLoop *loop;
} TrgSecretSync;

const SecretSchema *
trg_secret_get_schema(void)
{
    static const SecretSchema the_schema = {
        "io.github.TransmissionRemoteGtk.password", SECRET_SCHEMA_NONE,
        {
            {  TRG_PREFS_KEY_PROFILE_UUID, SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  NULL, 0 },
        }
    };
    return &the_schema;
}

static TrgSecretSync *
trg_secret_sync_new(void)
{
    TrgSecretSync *sync;

    sync = g_new0(TrgSecretSync, 1);

    sync->context = g_main_context_new();
    sync->loop = g_main_loop_new(sync->context, FALSE);

    return sync;
}

static void
trg_secret_sync_free(gpointer data)
{
    TrgSecretSync *sync = data;

    while(g_main_context_iteration(sync->context, FALSE));

    g_clear_object(&sync->result);
    g_main_loop_unref(sync->loop);
    g_main_context_unref(sync->context);
    g_free(sync);
}

static void
trg_secret_sync_on_result(GObject *source,
                          GAsyncResult *result,
                          gpointer user_data)
{
    TrgSecretSync *sync = user_data;
    g_assert(sync->result == NULL);
    sync->result = g_object_ref(result);
    g_main_loop_quit(sync->loop);
}

static gboolean
trg_secret_get_password_timeout(gpointer userdata)
{
    GCancellable *cancellable = (GCancellable*)userdata;

    if(!g_cancellable_is_cancelled(cancellable))
        g_cancellable_cancel(cancellable);

    return G_SOURCE_REMOVE;
}

gchar *
trg_secret_get_password(const gchar *uuid, guint timeout, GError **error)
{
    /* Basically recreating secret_password_lookup_sync, but with a timeout */

    TrgSecretSync *sync = trg_secret_sync_new();

    g_main_context_push_thread_default(sync->context);

    GCancellable *cancellable = g_cancellable_new();
    GSource *source = g_timeout_source_new(timeout);
    g_source_set_callback(source, trg_secret_get_password_timeout, cancellable, NULL);
    g_source_attach(source, sync->context);
    g_source_unref(source);

    secret_password_lookup(TRG_SECRET_SCHEMA, cancellable,
                           trg_secret_sync_on_result, sync,
                           TRG_PREFS_KEY_PROFILE_UUID, uuid,
                           NULL);

    g_main_loop_run(sync->loop);

    if(!g_cancellable_is_cancelled(cancellable))
        g_source_destroy(source);

    gchar *password = secret_password_lookup_nonpageable_finish(sync->result, error);

    g_main_context_pop_thread_default(sync->context);

    trg_secret_sync_free(sync);

    return password;
}

