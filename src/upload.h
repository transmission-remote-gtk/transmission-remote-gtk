#ifndef UPLOAD_H_
#define UPLOAD_H_

#include <glib.h>

#include "trg-client.h"
#include "trg-main-window.h"

typedef struct {
    GSList *list; // list of filenames
    trg_response *upload_response; // OR: a HTTP response containing a torrent
    TrgClient *client;
    gpointer cb_data;
    TrgMainWindow *main_window; // a parent window to attach any error dialogs to
    guint flags;
    gchar *dir;
    gint priority;
    gboolean extra_args;
    guint progress_index;

} trg_upload;

void trg_do_upload(trg_upload *upload);

#endif
