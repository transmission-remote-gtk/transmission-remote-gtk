/*
 * icons.[ch] written by Paolo Bacchilega, who writes:
 * "There is no problem for me, you can license
 * my code under whatever licence you wish :)"
 *
 * $Id: icons.h 11709 2011-01-19 13:48:47Z jordan $
 */

#ifndef GTR_ICONS_H
#define GTR_ICONS_H

#include <gtk/gtk.h>

#define DIRECTORY_MIME_TYPE "folder"
#define UNKNOWN_MIME_TYPE   "unknown"

const char *gtr_get_mime_type_from_filename(const char *file);

GdkPixbuf *gtr_get_mime_type_icon(const char *mime_type, GtkIconSize icon_size,
                                  GtkWidget *for_widget);

#endif
