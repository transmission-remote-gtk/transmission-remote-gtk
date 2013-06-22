# New ports collection makefile for:	transmission-remote-gtk
# Date created:			22 February 2011
# Whom:				J.D. Morris <jdm720@gmail.com>
#
# $FreeBSD$
#

PORTNAME=	transmission-remote-gtk
PORTVERSION=	1.1.1
CATEGORIES=	net-p2p
MASTER_SITES=	${MASTER_SITE_GOOGLE_CODE}

MAINTAINER=	jdm720@gmail.com
COMMENT=	GTK app for remote management of Transmission via RPC

LIB_DEPENDS=	json-glib-1.0:${PORTSDIR}/devel/json-glib \
		unique-1.0.2:${PORTSDIR}/x11-toolkits/unique \
		curl:${PORTSDIR}/ftp/curl

BUILD_DEPENDS=	intltool-update:${PORTSDIR}/textproc/intltool \
		p5-XML-Parser>=0:${PORTSDIR}/textproc/p5-XML-Parser

RUN_DEPENDS=	${BUILD_DEPENDS}

USE_GNOME=		glib20 gconf2
INSTALLS_ICONS=		yes
USE_GMAKE=		yes
GNU_CONFIGURE=		yes
CONFIGURE_ARGS+=	--without-libappindicator

OPTIONS_SINGLE=		GTK
OPTIONS_SINGLE_GTK=	GTK2 GTK3
OPTIONS_DEFINE=		NLS GEOIP LIBNOTIFY LIBPROXY
OPTIONS_DEFAULT=	GTK3 NLS LIBNOTIFY

LIBNOTIFY_DESC=	Desktop Notifications framework (devel/libnotify)

.include <bsd.port.options.mk>

.if ${PORT_OPTIONS:MGTK2}
USE_GNOME+=	gtk20
CONFIGURE_ARGS+=--enable-gtk2
.else
USE_GNOME+=	gtk30
.endif

.if ${PORT_OPTIONS:MNLS}
USES+=		gettext
CONFIGURE_ENV+=	DATADIRNAME=share
PLIST_SUB+=	NLS=""
.else
CONFIGURE_ARGS+=--disable-nls
PLIST_SUB=	NLS="@comment "
.endif

.if ${PORT_OPTIONS:MGEOIP}
LIB_DEPENDS+=	GeoIP:${PORTSDIR}/net/GeoIP
.else
CONFIGURE_ARGS+=--without-libgeoip
.endif

.if ${PORT_OPTIONS:MLIBNOTIFY}
LIB_DEPENDS+=	notify:${PORTSDIR}/devel/libnotify
.else
CONFIGURE_ARGS+=--without-libnotify
.endif

.if ${PORT_OPTIONS:MLIBPROXY}
LIB_DEPENDS+=	libproxy:${PORTSDIR}/net/libproxy
.else
CONFIGURE_ARGS+=--without-libproxy
.endif

MAN1=		transmission-remote-gtk.1

.include <bsd.port.mk>
