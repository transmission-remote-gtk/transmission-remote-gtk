src_files = [
    'bencode.c',
    'hig.c',
    'icons.c',
    'json.c',
    'main.c',
    'remote-exec.c',
    'requests.c',
    'session-get.c',
    'torrent.c',
    'torrent-cell-renderer.c',
    'trg-about-window.c',
    'trg-cell-renderer-counter.c',
    'trg-cell-renderer-epoch.c',
    'trg-cell-renderer-eta.c',
    'trg-cell-renderer-file-icon.c',
    'trg-cell-renderer-numgteqthan.c',
    'trg-cell-renderer-priority.c',
    'trg-cell-renderer-ratio.c',
    'trg-cell-renderer-size.c',
    'trg-cell-renderer-speed.c',
    'trg-cell-renderer-wanted.c',
    'trg-client.c',
    'trg-destination-combo.c',
    'trg-file-parser.c',
    'trg-file-rename-dialog.c',
    'trg-files-model.c',
    'trg-files-model-common.c',
    'trg-files-tree.c',
    'trg-files-tree-view.c',
    'trg-files-tree-view-common.c',
    'trg-general-panel.c',
    'trg-gtk-app.c',
    'trg-json-widgets.c',
    'trg-main-window.c',
    'trg-menu-bar.c',
    'trg-model.c',
    'trg-peers-model.c',
    'trg-peers-tree-view.c',
    'trg-persistent-tree-view.c',
    'trg-preferences-dialog.c',
    'trg-prefs.c',
    'trg-remote-prefs-dialog.c',
    'trg-sortable-filtered-model.c',
    'trg-state-selector.c',
    'trg-stats-dialog.c',
    'trg-status-bar.c',
    'trg-toolbar.c',
    'trg-torrent-add-dialog.c',
    'trg-torrent-add-url-dialog.c',
    'trg-torrent-graph.c',
    'trg-torrent-model.c',
    'trg-torrent-move-dialog.c',
    'trg-torrent-props-dialog.c',
    'trg-torrent-tree-view.c',
    'trg-trackers-model.c',
    'trg-trackers-tree-view.c',
    'trg-tree-view.c',
    'upload.c',
    'util.c'
]

# generate config file
configure_file(output: 'config.h', configuration: conf_data)


# man pages
pod2man = find_program('pod2man', required: false)
if pod2man.found()
  manpages = custom_target('manpages',
                           input: '@0@.pod'.format(project_name),
                           output: '@0@.1'.format(project_name),
                           command: [pod2man, '--release=@0@'.format(project_version),
                                     '--center="Transmission Remote GTK"',
                                     '@INPUT@', '@OUTPUT@'],
                           install: true,
                           install_dir: get_option('mandir') / 'man1')
endif

# gresources.xml
gnome = import('gnome')
gresource_data = meson.project_source_root() / 'data'
gresource_xml = gresource_data / '@0@.gresource.xml'.format(app_id)

gresources = gnome.compile_resources('trg-resources', gresource_xml,
                                     source_dir:  gresource_data)
src_files += gresources


# Windows
if win32
  windows = import('windows')
  windres = windows.compile_resources('win32.rc')
endif

exe_kwargs = {}
if rss_dep.found()
  exe_kwargs = {'link_with': glib_rss_lib,
                'include_directories': rss_includes}
  src_files += ['trg-rss-cell-renderer.c',
                'trg-rss-model.c',
                'trg-rss-window.c']
endif

trg_bin = executable(project_name, 
                     sources: src_files,
                     dependencies: trg_deps,
                     install: true,
                     pie: true,
                     kwargs: exe_kwargs)

