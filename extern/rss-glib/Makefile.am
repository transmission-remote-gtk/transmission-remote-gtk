
noinst_LTLIBRARIES = librss.la

librss_la_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"Rss\"

librss_la_CFLAGS = \
	$(TRG_CFLAGS) \
	$(MRSS_CFLAGS)

librss_la_LIBADD = \
	$(MRSS_LIBS)

librss_la_SOURCES = \
	rss-parser.c \
	rss-document.c \
	rss-item.c

noinst_HEADERS = \
	rss-document-private.h \
	rss-item-private.h \
	rss-parser-private.h \
	rss-parser.h \
	rss-document.h \
	rss-item.h \
	rss-version.h \
	rss-glib.h
        
