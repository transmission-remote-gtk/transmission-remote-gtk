SUBDIRS = icons

desktop_in_files = transmission-remote-gtk.desktop.in
desktop_FILES = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
@DESKTOP_FILE_RULES@

appstream_in_files = transmission-remote-gtk.appdata.xml.in
appstream_XML = $(appstream_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@
@APPSTREAM_XML_RULES@

EXTRA_DIST = $(desktop_in_files) $(appstream_in_files)

DISTCLEANFILES = $(desktop_FILES) $(appstream_XML)
