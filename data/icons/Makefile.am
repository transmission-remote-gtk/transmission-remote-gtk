
icondir = $(datadir)/icons
nobase_icon_DATA = \
	hicolor/16x16/apps/transmission-remote-gtk.png \
	hicolor/22x22/apps/transmission-remote-gtk.png \
	hicolor/24x24/apps/transmission-remote-gtk.png \
	hicolor/32x32/apps/transmission-remote-gtk.png \
	hicolor/48x48/apps/transmission-remote-gtk.png \
	hicolor/scalable/apps/transmission-remote-gtk.svg

EXTRA_DIST = $(nobase_icon_DATA)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

