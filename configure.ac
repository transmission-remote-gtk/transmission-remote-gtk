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

AC_PREREQ(2.63)

AC_INIT(transmission-remote-gtk, 1.1.1, alan@eth0.org.uk)
AC_CONFIG_SRCDIR(src)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC
AM_PROG_CC_C_O

AC_PROG_INSTALL
LT_INIT
IT_PROG_INTLTOOL([0.35.0])
PKG_PROG_PKG_CONFIG

AC_CHECK_HEADERS([stdlib.h string.h])
AC_TYPE_SIZE_T

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

trglicense='${docdir}-$(PACKAGE_VERSION)/COPYING'
AC_SUBST(trglicense)
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE(GETTEXT_PACKAGE, "AC_PACKAGE_NAME", [foo])
AC_DEFINE_DIR([LOCALEDIR], [datarootdir/locale], [gettext catalogs])

AC_ARG_WITH([libgeoip], AC_HELP_STRING([--without-libgeoip], [disable GeoIP support]))
have_libgeoip=no
if test x$with_libgeoip != xno; then
  AC_CHECK_HEADER([GeoIP.h],[have_geoip=yes],[have_geoip=no])
fi
if test "x$have_geoip" = "xyes"; then
  AC_DEFINE(HAVE_GEOIP, 1, [GeoIP Support.])
  GEOIP_LIBS="-lGeoIP"
  AC_SUBST([GEOIP_LIBS])
fi
AM_CONDITIONAL([HAVE_GEOIP], [test "x$have_geoip" = "xyes"])

AC_ARG_WITH([libunique], AC_HELP_STRING([--without-libunique], [disable libunique]))
have_libunique=no
AC_ARG_WITH([libnotify], AC_HELP_STRING([--without-libnotify], [disable libnotify]))
have_libnotify=no
AC_ARG_WITH([libproxy], AC_HELP_STRING([--without-libproxy], [disable libproxy]))
have_libproxy=no
AC_ARG_WITH([libappindicator], AC_HELP_STRING([--without-libappindicator], [disable libappindicator]))
have_libappindicator=no

if test x$with_libnotify != xno; then
  PKG_CHECK_MODULES([notify], [libnotify], AC_DEFINE(HAVE_LIBNOTIFY, 1, [Define if libnotify is available]), AC_MSG_WARN([libnotify is required for popup desktop notifications]))
fi

if test x$with_libproxy != xno; then
  PKG_CHECK_MODULES([libproxy], [libproxy-1.0], AC_DEFINE(HAVE_LIBPROXY, 1, [Define if libproxy is available]), AC_MSG_WARN([libproxy is required for HTTP proxy support]))
fi

AC_ARG_ENABLE(debug, [AS_HELP_STRING(--enable-debug,
                                     [enable debugging])])
if test x$enable_debug = xyes; then
    AC_DEFINE([DEBUG], [],  [enable debugging])
fi

AC_ARG_ENABLE(gtk3, [AS_HELP_STRING(--enable-gtk3,
                                     [use gtk3 support (now default, left for compatability)])])
AC_ARG_ENABLE(gtk2, [AS_HELP_STRING(--enable-gtk2,
                                     [use old gtk2 support])])

PKG_CHECK_MODULES([jsonglib], [json-glib-1.0 >= 0.8])
PKG_CHECK_MODULES([gthread], [gthread-2.0])

AC_DEFUN([TRG_GTK3_CONFIGURE], [
AC_MSG_NOTICE([attempting gtk-3.0 build])
PKG_CHECK_MODULES([gtk], [gtk+-3.0 >= 3.00], [
if test x$with_libappindicator != xno; then
PKG_CHECK_MODULES([libappindicator], [appindicator3-0.1], AC_DEFINE(HAVE_LIBAPPINDICATOR, 1, [Define if libappindicator is available]), AC_MSG_WARN([Ubuntu Unity users should consider building with libappindicator]))
fi
], [$1])
])

AC_DEFUN([TRG_GTK2_CONFIGURE], [
AC_MSG_NOTICE([attempting gtk-2.0 build])
PKG_CHECK_MODULES([gtk], [gtk+-2.0 >= 2.16])
if test x$with_libappindicator != xno; then
  PKG_CHECK_MODULES([libappindicator], [appindicator-0.1], AC_DEFINE(HAVE_LIBAPPINDICATOR, 1, [Define if libappindicator is available]), AC_MSG_WARN([Ubuntu Unity users should consider building with libappindicator]))
fi
if test x$with_libunique != xno; then
  PKG_CHECK_MODULES([unique], [unique-1.0], AC_DEFINE(HAVE_LIBUNIQUE, 1, [Define if libunique is available]), AC_MSG_WARN([libunique is required for opening torrents on gtk+-2.0. not needed for gtk+-3.0 or win32. ]))
fi
])

if test x$enable_gtk2 = xyes; then
  TRG_GTK2_CONFIGURE([])
else
  TRG_GTK3_CONFIGURE()
fi

PKG_CHECK_MODULES([libcurl], [libcurl])
PKG_CHECK_MODULES([gio], [gio-2.0 >= 2.22])

AC_OUTPUT([Makefile po/Makefile.in src/Makefile])
