# New ports collection makefile for:	transmission-remote-gtk
# Date created:			22 February 2011
# Whom:				J.D. Morris <jdm720@gmail.com>
#
# $FreeBSD$
#

PORTNAME=	transmission-remote-gtk
PORTVERSION=	0.5
CATEGORIES=	net-p2p
MASTER_SITES=	${MASTER_SITE_GOOGLE_CODE}

MAINTAINER=	jdm720@gmail.com
COMMENT=	GTK app for remote management of Transmission via RPC

LIB_DEPENDS=	json-glib-1.0:${PORTSDIR}/devel/json-glib \
		notify.1:${PORTSDIR}/devel/libnotify \
		unique-1.0.2:${PORTSDIR}/x11-toolkits/unique \
		curl.6:${PORTSDIR}/ftp/curl

RUN_DEPENDS=	notification-daemon:${PORTSDIR}/deskutils/notification-daemon

USE_GNOME=		glib20 gtk20 gconf2
GNU_CONFIGURE=		yes
INSTALLS_ICONS=		yes
GCONF_SCHEMAS=		transmission-remote-gtk.schemas

.include <bsd.port.pre.mk>

.include <bsd.port.post.mk>
