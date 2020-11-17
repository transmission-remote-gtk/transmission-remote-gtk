#
# transmission-remote-gtk - A GTK RPC client to Transmission
# Copyright (C) 2011  Alan Fitton

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

bin_PROGRAMS = transmission-remote-gtk

transmission_remote_gtk_SOURCES = \
	  trg-cell-renderer-speed.c \
	  trg-cell-renderer-counter.c \
	  trg-cell-renderer-size.c \
	  trg-cell-renderer-ratio.c \
	  trg-cell-renderer-eta.c \
	  trg-cell-renderer-priority.c \
	  trg-cell-renderer-wanted.c \
	  trg-cell-renderer-file-icon.c \
	  trg-cell-renderer-epoch.c \
	  trg-cell-renderer-numgteqthan.c \
	  torrent-cell-renderer.c \
	  trg-remote-prefs-dialog.c \
	  trg-torrent-props-dialog.c \
	  trg-torrent-add-url-dialog.c \
	  trg-torrent-add-dialog.c \
	  trg-torrent-move-dialog.c \
	  trg-preferences-dialog.c \
	  trg-stats-dialog.c \
	  trg-about-window.c \
	  trg-destination-combo.c \
	  trg-state-selector.c \
	  trg-general-panel.c \
	  trg-torrent-graph.c \
	  trg-icons.c \
	  icons.c \
	  trg-toolbar.c \
	  trg-menu-bar.c \
	  trg-status-bar.c \
	  trg-file-parser.c \
	  trg-json-widgets.c \
	  trg-model.c \
	  trg-sortable-filtered-model.c \
	  trg-file-rename-dialog.c \
	  trg-files-tree.c \
	  trg-files-model.c \
	  trg-files-tree-view-common.c \
	  trg-files-tree-view.c \
	  trg-files-model-common.c \
	  trg-trackers-model.c \
	  trg-trackers-tree-view.c \
	  trg-peers-model.c \
	  trg-peers-tree-view.c \
	  trg-torrent-model.c \
	  trg-torrent-tree-view.c \
	  trg-persistent-tree-view.c \
	  trg-tree-view.c \
	  util.c \
	  hig.c \
	  bencode.c \
	  trg-prefs.c \
	  remote-exec.c \
	  trg-gtk-app.c \
	  requests.c \
	  torrent.c \
	  session-get.c \
	  json.c \
	  trg-client.c \
	  trg-main-window.c \
	  main.c \
	  upload.c

noinst_HEADERS = \
	  trg-cell-renderer-speed.h \
	  trg-cell-renderer-counter.h \
	  trg-cell-renderer-size.h \
	  trg-cell-renderer-ratio.h \
	  trg-cell-renderer-eta.h \
	  trg-cell-renderer-priority.h \
	  trg-cell-renderer-wanted.h \
	  trg-cell-renderer-file-icon.h \
	  trg-cell-renderer-epoch.h \
	  trg-cell-renderer-numgteqthan.h \
	  torrent-cell-renderer.h \
	  trg-remote-prefs-dialog.h \
	  trg-torrent-props-dialog.h \
	  trg-torrent-add-url-dialog.h \
	  trg-torrent-add-dialog.h \
	  trg-torrent-move-dialog.h \
	  trg-preferences-dialog.h \
	  trg-stats-dialog.h \
	  trg-about-window.h \
	  trg-destination-combo.h \
	  trg-state-selector.h \
	  trg-general-panel.h \
	  trg-torrent-graph.h \
	  trg-icons.h \
	  icons.h \
	  trg-toolbar.h \
	  trg-menu-bar.h \
	  trg-status-bar.h \
	  trg-file-parser.h \
	  trg-json-widgets.h \
	  trg-model.h \
	  trg-sortable-filtered-model.h \
	  trg-file-rename-dialog.h \
	  trg-files-tree.h \
	  trg-files-model.h \
	  trg-files-tree-view-common.h \
	  trg-files-tree-view.h \
	  trg-files-model-common.h \
	  trg-trackers-model.h \
	  trg-trackers-tree-view.h \
	  trg-peers-model.h \
	  trg-peers-tree-view.h \
	  trg-torrent-model.h \
	  trg-torrent-tree-view.h \
	  trg-persistent-tree-view.h \
	  trg-tree-view.h \
	  util.h \
	  hig.h \
	  bencode.h \
	  trg-prefs.h \
	  remote-exec.h \
	  trg-gtk-app.h \
	  requests.h \
	  torrent.h \
	  session-get.h \
	  json.h \
	  trg-client.h \
	  trg-main-window.h \
	  upload.h \
	  protocol-constants.h \
	  icon-turtle.h

transmission_remote_gtk_CPPFLAGS = \
	-DLOCALEDIR=\""$(localedir)"\" \
	-I$(top_srcdir)/extern

transmission_remote_gtk_CFLAGS = \
	$(TRG_CFLAGS) \
	$(GEOIP_CFLAGS) \
	$(NOTIFY_CFLAGS) \
	$(MRSS_CFLAGS) \
	$(APPINDICATOR_CFLAGS) \
	$(PROXY_CFLAGS)

transmission_remote_gtk_LDFLAGS = \
	$(LIBM) \
	$(TRG_LIBS) \
	$(GEOIP_LIBS) \
	$(NOTIFY_LIBS) \
	$(PROXY_LIBS) \
	$(APPINDICATOR_LIBS) \
	$(MRSS_CFLAGS)

if HAVE_RSS
transmission_remote_gtk_LDFLAGS += ${top_builddir}/extern/rss-glib/librss.la

transmission_remote_gtk_SOURCES += trg-rss-model.c trg-rss-window.c trg-rss-cell-renderer.c
noinst_HEADERS += trg-rss-model.h trg-rss-window.h trg-rss-cell-renderer.h
endif

if WIN32
.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@

transmission_remote_gtk_CFLAGS += -mms-bitfields -mwin32 -mwindows
transmission_remote_gtk_LDFLAGS += -Wl,--allow-multiple-definition -lws2_32 -lintl
transmission_remote_gtk_SOURCES += win32.rc win32-mailslot.c
else
%.1: %.pod
	        $(POD2MAN) --release="$(PACKAGE_VERSION)" --center="Transmission Remote GTK" $< > $@

EXTRA_DIST = transmission-remote-gtk.pod
CLEANFILES = transmission-remote-gtk.1

man_MANS = transmission-remote-gtk.1
endif

