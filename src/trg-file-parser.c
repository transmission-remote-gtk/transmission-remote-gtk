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

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include "bencode.h"
#include "trg-file-parser.h"

static trg_files_tree_node *trg_file_parser_node_insert(trg_files_tree_node
                                                        * top,
                                                        trg_files_tree_node
                                                        * last,
                                                        be_node *
                                                        file_node,
                                                        gint index)
{
    be_node *file_length_node = be_dict_find(file_node, "length", BE_INT);
    be_node *file_path_list = be_dict_find(file_node, "path", BE_LIST);
    trg_files_tree_node *lastIter = last;
    GList *parentList = NULL;
    be_node *path_el_node;
    GList *li;
    int i;

    if (!file_path_list || !file_length_node)
        return NULL;

    if (lastIter)
        while ((lastIter = lastIter->parent))
            parentList = g_list_prepend(parentList, lastIter);

    li = parentList;
    lastIter = top;

    /* Iterate over the path list which contains each file/directory
     * component of the path in order.
     */
    for (i = 0; (path_el_node = file_path_list->val.l[i]); i++) {
        gboolean isFile = !file_path_list->val.l[i + 1];
        trg_files_tree_node *target_node = NULL;

        if (li && !isFile) {
            trg_files_tree_node *lastPathNode = (trg_files_tree_node *) li->data;

            if (!g_strcmp0(lastPathNode->name, path_el_node->val.s)) {
                target_node = lastPathNode;
                li = g_list_next(li);
            } else {
                li = NULL;
            }
        }

        if (!target_node && lastIter && lastIter->childrenHash && !isFile)
          target_node = g_hash_table_lookup(lastIter->childrenHash, path_el_node->val.s);

        if (!target_node) {
            target_node = g_new0(trg_files_tree_node, 1);
            target_node->name = g_strdup(path_el_node->val.s);
            target_node->parent = lastIter;
            trg_files_tree_node_add_child(lastIter, target_node);
        }

        if (isFile) {
            target_node->length = (gint64) file_length_node->val.i;

            while (lastIter) {
                lastIter->length += target_node->length;
                lastIter = lastIter->parent;
            }
        }

        target_node->index = isFile ? index : -1;
        lastIter = target_node;
    }

    g_list_free(parentList);

    return lastIter;
}

void trg_torrent_file_free(trg_torrent_file * t)
{
    trg_files_tree_node_free(t->top_node);
    g_free(t->name);
    g_free(t);
}

static trg_files_tree_node *trg_parse_torrent_file_nodes(be_node *
                                                         info_node)
{
    be_node *files_node = be_dict_find(info_node, "files", BE_LIST);
    trg_files_tree_node *top_node = g_new0(trg_files_tree_node, 1);
    trg_files_tree_node *lastNode = NULL;
    int i;

    /* Probably means single file mode. */
    if (!files_node)
        return NULL;

    for (i = 0; files_node->val.l[i]; ++i) {
        be_node *file_node = files_node->val.l[i];

        if (!be_validate_node(file_node, BE_DICT)
            || !(lastNode =
                 trg_file_parser_node_insert(top_node, lastNode,
                                             file_node, i))) {
            /* Unexpected format. Throw away everything, file indexes need to
             * be correct. */
            trg_files_tree_node_free(top_node);
            return NULL;
        }
    }

    return top_node;
}

trg_torrent_file *trg_parse_torrent_data(const gchar *data, gsize length) {
	trg_torrent_file *ret = NULL;
	be_node *top_node, *info_node, *name_node;

    top_node = be_decoden(data, length);

    if (!top_node) {
        return NULL;
    } else if (!be_validate_node(top_node, BE_DICT)) {
        goto out;
    }

    info_node = be_dict_find(top_node, "info", BE_DICT);
    if (!info_node)
        goto out;

    name_node = be_dict_find(info_node, "name", BE_STR);
    if (!name_node)
        goto out;

    ret = g_new0(trg_torrent_file, 1);
    ret->name = g_strdup(name_node->val.s);

    ret->top_node = trg_parse_torrent_file_nodes(info_node);
    if (!ret->top_node) {
        trg_files_tree_node *file_node;
        be_node *length_node = be_dict_find(info_node, "length", BE_INT);

        if (!length_node) {
            g_free(ret);
            ret = NULL;
            goto out;
        }

        file_node = g_new0(trg_files_tree_node, 1);
        file_node->length = (gint64) (length_node->val.i);
        file_node->name = g_strdup(ret->name);
        ret->top_node = file_node;
    }

  out:
    be_free(top_node);
    return ret;
}

trg_torrent_file *trg_parse_torrent_file(const gchar * filename)
{
    GError *error = NULL;
    trg_torrent_file *ret = NULL;
    GMappedFile *mf;

    if (!g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
        g_message("%s does not exist", filename);
        return NULL;
    }

    mf = g_mapped_file_new(filename, FALSE, &error);

    if (error) {
        g_error("%s", error->message);
        g_error_free(error);
        g_mapped_file_unref(mf);
        return NULL;
    } else {
        ret = trg_parse_torrent_data(g_mapped_file_get_contents(mf), g_mapped_file_get_length(mf));
    }

    g_mapped_file_unref(mf);

    return ret;
}
