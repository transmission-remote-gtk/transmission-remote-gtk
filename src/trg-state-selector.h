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

#ifndef TRG_STATE_LIST_H_
#define TRG_STATE_LIST_H_

#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"
#include "trg-torrent-model.h"

enum {
    STATE_SELECTOR_ICON,
    STATE_SELECTOR_NAME,
    STATE_SELECTOR_COUNT,
    STATE_SELECTOR_BIT,
    STATE_SELECTOR_SERIAL,
    STATE_SELECTOR_INDEX,
    STATE_SELECTOR_COLUMNS
};

G_BEGIN_DECLS
#define TRG_TYPE_STATE_SELECTOR trg_state_selector_get_type()
#define TRG_STATE_SELECTOR(obj)                                                                    \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TRG_TYPE_STATE_SELECTOR, TrgStateSelector))
#define TRG_STATE_SELECTOR_CLASS(klass)                                                            \
    (G_TYPE_CHECK_CLASS_CAST((klass), TRG_TYPE_STATE_SELECTOR, TrgStateSelectorClass))
#define TRG_IS_STATE_SELECTOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TRG_TYPE_STATE_SELECTOR))
#define TRG_IS_STATE_SELECTOR_CLASS(klass)                                                         \
    (G_TYPE_CHECK_CLASS_TYPE((klass), TRG_TYPE_STATE_SELECTOR))
#define TRG_STATE_SELECTOR_GET_CLASS(obj)                                                          \
    (G_TYPE_INSTANCE_GET_CLASS((obj), TRG_TYPE_STATE_SELECTOR, TrgStateSelectorClass))
typedef struct {
    GtkTreeView parent;
} TrgStateSelector;

typedef struct {
    GtkTreeViewClass parent_class;

    void (*torrent_state_changed)(TrgStateSelector *selector, guint flag, gpointer data);

} TrgStateSelectorClass;

GType trg_state_selector_get_type(void);
TrgStateSelector *trg_state_selector_new(TrgClient *client, TrgTorrentModel *tmodel);

G_END_DECLS guint32 trg_state_selector_get_flag(TrgStateSelector *s);
void trg_state_selector_update(TrgStateSelector *s, guint whatsChanged);
gchar *trg_state_selector_get_selected_text(TrgStateSelector *s);
GRegex *trg_state_selector_get_url_host_regex(TrgStateSelector *s);
void trg_state_selector_disconnect(TrgStateSelector *s);
void trg_state_selector_set_show_trackers(TrgStateSelector *s, gboolean show);
void trg_state_selector_set_directories_first(TrgStateSelector *s, gboolean _dirsFirst);
void trg_state_selector_set_show_dirs(TrgStateSelector *s, gboolean show);
void trg_state_selector_set_queues_enabled(TrgStateSelector *s, gboolean enabled);
void trg_state_selector_stats_update(TrgStateSelector *s, trg_torrent_model_update_stats *stats);

#endif /* TRG_STATE_LIST_H_ */
