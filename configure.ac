dnl
dnl transmission-remote-gtk - A GTK RPC client to Transmission
dnl Copyright (C) 2011  Alan Fitton

dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.

dnl You should have received a copy of the GNU General Public License along
dnl with this program; if not, write to the Free Software Foundation, Inc.,
dnl 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
dnl

AC_PREREQ([2.69])

AC_INIT([transmission-remote-gtk], [1.2], [alan@eth0.org.uk])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.12 tar-pax dist-xz no-dist-gzip subdir-objects no-define foreign -Wall -Wno-portability])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])

AX_IS_RELEASE([git-directory])
AX_CHECK_ENABLE_DEBUG([yes], [G_ENABLE_DEBUG], [DEBUG G_DISABLE_ASSERT G_DISABLE_CHECKS G_DISABLE_CAST_CHECKS])

AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL

LT_PREREQ([2.2.0])
LT_INIT
IT_PROG_INTLTOOL([0.50.1])
PKG_PROG_PKG_CONFIG([0.28])
DESKTOP_FILE

GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext package name])


case "${host_os}" in
        *mingw32*) WIN32="yes" ;;
        *cygwin*) WIN32="yes" ;;
        *freebsd*) WIN32="no";
        	CPPFLAGS="$CPPFLAGS -I/usr/local/include" ;;
        *) WIN32="no" ;;
esac

AM_CONDITIONAL([WIN32], test "$WIN32" = "yes")
#if WIN32
CFLAGS="$CFLAGS -IC:/MinGW/msys/1.0/include"
#endif

AC_ARG_WITH([libgeoip], AC_HELP_STRING([--without-libgeoip], [disable GeoIP support]))
have_libgeoip=no
AS_IF([test x$with_libgeoip != xno], [
	AC_CHECK_HEADER([GeoIP.h],[have_geoip=yes],[have_geoip=no])
])
AS_IF([test "x$have_geoip" = "xyes"], [
	AC_DEFINE(HAVE_GEOIP, 1, [GeoIP Support.])
	GEOIP_LIBS="-lGeoIP"
	AC_SUBST([GEOIP_LIBS])
])
AM_CONDITIONAL([HAVE_GEOIP], [test "x$have_geoip" = "xyes"])

AC_ARG_WITH([libnotify], AC_HELP_STRING([--without-libnotify], [disable libnotify]))
have_libnotify=no
AC_ARG_WITH([libmrss], AC_HELP_STRING([--without-libmrss], [disable RSS support]))
have_libmrss=no
AC_ARG_WITH([libproxy], AC_HELP_STRING([--without-libproxy], [disable libproxy]))
have_libproxy=no
AC_ARG_WITH([libappindicator], AC_HELP_STRING([--without-libappindicator], [disable libappindicator]))
have_libappindicator=no

AS_IF([test x$with_libnotify != xno], [
	PKG_CHECK_MODULES([notify], [libnotify], AC_DEFINE(HAVE_LIBNOTIFY, 1, [Define if libnotify is available]), AC_MSG_WARN([libnotify is required for popup desktop notifications]))
])

#if test x$with_librssglib != xno; then
#  PKG_CHECK_MODULES([rssglib], [rss-glib-1.0], AC_DEFINE(HAVE_RSSGLIB, 1, [Define if rss-glib is available]), AC_MSG_WARN([rss-glib is required for RSS viewer functionality]))
#fi
#
AS_IF([test x$with_libproxy != xno], [
	PKG_CHECK_MODULES([libproxy], [libproxy-1.0], AC_DEFINE(HAVE_LIBPROXY, 1, [Define if libproxy is available]), AC_MSG_WARN([libproxy is required for HTTP proxy support]))
])

AS_IF([test x$with_libmrss != xno], [
	PKG_CHECK_MODULES([mrss], [mrss >= 0.18], [ have_libmrss="yes" ], [ have_libmrss="no" ])
])

AS_IF([test x$have_libmrss == xyes], [
	AC_DEFINE(HAVE_RSS, 1, [Define if RSS features available])
], [
	AC_MSG_WARN([libmrss is required for RSS reader])
])

AM_CONDITIONAL([HAVE_RSS], [test x$have_libmrss == xyes ])

AC_DEFINE([GLIB_VERSION_MIN_REQUIRED], [GLIB_VERSION_2_44], [Dont warn using older APIs])
AC_DEFINE([GLIB_VERSION_MAX_ALLOWED], [GLIB_VERSION_2_44], [Prevents using newer APIs])

PKG_CHECK_MODULES([jsonglib], [json-glib-1.0 >= 0.8])
PKG_CHECK_MODULES([gthread], [gthread-2.0])
PKG_CHECK_MODULES([libcurl], [libcurl])
PKG_CHECK_MODULES([gio], [gio-2.0 >= 2.44])
PKG_CHECK_MODULES([gtk], [gtk+-3.0 >= 3.16], [
	AS_IF([test x$with_libappindicator != xno], [
		PKG_CHECK_MODULES([libappindicator], [appindicator3-0.1],
			AC_DEFINE(HAVE_LIBAPPINDICATOR, 1, [Define if libappindicator is available]), AC_MSG_WARN([Ubuntu Unity users should consider building with libappindicator]))
	])
])

AX_APPEND_COMPILE_FLAGS([ \
	-std=gnu99 \
	-funsigned-char \
	-fstack-protector-strong \
	-fPIE \
	-fPIC \
	-Wall \
	-Wextra \
	-Winline \
	-Wno-padded \
	-Wno-unused-parameter \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Werror=format=2 \
	-Werror=implicit-function-declaration \
	-Werror=pointer-arith \
	-Werror=init-self \
	-Werror=format-security \
	-Werror=missing-include-dirs \
	-Werror=date-time \
	-Wno-conversion \
	-Wno-sign-compare \
	-Wno-deprecated-declarations \
])
dnl TODO: Fix last two hidden warnings


AC_CONFIG_FILES([
	Makefile
	po/Makefile.in
	src/Makefile
	extern/Makefile
	extern/rss-glib/Makefile
	data/Makefile
	data/icons/Makefile
])

AC_OUTPUT

echo "
	$PACKAGE $VERSION

	prefix ........: $prefix
	debug .........: $enable_debug

	geoip .........: $have_libgeoip
	libnotify .....: $have_libnotify
	libmrss .......: $have_libmrss
	libproxy ......: $have_libproxy
	libappindicator: $have_libappindicator
"
