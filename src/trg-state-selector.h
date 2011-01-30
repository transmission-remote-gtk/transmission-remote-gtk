/*
 * transmission-remote-gtk - Transmission RPC client for GTK
 * Copyright (C) 2010  Alan Fitton

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

enum {
    STATE_SELECTOR_ICON,
    STATE_SELECTOR_NAME,
    STATE_SELECTOR_BIT,
    STATE_SELECTOR_COLUMNS
};

G_BEGIN_DECLS
#define TRG_TYPE_STATE_SELECTOR trg_state_selector_get_type()
#define TRG_STATE_SELECTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_STATE_SELECTOR, TrgStateSelector))
#define TRG_STATE_SELECTOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_STATE_SELECTOR, TrgStateSelectorClass))
#define TRG_IS_STATE_SELECTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_STATE_SELECTOR))
#define TRG_IS_STATE_SELECTOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_STATE_SELECTOR))
#define TRG_STATE_SELECTOR_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_STATE_SELECTOR, TrgStateSelectorClass))
    typedef struct {
    GtkTreeView parent;
} TrgStateSelector;

typedef struct {
    GtkTreeViewClass parent_class;

    /* SIGNALS */

    void (*torrent_state_changed) (TrgStateSelector * selector,
				   guint flag, gpointer data);
} TrgStateSelectorClass;

GType trg_state_selector_get_type(void);
TrgStateSelector *trg_state_selector_new(void);

G_END_DECLS guint32 trg_state_selector_get_flag(TrgStateSelector * s);

#endif				/* TRG_STATE_LIST_H_ */
