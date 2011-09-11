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

EXTRA_DIST = transmission-remote-gtk.desktop.in
CLEANFILES = transmission-remote-gtk.desktop

desktopdir = $(datadir)/applications
desktop_DATA = transmission-remote-gtk.desktop

bin_PROGRAMS = transmission-remote-gtk
INCLUDES = --pedantic -Wall -I.. -DTRGLICENSE=\""$(trglicense)"\" -DTRGLOCALEDIR=\""$(trglocaledir)"\" $(jsonglib_CFLAGS) $(gthread_CFLAGS) $(gtk_CFLAGS) $(gio_CFLAGS) $(unique_CFLAGS) $(notify_CFLAGS) $(libproxy_CFLAGS) -std=gnu99

transmission_remote_gtk_SOURCES = main.c \
	  requests.c \
	  base64.c \
	  json.c \
	  trg-main-window.c \
	  util.c \
	  trg-about-window.c \
	  torrent.c \
	  tpeer.c \
  	  tfile.c \
  	  dispatch.c \
	  session-get.c \
	  trg-client.c \
	  trg-preferences-dialog.c \
	  hig.c \
	  trg-torrent-tree-view.c \
	  trg-tree-view.c \
	  trg-torrent-model.c \
	  trg-peers-model.c \
	  trg-peers-tree-view.c \
	  trg-model.c \
	  trg-files-model.c \
	  trg-files-tree-view.c \
	  trg-state-selector.c \
	  trg-general-panel.c \
	  trg-toolbar.c \
	  trg-menu-bar.c \
	  trg-status-bar.c \
	  trg-trackers-tree-view.c \
	  trg-trackers-model.c \
	  trg-torrent-props-dialog.c \
	  trg-torrent-add-url-dialog.c \
	  trg-json-widgets.c \
	  trg-cell-renderer-speed.c \
	  trg-cell-renderer-size.c \
	  trg-cell-renderer-ratio.c \
	  trg-cell-renderer-eta.c \
	  trg-remote-prefs-dialog.c \
	  trg-cell-renderer-wanted.c \
	  trg-cell-renderer-priority.c \
	  trg-cell-renderer-epoch.c \
	  trg-cell-renderer-numgteqthan.c \
	  trg-torrent-move-dialog.c \
	  trg-stats-dialog.c \
	  trg-torrent-graph.c \
	  trg-torrent-add-dialog.c \
	  trg-file-parser.c \
	  bencode.c \
	  trg-prefs.c \
	  trg-destination-combo.c \
	  $(NULL)

transmission_remote_gtk_LDFLAGS = -lm -lcurl $(jsonglib_LIBS) $(gtk_LIBS) $(gthread_LIBS) $(GEOIP_LIBS) $(gio_LIBS) $(unique_LIBS) $(notify_LIBS) $(libproxy_LIBS)

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
