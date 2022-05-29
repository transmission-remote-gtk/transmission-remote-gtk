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

#include <glib.h>
#include <gtk/gtk.h>
#include <json-glib/json-glib.h>

#include "protocol-constants.h"
#include "remote-exec.h"
#include "torrent.h"
#include "trg-prefs.h"
#include "trg-torrent-model.h"

/* A few functions used to build local commands, otherwise known as actions.
 *
 * The functionality from a user perspective is documented in the wiki.
 * The code below really just uses GRegex to replace variable identifier
 * with the values inside the connected profile, the session, or the first selected
 * torrent (in that order of precedence). A field seperator I call a repeater
 * can be appended to a variable in square brackets, like %{id}[,] to
 * cause it to be repeated for each selection.
 */

static const char json_exceptions[] = {
    0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d,
    0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c,
    0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab,
    0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
    0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
    0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, '\0' /* g_strescape() expects a
                                                                  NUL-terminated string */
};

static gchar *dump_json_value(JsonNode *node)
{
    GValue value = G_VALUE_INIT;
    GString *buffer;

    buffer = g_string_new("");

    json_node_get_value(node, &value);

    switch (G_VALUE_TYPE(&value)) {
    case G_TYPE_INT64:
        g_string_append_printf(buffer, "%" G_GINT64_FORMAT, g_value_get_int64(&value));
        break;
    case G_TYPE_STRING: {
        gchar *tmp;

        tmp = g_strescape(g_value_get_string(&value), json_exceptions);
        g_string_append(buffer, tmp);

        g_free(tmp);
    } break;
    case G_TYPE_DOUBLE: {
        gchar buf[G_ASCII_DTOSTR_BUF_SIZE];

        g_string_append(buffer, g_ascii_dtostr(buf, sizeof(buf), g_value_get_double(&value)));
    } break;
    case G_TYPE_BOOLEAN:
        g_string_append_printf(buffer, "%s", g_value_get_boolean(&value) ? "true" : "false");
        break;
    default:
        break;
    }

    g_value_unset(&value);

    return g_string_free(buffer, FALSE);
}

gchar *build_remote_exec_cmd(TrgClient *tc, GtkTreeModel *model, GList *selection,
                             const gchar *input)
{
    TrgPrefs *prefs = trg_client_get_prefs(tc);
    JsonObject *session = trg_client_get_session(tc);
    JsonObject *profile = trg_prefs_get_connection(prefs);
    gchar *work;
    GRegex *regex, *replacerx;
    GMatchInfo *match_info;
    gchar *whole, *wholeEscaped, *id, *tmp, *valueEscaped, *valuestr, *repeater;
    JsonNode *replacement;

    if (!profile)
        return NULL;

    work = g_strdup(input);
    regex = g_regex_new("%{([A-Za-z\\-]+)}(?:\\[(.*)\\])?", 0, 0, NULL);

    g_regex_match_full(regex, input, -1, 0, 0, &match_info, NULL);

    if (match_info) {
        while (g_match_info_matches(match_info)) {
            whole = g_match_info_fetch(match_info, 0);
            wholeEscaped = g_regex_escape_string(whole, -1);
            id = g_match_info_fetch(match_info, 1);
            repeater = g_match_info_fetch(match_info, 2);

            replacerx = g_regex_new(wholeEscaped, 0, 0, NULL);
            valuestr = NULL;

            if (profile && json_object_has_member(profile, id)) {
                replacement = json_object_get_member(profile, id);
                if (JSON_NODE_HOLDS_VALUE(replacement))
                    valuestr = dump_json_value(replacement);
            } else if (session && json_object_has_member(session, id)) {
                replacement = json_object_get_member(session, id);
                if (JSON_NODE_HOLDS_VALUE(replacement))
                    valuestr = dump_json_value(replacement);
            } else {
                GString *gs = g_string_new("");
                GList *li;
                GtkTreeIter iter;
                JsonObject *json;
                gchar *piece;

                for (li = selection; li; li = g_list_next(li)) {
                    piece = NULL;
                    gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)li->data);
                    gtk_tree_model_get(model, &iter, TORRENT_COLUMN_JSON, &json, -1);
                    if (json_object_has_member(json, id)) {
                        replacement = json_object_get_member(json, id);
                        if (JSON_NODE_HOLDS_VALUE(replacement)) {
                            piece = dump_json_value(replacement);
                        }
                    }

                    if (!piece) {
                        if (!g_strcmp0(id, "full-dir")) {
                            piece = torrent_get_full_dir(json);
                        } else if (!g_strcmp0(id, "full-path")) {
                            piece = torrent_get_full_path(json);
                        }
                    }

                    if (piece) {
                        g_string_append(gs, piece);
                        g_free(piece);
                    }

                    if (!repeater)
                        break;

                    if (piece && li != g_list_last(selection))
                        g_string_append(gs, repeater);
                }

                if (gs->len > 0)
                    valuestr = g_string_free(gs, FALSE);
                else
                    g_string_free(gs, TRUE);
            }

            if (valuestr) {
                // Values are not guaranteed to be shell safe
                valueEscaped = g_shell_quote(valuestr);

                tmp = g_regex_replace(replacerx, work, -1, 0, valueEscaped, 0, NULL);
                g_free(work);
                work = tmp;
                g_free(valuestr);
                g_free(valueEscaped);
            }

            g_regex_unref(replacerx);
            g_free(whole);
            g_free(repeater);
            g_free(wholeEscaped);
            g_free(id);
            g_match_info_next(match_info, NULL);
        }

        g_match_info_free(match_info);
    }

    g_regex_unref(regex);
    return work;
}
