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


#include <glib/gprintf.h>

/*
 * Suppress: "warning: missing initializer for field ‘reserved’ of ‘SecretSchema 
 * {aka const struct <anonymous>}’ [-Wmissing-field-initializers]" which is harmless
 */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

typedef struct {
   const gchar *uuid;
   GCancellable *cancellable;
   GError **error;
} TrgSecretGetPasswordTaskData;

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

static gboolean
trg_secret_get_password_timeout(gpointer userdata)
{
    GCancellable *cancellable = (GCancellable*)userdata;

    if(!g_cancellable_is_cancelled(cancellable))
        g_cancellable_cancel(cancellable);

    return FALSE;
}

static void
trg_secret_get_password_task(GTask *task, gpointer source, gpointer userdata, GCancellable *notused)
{
    TrgSecretGetPasswordTaskData *task_data = (TrgSecretGetPasswordTaskData*)userdata;

    gchar *password = secret_password_lookup_nonpageable_sync (TRG_SECRET_SCHEMA,
                                                               task_data->cancellable,
                                                               task_data->error,
                                                               TRG_PREFS_KEY_PROFILE_UUID,
                                                               task_data->uuid,
                                                               NULL);
    g_task_return_pointer (task, password, NULL);
}

gchar *
trg_secret_get_password(const gchar *uuid, guint timeout, GError **error)
{
    TrgSecretGetPasswordTaskData *task_data = g_new0(TrgSecretGetPasswordTaskData,1);

    task_data->uuid = uuid;
    task_data->cancellable = g_cancellable_new();
    task_data->error = error;

    guint timeout_source_id = g_timeout_add (timeout, 
                                             trg_secret_get_password_timeout,
                                             task_data->cancellable);

    GTask *task = g_task_new(NULL, NULL, NULL, NULL);
    g_task_set_task_data(task, task_data, NULL);
    g_task_run_in_thread_sync(task, trg_secret_get_password_task);

    if(!g_cancellable_is_cancelled(task_data->cancellable))
        g_source_remove(timeout_source_id);

    gchar *password = g_task_propagate_pointer (task, NULL);

    g_object_unref(task);
    g_free(task_data);

    return password;
}
