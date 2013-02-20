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

#if WIN32

#define TRG_MAILSLOT_NAME "\\\\.\\mailslot\\TransmissionRemoteGTK"
#define MAILSLOT_BUFFER_SIZE 1024*32

#include <windows.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "trg-main-window.h"
#include "win32-mailslot.h"

struct trg_mailslot_recv_args {
    TrgMainWindow *win;
    gchar **uris;
    gboolean present;
};

/* to be queued into the glib main loop with g_idle_add() */

static gboolean mailslot_recv_args(gpointer data)
{
    struct trg_mailslot_recv_args *args =
        (struct trg_mailslot_recv_args *) data;

    if (args->present) {
        gtk_window_deiconify(GTK_WINDOW(args->win));
        gtk_window_present(GTK_WINDOW(args->win));
    }

    if (args->uris)
        trg_add_from_filename(args->win, args->uris);

    g_free(args);

    return FALSE;
}

static gpointer mailslot_recv_thread(gpointer data)
{
    TrgMainWindow *win = TRG_MAIN_WINDOW(data);
    JsonParser *parser;
    char szBuffer[MAILSLOT_BUFFER_SIZE];
    HANDLE hMailslot;
    DWORD cbBytes;
    BOOL bResult;

    hMailslot = CreateMailslot(TRG_MAILSLOT_NAME,       /* mailslot name */
                               MAILSLOT_BUFFER_SIZE,    /* input buffer size */
                               MAILSLOT_WAIT_FOREVER,   /* no timeout */
                               NULL);   /* default security attribute */

    if (INVALID_HANDLE_VALUE == hMailslot) {
        g_error("\nError occurred while creating the mailslot: %d",
                GetLastError());
        return NULL;            /* Error */
    }

    while (1) {
        bResult = ReadFile(hMailslot,   /* handle to mailslot */
                           szBuffer,    /* buffer to receive data */
                           sizeof(szBuffer),    /* size of buffer */
                           &cbBytes,    /* number of bytes read */
                           NULL);       /* not overlapped I/O */

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

            args->present = json_object_has_member(obj, "present")
                && json_object_get_boolean_member(obj, "present");
            args->win = win;

            if (json_object_has_member(obj, "args")) {
                JsonArray *array =
                    json_object_get_array_member(obj, "args");
                GList *arrayList = json_array_get_elements(array);

                if (arrayList) {
                    guint arrayLength = g_list_length(arrayList);
                    guint i = 0;
                    GList *li;

                    args->uris = g_new0(gchar *, arrayLength + 1);

                    for (li = arrayList; li; li = g_list_next(li)) {
                        const gchar *liStr =
                            json_node_get_string((JsonNode *) li->data);
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
    return NULL;                /* Success */
}

void mailslot_start_background_listener(TrgMainWindow * win)
{
    g_thread_create(mailslot_recv_thread, win, FALSE, NULL);
}

gboolean mailslot_send_message(gchar ** args)
{
    HANDLE hMailSlot = CreateFile(TRG_MAILSLOT_NAME,    /* mailslot name */
                                  GENERIC_WRITE,        /* mailslot write only */
                                  FILE_SHARE_READ,      /* required for mailslots */
                                  NULL, /* default security attributes */
                                  OPEN_EXISTING,        /* opens existing mailslot */
                                  FILE_ATTRIBUTE_NORMAL,        /* normal attributes */
                                  NULL);        /* no template file */

    if (hMailSlot != INVALID_HANDLE_VALUE) {
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
        } else {
            json_object_set_boolean_member(obj, "present", TRUE);
        }

        json_node_take_object(node, obj);

        generator = json_generator_new();
        json_generator_set_root(generator, node);
        msg = json_generator_to_data(generator, NULL);

        json_node_free(node);
        g_object_unref(generator);

        WriteFile(hMailSlot,    /* handle to mailslot */
                  msg,          /* buffer to write from */
                  strlen(msg) + 1,      /* number of bytes to write, include the NULL */
                  &cbBytes,     /* number of bytes written */
                  NULL);

        CloseHandle(hMailSlot);
        g_free(msg);

        return TRUE;
    }

    return FALSE;
}

#endif
