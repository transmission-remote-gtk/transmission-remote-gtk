NULL =

AM_CFLAGS = \
	-I. 		\
	-DPREFIX=\""$(prefix)"\" 	\
	-DLIBDIR=\""$(libdir)"\" 	\
	-DG_LOG_DOMAIN=\"Rss\" 		\
        $(gtk_CFLAGS) \
	$(MRSS_CFLAGS)			\
	$(NULL)

source_h = \
	rss-parser.h \
	rss-document.h \
	rss-item.h \
	rss-version.h \
	$(NULL)

source_h_private = \
	rss-document-private.h \
	rss-item-private.h \
	rss-parser-private.h \
	$(NULL)

source_c = \
	rss-parser.c \
	rss-document.c \
	rss-item.c \
	$(NULL)

noinst_LTLIBRARIES = librss.la

librss_la_LIBADD = $(MRSS_LIBS)
librss_la_SOURCES = \
	$(source_c) \
	$(source_h) \
	$(source_h_private) \
	$(NULL)

EXTRA_DIST = \
        rss-glib.h

librss_la_LDFLAGS = $(LDADD)
