# create a config.h for rss-glib that sets the glib API versions
# to the same one used in transmission-remote-gtk
rss_conf_data = configuration_data()
rss_conf_data.set('GLIB_VERSION_MAX_ALLOWED', glib_max)
rss_conf_data.set('GLIB_VERSION_MIN_REQUIRED', glib_min)
configure_file(output: 'rss-config.h', configuration: rss_conf_data)


rss_src_files = ['rss-document.c', 'rss-item.c', 'rss-parser.c']
rss_includes = include_directories('..')
glib_rss_lib = static_library('rss',
                              sources: rss_src_files,
                              dependencies: [rss_dep, glib_dep, gio_dep])
