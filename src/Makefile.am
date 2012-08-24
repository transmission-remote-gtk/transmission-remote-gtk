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

NULL =

public_icons_themes = \
        hicolor \
        $(NULL)

public_icons = \
        hicolor_apps_scalable_transmission-remote-gtk.svg \
	hicolor_apps_16x16_transmission-remote-gtk.png \
	hicolor_apps_24x24_transmission-remote-gtk.png \
	hicolor_apps_48x48_transmission-remote-gtk.png \
	hicolor_apps_22x22_transmission-remote-gtk.png \
	hicolor_apps_32x32_transmission-remote-gtk.png \
        $(NULL)

if !WIN32
EXTRA_DIST = transmission-remote-gtk.desktop.in transmission-remote-gtk.pod
CLEANFILES = transmission-remote-gtk.desktop transmission-remote-gtk.1

man_MANS = transmission-remote-gtk.1

desktopdir = $(datadir)/applications
desktop_DATA = transmission-remote-gtk.desktop

endif

bin_PROGRAMS = transmission-remote-gtk
INCLUDES = -std=c99 -Wall -I.. -Wno-overflow -DTRGLICENSE=\""$(trglicense)"\" $(libcurl_CFLAGS) $(jsonglib_CFLAGS) $(gthread_CFLAGS) $(gtk_CFLAGS) $(gio_CFLAGS) $(unique_CFLAGS) $(notify_CFLAGS) $(libproxy_CFLAGS) $(libappindicator_CFLAGS)
		
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
	  $(NULL)

transmission_remote_gtk_LDFLAGS = -lm $(jsonglib_LIBS) $(gtk_LIBS) $(gthread_LIBS) $(GEOIP_LIBS) $(gio_LIBS) $(unique_LIBS) $(notify_LIBS) $(libproxy_LIBS) $(libcurl_LIBS) $(libappindicator_LIBS)

if WIN32
.rc.o:
	windres $^ -o $@
%.o : %.rc
	windres $^ -o $@

CFLAGS += -mms-bitfields -mwin32 -mwindows
LDFLAGS += -Wl,--allow-multiple-definition -lws2_32 -lintl
transmission_remote_gtk_SOURCES += win32.rc win32-mailslot.c
INCLUDES += -O2
else
%.1: %.pod
	        pod2man --release="" --center="Transmission Remote GTK" $< > $@
endif

install-data-local: install-icons update-icon-cache

gtk_update_icon_cache = gtk-update-icon-cache -f -t

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		for theme in $(public_icons_themes); do \
			$(gtk_update_icon_cache) $(datadir)/icons/$$theme; \
	done; \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		for theme in $(public_icons_themes); do \
			echo "***   $(gtk_update_icon_cache) $(datadir)/icons/$$theme"; \
		done; \
	fi

install-icons:
	for icon in $(public_icons); do \
                THEME=`echo $$icon | cut -d_ -f1`; \
                CONTEXT=`echo $$icon | cut -d_ -f2`; \
                SIZE=`echo $$icon | cut -d_ -f3`; \
                ICONFILE=`echo $$icon | cut -d_ -f4`; \
                mkdir -p $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
                $(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
        done; \
        for icon in $(private_icons); do \
                THEME=`echo $$icon | cut -d_ -f1`; \
                CONTEXT=`echo $$icon | cut -d_ -f2`; \
                SIZE=`echo $$icon | cut -d_ -f3`; \
                ICONFILE=`echo $$icon | cut -d_ -f4`; \
                mkdir -p $(DESTDIR)$(pkgdatadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
                $(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(pkgdatadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
        done

transmission-remote-gtk.desktop: transmission-remote-gtk.desktop.in
	sed -e 's,@bindir\@,$(bindir),g' <transmission-remote-gtk.desktop.in > $@
