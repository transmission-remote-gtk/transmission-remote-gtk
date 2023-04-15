#include "config.h"

#include <glib.h>

#include "json.h"
#include "protocol-constants.h"
#include "requests.h"
#include "trg-client.h"
#include "trg-main-window.h"
#include "upload.h"
#include "util.h"

static gboolean upload_complete_callback(gpointer data);
static void next_upload(trg_upload *upload);

static void add_set_common_args(JsonObject *args, gint priority, gchar *dir)
{
    json_object_set_string_member(args, FIELD_FILE_DOWNLOAD_DIR, dir);
    json_object_set_int_member(args, FIELD_BANDWIDTH_PRIORITY, (gint64)priority);
}

void trg_upload_free(trg_upload *upload)
{
    g_str_slist_free(upload->list);
    g_free(upload->dir);
    g_free(upload->uid);
    g_free(upload->file_wanted);
    g_free(upload->file_priorities);
    trg_response_free(upload->upload_response);
    g_free(upload);
}

static void add_priorities(JsonObject *args, gint *priorities, gint n_files)
{
    gint i;
    for (i = 0; i < n_files; i++) {
        gint priority = priorities[i];
        if (priority == TR_PRI_LOW)
            add_file_id_to_array(args, FIELD_FILES_PRIORITY_LOW, i);
        else if (priority == TR_PRI_HIGH)
            add_file_id_to_array(args, FIELD_FILES_PRIORITY_HIGH, i);
        else
            add_file_id_to_array(args, FIELD_FILES_PRIORITY_NORMAL, i);
    }
}

static void add_wanteds(JsonObject *args, gint *wanteds, gint n_files)
{
    gint i;
    for (i = 0; i < n_files; i++) {
        if (wanteds[i])
            add_file_id_to_array(args, FIELD_FILES_WANTED, i);
        else
            add_file_id_to_array(args, FIELD_FILES_UNWANTED, i);
    }
}

static void next_upload(trg_upload *upload)
{
    JsonNode *req = NULL;
    g_autoptr(GError) error = NULL;

    if (upload->list && upload->progress_index < g_slist_length(upload->list))

        req = torrent_add_from_file((gchar *)g_slist_nth_data(upload->list, upload->progress_index),
                                    upload->flags, &error);

    if (req) {
        JsonObject *args = node_get_arguments(req);

        if (upload->extra_args)
            add_set_common_args(args, upload->priority, upload->dir);

        if (upload->file_wanted)
            add_wanteds(args, upload->file_wanted, upload->n_files);

        if (upload->file_priorities)
            add_priorities(args, upload->file_priorities, upload->n_files);

        upload->progress_index++;
        dispatch_rpc_async(upload->client, req, upload_complete_callback, upload);
    } else {
        if (error)
            trg_error_dialog(GTK_WINDOW(upload->main_window), error->message);

        trg_upload_free(upload);
    }
}

static gboolean upload_complete_callback(gpointer data)
{
    trg_response *response = (trg_response *)data;
    trg_upload *upload = (trg_upload *)response->cb_data;

    if (upload->callback)
        upload->callback(data);

    /* the callback we're delegating to will destroy the response */

    if (upload->main_window)
        on_generic_interactive_action(upload->main_window, response);
    else
        trg_response_free(response);

    next_upload(upload);

    return FALSE;
}

void trg_do_upload(trg_upload *upload)
{
    next_upload(upload);
}
