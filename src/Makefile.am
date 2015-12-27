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

if !WIN32
EXTRA_DIST = transmission-remote-gtk.desktop.in transmission-remote-gtk.pod transmission-remote-gtk.appdata.xml
CLEANFILES = transmission-remote-gtk.desktop transmission-remote-gtk.1

man_MANS = transmission-remote-gtk.1

desktopdir = $(datadir)/applications
desktop_DATA = transmission-remote-gtk.desktop

appdatadir = $(datadir)/appdata
appdata_DATA = transmission-remote-gtk.appdata.xml
endif

bin_PROGRAMS = transmission-remote-gtk
AM_CFLAGS = -DLOCALEDIR=\""$(localedir)"\" $(libcurl_CFLAGS) $(jsonglib_CFLAGS) $(gthread_CFLAGS) $(gtk_CFLAGS) $(gio_CFLAGS) $(notify_CFLAGS) $(libproxy_CFLAGS) $(libappindicator_CFLAGS) $(mrss_CFLAGS) -I$(top_srcdir)/extern
		
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
	  upload.c \
	  $(NULL)

transmission_remote_gtk_LDFLAGS = -lm $(jsonglib_LIBS) $(gtk_LIBS) $(gthread_LIBS) $(GEOIP_LIBS) $(gio_LIBS) $(notify_LIBS) $(libproxy_LIBS) $(libcurl_LIBS) $(libappindicator_LIBS)

if HAVE_RSS
transmission_remote_gtk_LDFLAGS += $(mrss_LIBS) ../extern/rss-glib/librss.la

transmission_remote_gtk_SOURCES += trg-rss-model.c trg-rss-window.c trg-rss-cell-renderer.c
endif

if WIN32
.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@

CFLAGS += -mms-bitfields -mwin32 -mwindows
LDFLAGS += -Wl,--allow-multiple-definition -lws2_32 -lintl
transmission_remote_gtk_SOURCES += win32.rc win32-mailslot.c
AM_CFLAGS += -O2
else
%.1: %.pod
	        pod2man --release="" --center="Transmission Remote GTK" $< > $@
endif


transmission-remote-gtk.desktop: transmission-remote-gtk.desktop.in
	sed -e 's,@bindir\@,$(bindir),g' <transmission-remote-gtk.desktop.in > $@
