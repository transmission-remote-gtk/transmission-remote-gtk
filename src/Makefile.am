EXTRA_DIST = transmission-remote-gtk.desktop.in
CLEANFILES = transmission-remote-gtk.desktop

desktopdir = $(datadir)/applications
desktop_DATA = transmission-remote-gtk.desktop

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = transmission-remote-gtk.schemas

bin_PROGRAMS = transmission-remote-gtk
INCLUDES = --pedantic -Wall -I.. -O2 $(jsonglib_CFLAGS) $(gthread_CFLAGS) $(gtk_CFLAGS) $(gconf_CFLAGS) $(gio_CFLAGS) $(unique_CFLAGS) $(notify_CFLAGS)

transmission_remote_gtk_SOURCES = main.c requests.c base64.c json.c http.c dispatch.c trg-main-window.c util.c trg-about-window.c torrent.c \
				tpeer.c tfile.c session-get.c trg-client.c trg-preferences-dialog.c hig.c trg-torrent-tree-view.c \
				trg-tree-view.c trg-torrent-model.c trg-peers-model.c trg-peers-tree-view.c trg-model.c trg-files-model.c \
				trg-files-tree-view.c trg-state-selector.c trg-general-panel.c trg-toolbar.c trg-menu-bar.c \
				trg-status-bar.c trg-trackers-tree-view.c trg-trackers-model.c trg-torrent-props-dialog.c \
				trg-json-widgets.c trg-torrent-add-url-dialog.c trg-cell-renderer-speed.c trg-cell-renderer-size.c \
				trg-cell-renderer-ratio.c trg-cell-renderer-eta.c trg-remote-prefs-dialog.c trg-cell-renderer-wanted.c \
				trg-cell-renderer-priority.c

transmission_remote_gtk_LDFLAGS = -lcurl $(jsonglib_LIBS) $(gtk_LIBS) $(gthread_LIBS) $(GEOIP_LIBS) $(gconf_LIBS) $(gio_LIBS) $(unique_LIBS) $(notify_LIBS)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(srcdir)/$(schema_DATA)

transmission-remote-gtk.desktop: transmission-remote-gtk.desktop.in
	sed -e 's,@bindir\@,$(bindir),g' $< > $@
