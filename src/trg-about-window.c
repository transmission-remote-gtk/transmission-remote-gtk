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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "trg-about-window.h"
#include "util.h"

GtkWidget *trg_about_window_new(GtkWindow * parent)
{
    GtkWidget *dialog;
    GdkPixbuf *logo;
    gchar *licenseText = NULL;
    const gchar *trgAuthors[] = { "Alan Fitton <alan@eth0.org.uk>", NULL };
    gchar *licenseFile;

#ifdef WIN32
    licenseFile = trg_win32_support_path("COPYING.TXT");
#else
    licenseFile = g_strdup(TRGLICENSE);
#endif

    dialog = gtk_about_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

    logo =
        gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
                                 PACKAGE_NAME, 48,
                                 GTK_ICON_LOOKUP_USE_BUILTIN, NULL);

    if (logo != NULL) {
        gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), logo);
        g_object_unref(logo);
    }

    if (g_file_get_contents(licenseFile, &licenseText, NULL, NULL)) {
        gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog),
                                     licenseText);
    } else {
        gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog), "GPL2");
    }

    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog),
                                      PACKAGE_NAME);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog),
                                 PACKAGE_VERSION);
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
                                   "(C) 2011-2013 Alan Fitton");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
                                  _
                                  ("A remote client to transmission-daemon."));

    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog),
                                 "http://code.google.com/p/transmission-remote-gtk/");
    gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(dialog),
                                       "http://code.google.com/p/transmission-remote-gtk/");

    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), trgAuthors);
    /*gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(dialog), documenters); */
    gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dialog),
                                            "translations kindly contributed by\n\n"
                                            "* Pierre Rudloff (French)\n"
                                            "* Julian Held (German)\n"
                                            "* Algimantas Margevičius (Lithuanian)\n"
                                            "* Youn sok Choi (Korean)\n"
                                            "* Piotr (Polish)\n"
                                            "* Y3AVD (Russian)\n"
                                            "* aspidzent (Spanish)\n"
                                            "* Åke Svensson (Swedish)\n"
                                            "* ROR191 (Ukranian)\n");

    g_free(licenseFile);
    g_free(licenseText);

    return dialog;
}
