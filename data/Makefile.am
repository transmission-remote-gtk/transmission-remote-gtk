SUBDIRS = icons

desktop_in_files = transmission-remote-gtk.desktop.in
desktop_FILES = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
@DESKTOP_FILE_RULES@

EXTRA_DIST = $(desktop_in_files)
