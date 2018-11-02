SUBDIRS = icons

%.desktop: %.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

desktop_in_files = io.github.TransmissionRemoteGtk.desktop.in
desktop_FILES = $(desktop_in_files:.desktop.in=.desktop)
@DESKTOP_FILE_RULES@

%.appdata.xml: %.appdata.xml.in
	$(AM_V_GEN)$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

appstream_in_files = io.github.TransmissionRemoteGtk.appdata.xml.in
appstream_XML = $(appstream_in_files:.xml.in=.xml)
@APPSTREAM_XML_RULES@

EXTRA_DIST = $(desktop_in_files) $(appstream_in_files)

CLEANFILES = $(desktop_FILES) $(appstream_XML)
