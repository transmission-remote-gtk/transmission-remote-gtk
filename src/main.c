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
#include <curl/types.h>
#include <curl/easy.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <unique/unique.h>
#include "http.h"
#include "trg-main-window.h"
#include "trg-client.h"

enum {
    COMMAND_0,
    COMMAND_ADD
};

static UniqueResponse
message_received_cb(UniqueApp * app,
		    gint command,
		    UniqueMessageData * message,
		    guint time_, gpointer user_data)
{
    TrgMainWindow *win;
    UniqueResponse res;

    win = TRG_MAIN_WINDOW(user_data);

    switch (command) {
    case UNIQUE_ACTIVATE:
	gtk_window_set_screen(GTK_WINDOW(user_data),
			      unique_message_data_get_screen(message));
	gtk_window_present_with_time(GTK_WINDOW(user_data), time_);
	res = UNIQUE_RESPONSE_OK;
	break;
    case COMMAND_ADD:
	res =
	    trg_add_from_filename(win,
				  unique_message_data_get_filename
				  (message)) ? UNIQUE_RESPONSE_OK :
	    UNIQUE_RESPONSE_FAIL;
	break;
    default:
	res = UNIQUE_RESPONSE_OK;
	break;
    }

    return res;
}

int main(int argc, char *argv[])
{
    int returnValue;
    UniqueApp *app;
    TrgMainWindow *window;
    trg_client *client;

    g_type_init();
    g_thread_init(NULL);
    gdk_threads_init();
    gtk_init(&argc, &argv);

    app = unique_app_new_with_commands("org.eth0.uk.org.trg", NULL,
				       "add", COMMAND_ADD, NULL);

    if (unique_app_is_running(app)) {
	UniqueCommand command;
	UniqueResponse response;
	UniqueMessageData *message;

	if (argc > 1) {
	    command = COMMAND_ADD;
	    message = unique_message_data_new();
	    unique_message_data_set_filename(message, argv[1]);
	} else {
	    command = UNIQUE_ACTIVATE;
	    message = NULL;
	}

	response = unique_app_send_message(app, command, message);
	unique_message_data_free(message);

	if (response == UNIQUE_RESPONSE_OK) {
	    returnValue = 0;
	} else {
	    returnValue = 1;
	}
    } else {
	client = trg_init_client();

	curl_global_init(CURL_GLOBAL_ALL);

	window = trg_main_window_new(client);
	unique_app_watch_window(app, GTK_WINDOW(window));

	g_signal_connect(app, "message-received",
			 G_CALLBACK(message_received_cb), window);

	gtk_widget_show_all(GTK_WIDGET(window));

	auto_connect_if_required(window, client);
	gtk_main();

	curl_global_cleanup();
    }

    g_object_unref(app);
    gdk_threads_leave();

    return EXIT_SUCCESS;
}
