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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#ifdef HAVE_LIBUNIQUE
#include <unique/unique.h>
#elif WIN32
#include <windows.h>
#endif

#include "trg-main-window.h"
#include "trg-client.h"
#include "util.h"

#define TRG_LIBUNIQUE_DOMAIN "uk.org.eth0.trg"
#define TRG_MAILSLOT_NAME "\\\\.\\mailslot\\TransmissionRemoteGTK"  //Name given to the Mailslot
#define MAILSLOT_BUFFER_SIZE 1024*32

#ifdef HAVE_LIBUNIQUE

enum {
    COMMAND_0,
    COMMAND_ADD
};

static UniqueResponse
message_received_cb(UniqueApp * app G_GNUC_UNUSED,
        gint command,
        UniqueMessageData * message,
        guint time_, gpointer user_data)
{
    TrgMainWindow *win;
    UniqueResponse res;
    gchar **uris;

    win = TRG_MAIN_WINDOW(user_data);

    switch (command) {
        case UNIQUE_ACTIVATE:
        gtk_window_set_screen(GTK_WINDOW(user_data),
                unique_message_data_get_screen(message));
        gtk_window_present_with_time(GTK_WINDOW(user_data), time_);
        res = UNIQUE_RESPONSE_OK;
        break;
        case COMMAND_ADD:
        uris = unique_message_data_get_uris(message);
        res =
        trg_add_from_filename(win,
                uris) ? UNIQUE_RESPONSE_OK :
        UNIQUE_RESPONSE_FAIL;
        break;
        default:
        res = UNIQUE_RESPONSE_OK;
        break;
    }

    return res;
}

#elif WIN32

struct trg_mailslot_recv_args {
    TrgMainWindow *win;
    gchar **uris;
    gboolean present;
};

/* to be queued into the glib main loop with g_idle_add() */

static gboolean mailslot_recv_args(gpointer data) {
    struct trg_mailslot_recv_args *args = (struct trg_mailslot_recv_args*) data;

    if (args->uris)
        trg_add_from_filename(args->win, args->uris);

    if (args->present) {
        gtk_window_deiconify(GTK_WINDOW(args->win));
        gtk_window_present(GTK_WINDOW(args->win));
    }

    g_free(args);

    return FALSE;
}

static gpointer mailslot_recv_thread(gpointer data) {
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    JsonParser *parser;
    char szBuffer[MAILSLOT_BUFFER_SIZE];
    HANDLE hMailslot;
    DWORD cbBytes;
    BOOL bResult;

    hMailslot = CreateMailslot(TRG_MAILSLOT_NAME, // mailslot name
            MAILSLOT_BUFFER_SIZE, // input buffer size
            MAILSLOT_WAIT_FOREVER, // no timeout
            NULL); // default security attribute

    if (INVALID_HANDLE_VALUE == hMailslot) {
        g_error(
                "\nError occurred while creating the mailslot: %d", GetLastError());
        return NULL; //Error
    }

    while (1) {
        bResult = ReadFile(hMailslot, // handle to mailslot
                szBuffer, // buffer to receive data
                sizeof(szBuffer), // size of buffer
                &cbBytes, // number of bytes read
                NULL); // not overlapped I/O

        if ((!bResult) || (0 == cbBytes)) {
            g_error("Mailslot error from client: %d", GetLastError());
            break;
        }

        parser = json_parser_new();

        if (json_parser_load_from_data(parser, szBuffer, cbBytes, NULL)) {
            JsonNode *node = json_parser_get_root(parser);
            JsonObject *obj = json_node_get_object(node);
            struct trg_mailslot_recv_args *args =
                    g_new0(struct trg_mailslot_recv_args, 1);

            args->present = json_object_has_member(obj, "present") && json_object_get_boolean_member(obj, "present");
            args->win = win;

            if (json_object_has_member(obj, "args")) {
                JsonArray *array = json_node_get_array(node);
                GList *arrayList = json_array_get_elements(array);
                if (arrayList) {
                    guint arrayLength = g_list_length(arrayList);
                    int i = 0;
                    GList *li;

                    args->uris = g_new0(gchar*, arrayLength+1);

                    for (li = arrayList; li; li = g_list_next(li)) {
                        const gchar *liStr = json_node_get_string((JsonNode*) li->data);
                        args->uris[i++] = g_strdup(liStr);
                    }

                    g_list_free(arrayList);
                }
            }

            json_node_free(node);

            g_idle_add(mailslot_recv_args, args);
        }

        g_object_unref(parser);
    }

    CloseHandle(hMailslot);
    return NULL; //Success
}

