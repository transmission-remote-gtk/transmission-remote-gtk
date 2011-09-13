typedef struct {
    char *name;
    gint64 length;
    GList *children;
    guint index;
} trg_torrent_file_node;

typedef struct {
    char *name;
    trg_torrent_file_node *top_node;
    gint64 total_length;
} trg_torrent_file;

void trg_torrent_file_free(trg_torrent_file * t);
trg_torrent_file *trg_parse_torrent_file(const gchar *filename);
