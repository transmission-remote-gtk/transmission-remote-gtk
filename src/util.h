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

/* Most of these functions are taken from the Transmission Project. */

#ifndef UTIL_H_
#define UTIL_H_

#include <gtk/gtk.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "trg-client.h"

#define trg_strlspeed(a, b) tr_formatter_speed_KBps(a, b, sizeof(a))
#define trg_strlpercent(a, b) tr_strlpercent(a, b, sizeof(a))
#define trg_strlsize(a, b) tr_formatter_size_B(a, b, sizeof(a))
#define trg_strlratio(a, b) tr_strlratio(a, b, sizeof(a))

#define TR_RATIO_NA  -1
#define TR_RATIO_INF -2

extern const int disk_K;
extern const char *disk_K_str;
extern const char *disk_M_str;
extern const char *disk_G_str;
extern const char *disk_T_str;

extern const int speed_K;
extern const char *speed_K_str;
extern const char *speed_M_str;
extern const char *speed_G_str;
extern const char *speed_T_str;

void add_file_id_to_array(JsonObject * args, const gchar * key,
                          gint index);
void g_str_slist_free(GSList * list);
GRegex *trg_uri_host_regex_new(void);
gchar *trg_gregex_get_first(GRegex * rx, const gchar * uri);
gchar *make_error_message(JsonObject * response, int status);
void trg_error_dialog(GtkWindow * parent, trg_response * response);
gchar *add_links_to_text(const gchar * original);

void
tr_formatter_size_init(unsigned int kilo,
                       const char *kb, const char *mb,
                       const char *gb, const char *tb);
char *tr_formatter_size_B(char *buf, gint64 bytes, size_t buflen);

void
tr_formatter_speed_init(unsigned int kilo,
                        const char *kb, const char *mb,
                        const char *gb, const char *tb);
char *tr_formatter_speed_KBps(char *buf, double KBps, size_t buflen);

char *tr_strltime_long(char *buf, long seconds, size_t buflen);
gchar *epoch_to_string(gint64 epoch);
char *tr_strltime_short(char *buf, long seconds, size_t buflen);
char *tr_strlpercent(char *buf, double x, size_t buflen);
char *tr_strratio(char *buf, size_t buflen, double ratio,
                  const char *infinity);
char *tr_strlratio(char *buf, double ratio, size_t buflen);
char *gtr_localtime(time_t time);
char *gtr_localtime2(char *buf, time_t time, size_t buflen);
int tr_snprintf(char *buf, size_t buflen, const char *fmt, ...);
int tr_snprintf(char *buf, size_t buflen, const char *fmt, ...);
size_t tr_strlcpy(char *dst, const void *src, size_t siz);
double tr_truncd(double x, int decimal_places);
int evutil_vsnprintf(char *buf, size_t buflen, const char *format,
                     va_list ap);
char *tr_strlsize(char *buf, guint64 bytes, size_t buflen);
void rm_trailing_slashes(gchar * str);
void trg_widget_set_visible(GtkWidget * w, gboolean visible);
gchar *trg_base64encode(const gchar * filename);
GtkWidget *my_scrolledwin_new(GtkWidget * child);
gboolean is_url(const gchar * string);
gboolean is_magnet(const gchar * string);
GtkWidget *gtr_combo_box_new_enum(const char *text_1, ...);

gboolean should_be_minimised(int argc, char *argv[]);
gboolean is_minimised_arg(const gchar * arg);
GtkWidget *trg_vbox_new(gboolean homogeneous, gint spacing);
GtkWidget *trg_hbox_new(gboolean homogeneous, gint spacing);
gboolean is_unity();

#ifdef WIN32
gchar *trg_win32_support_path(gchar * file);
#endif

#endif                          /* UTIL_H_ */