static int winunique_send_message(HANDLE h, gchar **args) {
    DWORD cbBytes;
    JsonNode *node = json_node_new(JSON_NODE_OBJECT);
    JsonObject *obj = json_object_new();
    JsonArray *array = json_array_new();
    JsonGenerator *generator;
    gchar *msg;
    int i;

    if (args) {
        for (i = 0; args[i]; i++)
            json_array_add_string_element(array, args[i]);

        json_object_set_array_member(obj, "args", array);

        g_strfreev(args);
    }

    json_object_set_boolean_member(obj, "present", TRUE);

    json_node_take_object(node, obj);

    generator = json_generator_new();
    json_generator_set_root(generator, node);
    msg = json_generator_to_data(generator, NULL);

    json_node_free(node);
    g_object_unref(generator);

    WriteFile(h, // handle to mailslot
            msg, // buffer to write from
            strlen(msg) + 1, // number of bytes to write, include the NULL
            &cbBytes, // number of bytes written
            NULL);

    CloseHandle(h);
    g_free(msg);

    return 0;
}

#endif

static gboolean should_be_minimised(int argc, char *argv[]) {
    int i;
    for (i = 1; i < argc; i++)
        if (!g_strcmp0(argv[i], "-m")
                || !g_strcmp0(argv[i], "--minimized")
                || !g_strcmp0(argv[i], "/m"))
            return TRUE;

    return FALSE;
}

static gchar **convert_args(int argc, char *argv[]) {
    gchar *cwd = g_get_current_dir();
    gchar **files = NULL;
    int i;
    if (argc > 1) {
        files = g_new0(gchar *, argc);
        for (i = 1; i < argc; i++) {
            if (!is_url(argv[i]) && !is_magnet(argv[i])
                    && g_file_test(argv[i], G_FILE_TEST_IS_REGULAR)
                    && !g_path_is_absolute(argv[i])) {
                files[i - 1] = g_build_path(G_DIR_SEPARATOR_S, cwd, argv[i],
                        NULL);
            } else {
                files[i - 1] = g_strdup(argv[i]);
            }
        }
    }

    g_free(cwd);

    return files;
}

int main(int argc, char *argv[]) {
    int returnValue = EXIT_SUCCESS;
    TrgMainWindow *window;
    TrgClient *client;
    gchar **args = convert_args(argc, argv);
    gboolean withUnique;
#ifdef HAVE_LIBUNIQUE
    UniqueApp *app = NULL;
#endif
#ifdef WIN32
    gchar *localedir, *moddir;
    HANDLE hMailSlot;
#endif
#ifdef TRG_MEMPROFILE
    GMemVTable gmvt = {malloc,realloc,free,calloc,malloc,realloc};
    g_mem_set_vtable(&gmvt);
    g_mem_set_vtable(glib_mem_profiler_table);
    g_mem_profile();
#endif

    g_type_init();
    g_thread_init(NULL);
    gtk_init(&argc, &argv);

    g_set_application_name(PACKAGE_NAME);
#ifdef WIN32
    moddir = g_win32_get_package_installation_directory_of_module(NULL);
    localedir = g_build_path(G_DIR_SEPARATOR_S, moddir, "share", "locale",
            NULL);
    g_free(moddir);
    bindtextdomain(GETTEXT_PACKAGE, localedir);
    g_free(localedir);
#else
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
#endif
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    withUnique = (g_getenv("TRG_NOUNIQUE") == NULL);

#ifdef HAVE_LIBUNIQUE
    if (withUnique)
    app = unique_app_new_with_commands(TRG_LIBUNIQUE_DOMAIN, NULL,
            "add", COMMAND_ADD, NULL);

    if (withUnique && unique_app_is_running(app)) {
        UniqueCommand command;
        UniqueResponse response;
        UniqueMessageData *message;

        if (args) {
            command = COMMAND_ADD;
            message = unique_message_data_new();
            unique_message_data_set_uris(message, args);
            g_strfreev(args);
        } else {
            command = UNIQUE_ACTIVATE;
            message = NULL;
        }

        response = unique_app_send_message(app, command, message);
        unique_message_data_free(message);

        if (response != UNIQUE_RESPONSE_OK)
            returnValue = EXIT_FAILURE;
    } else {
#elif WIN32
    hMailSlot = CreateFile(TRG_MAILSLOT_NAME, // mailslot name
            GENERIC_WRITE, // mailslot write only
            FILE_SHARE_READ, // required for mailslots
            NULL, // default security attributes
            OPEN_EXISTING, // opens existing mailslot
            FILE_ATTRIBUTE_NORMAL, // normal attributes
            NULL); // no template file

    if (INVALID_HANDLE_VALUE != hMailSlot) {
        returnValue = winunique_send_message(hMailSlot, args);
    } else {
#endif
        client = trg_client_new();

        curl_global_init(CURL_GLOBAL_ALL);

        window = trg_main_window_new(client, should_be_minimised(argc, argv));

#ifdef HAVE_LIBUNIQUE
        if (withUnique) {
            unique_app_watch_window(app, GTK_WINDOW(window));
            g_signal_connect(app, "message-received",
                    G_CALLBACK(message_received_cb), window);
        }
#elif WIN32
        g_thread_create(mailslot_recv_thread, window, FALSE, NULL);
#endif

        auto_connect_if_required(window, args);
        gtk_main();

        curl_global_cleanup();
#ifdef HAVE_LIBUNIQUE
    }

    if (withUnique)
    g_object_unref(app);
#elif WIN32
    }
#endif

    return returnValue;
}
