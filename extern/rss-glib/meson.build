rss_src_files = ['rss-document.c', 'rss-item.c', 'rss-parser.c']
rss_includes = include_directories('..')
glib_rss_lib = static_library('rss',
                              sources: rss_src_files,
                              dependencies: [rss_dep, glib_dep, gio_dep])
