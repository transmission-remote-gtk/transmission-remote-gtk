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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "trg-gtk-app.h"
#include "trg-main-window.h"
#include "util.h"

enum {
    PROP_0,
    PROP_CLIENT,
    PROP_MINIMISE_ON_START,
    N_PROPS,
};

struct _TrgGtkApp {
    GtkApplication parent;
};

typedef struct {
    TrgClient *client;
    gboolean min_start;
} TrgGtkAppPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(TrgGtkApp, trg_gtk_app, GTK_TYPE_APPLICATION)

static void trg_gtk_app_get_property(GObject *object, guint property_id, GValue *value,
                                     GParamSpec *pspec)
{
    TrgGtkAppPrivate *priv = trg_gtk_app_get_instance_private(TRG_GTK_APP(object));
    switch (property_id) {
    case PROP_CLIENT:
        g_value_set_pointer(value, priv->client);
        break;
    case PROP_MINIMISE_ON_START:
        g_value_set_boolean(value, priv->min_start);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_gtk_app_set_property(GObject *object, guint property_id, const GValue *value,
                                     GParamSpec *pspec)
{
    TrgGtkAppPrivate *priv = trg_gtk_app_get_instance_private(TRG_GTK_APP(object));
    switch (property_id) {
    case PROP_CLIENT:
        priv->client = g_value_get_pointer(value);
        break;
    case PROP_MINIMISE_ON_START:
        priv->min_start = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void trg_gtk_app_startup(GApplication *app, gpointer userdata)
{
    TrgGtkAppPrivate *priv = trg_gtk_app_get_instance_private(TRG_GTK_APP(app));
    TrgMainWindow *window = trg_main_window_new(priv->client, priv->min_start);
    gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
}

static int trg_gtk_app_command_line(GApplication *application, GApplicationCommandLine *cmdline)
{
    GList *windows = gtk_application_get_windows(GTK_APPLICATION(application));
    TrgMainWindow *window;
    gchar **argv;

    if (!windows || !windows->data)
        return 1;

    window = TRG_MAIN_WINDOW(windows->data);
    argv = g_application_command_line_get_arguments(cmdline, NULL);

    if (g_application_command_line_get_is_remote(cmdline)) {
        if (!argv[0]) {
            gtk_window_present(GTK_WINDOW(window));
            g_strfreev(argv);
        } else {
            return trg_add_from_filename(window, argv);
        }
    } else {
        trg_main_window_set_start_args(window, argv);
        auto_connect_if_required(TRG_MAIN_WINDOW(windows->data));
    }

    return 0;
}

static void shift_args(gchar **argv, int i)
{
    gint j;
    g_free(argv[i]);
    for (j = i; argv[j]; j++)
        argv[j] = argv[j + 1];
}

static gboolean test_local_cmdline(GApplication *application, gchar ***arguments, gint *exit_status)
{
    TrgGtkAppPrivate *priv = trg_gtk_app_get_instance_private(TRG_GTK_APP(application));
    gchar **argv;
    gchar *cwd = g_get_current_dir();
    gchar *tmp;
    gint i;

    argv = *arguments;
    shift_args(argv, 0);

    i = 0;
    while (argv[i]) {
        if (is_minimised_arg(argv[i])) {
            shift_args(argv, i);
            priv->min_start = TRUE;
        } else if (!is_url(argv[i]) && !is_magnet(argv[i])
                   && g_file_test(argv[i], G_FILE_TEST_IS_REGULAR)
                   && !g_path_is_absolute(argv[i])) {
            tmp = g_build_path(G_DIR_SEPARATOR_S, cwd, argv[i], NULL);
            g_free(argv[i]);
            argv[i] = tmp;
        }
        i++;
    }

    *exit_status = 0;

    g_free(cwd);

    return FALSE;
}

static void trg_gtk_app_class_init(TrgGtkAppClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GApplicationClass *app_class = G_APPLICATION_CLASS(klass);

    object_class->get_property = trg_gtk_app_get_property;
    object_class->set_property = trg_gtk_app_set_property;
    app_class->local_command_line = test_local_cmdline;
    // app_class->startup = trg_gtk_app_startup;
    app_class->command_line = trg_gtk_app_command_line;

    g_object_class_install_property(
        object_class, PROP_CLIENT,
        g_param_spec_pointer("trg-client", "TClient", _("Client"),
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(
        object_class, PROP_MINIMISE_ON_START,
        g_param_spec_boolean("min-on-start", "Min On Start", _("Min On Start"), FALSE,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
}

static void trg_gtk_app_init(TrgGtkApp *self)
{
    g_application_set_inactivity_timeout(G_APPLICATION(self), 10000);
    g_signal_connect(self, "command-line", G_CALLBACK(trg_gtk_app_command_line), NULL);
    g_signal_connect(self, "startup", G_CALLBACK(trg_gtk_app_startup), NULL);
}

TrgGtkApp *trg_gtk_app_new(TrgClient *client)
{
    return g_object_new(TRG_TYPE_GTK_APP, "application-id", APPLICATION_ID, "flags",
                        G_APPLICATION_HANDLES_COMMAND_LINE, "trg-client", client, NULL);
}
