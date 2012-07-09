/* trg-torrent-graph.h */

#ifndef _TRG_TORRENT_GRAPH3
#define _TRG_TORRENT_GRAPH3

#include <gtk/gtk.h>

#if GTK_CHECK_VERSION( 3, 0, 0 )

#include <glib-object.h>
#include "trg-torrent-model.h"

G_BEGIN_DECLS
#define TRG_TYPE_TORRENT_GRAPH trg_torrent_graph_get_type()
#define TRG_TORRENT_GRAPH(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRG_TYPE_TORRENT_GRAPH, TrgTorrentGraph))
#define TRG_TORRENT_GRAPH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TRG_TYPE_TORRENT_GRAPH, TrgTorrentGraphClass))
#define TRG_IS_TORRENT_GRAPH(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRG_TYPE_TORRENT_GRAPH))
#define TRG_IS_TORRENT_GRAPH_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRG_TYPE_TORRENT_GRAPH))
#define TRG_TORRENT_GRAPH_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TRG_TYPE_TORRENT_GRAPH, TrgTorrentGraphClass))
    typedef struct {
    GtkVBox parent;
} TrgTorrentGraph;

typedef struct {
    GtkVBoxClass parent_class;
} TrgTorrentGraphClass;

GType trg_torrent_graph_get_type(void);

TrgTorrentGraph *trg_torrent_graph_new(GtkStyle * style);

unsigned trg_torrent_graph_get_num_bars(TrgTorrentGraph * g);

void trg_torrent_graph_clear_background();

void trg_torrent_graph_draw(TrgTorrentGraph * g);

void trg_torrent_graph_start(TrgTorrentGraph * g);

void trg_torrent_graph_stop(TrgTorrentGraph * g);

void trg_torrent_graph_change_speed(TrgTorrentGraph * g, guint new_speed);

void trg_torrent_graph_set_speed(TrgTorrentGraph * g,
                                 trg_torrent_model_update_stats * stats);
void trg_torrent_graph_set_nothing(TrgTorrentGraph * g);

G_END_DECLS
#endif
#endif                          /* _TRG_TORRENT_GRAPH */
