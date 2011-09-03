AC_PREREQ(2.63)

AC_INIT(transmission-remote-gtk, 0.5.2, alan@eth0.org.uk)
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

AC_CHECK_HEADERS([stdlib.h string.h])
AC_TYPE_SIZE_T

OSNAME=`uname -s`
case "$OSNAME" in
        FreeBSD*)
                CPPFLAGS="$CPPFLAGS -I/usr/local/include"
        ;;
esac

trglocaledir='${prefix}/${DATADIRNAME}/locale'
trglicense='${docdir}-$(PACKAGE_VERSION)/COPYING'
AC_SUBST(trglocaledir)
AC_SUBST(trglicense)
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE(GETTEXT_PACKAGE, "AC_PACKAGE_NAME", [foo])

PKG_CHECK_MODULES([jsonglib], [json-glib-1.0 >= 0.8])
PKG_CHECK_MODULES([gthread], [gthread-2.0])
PKG_CHECK_MODULES([gtk], [gtk+-2.0 >= 2.16])
PKG_CHECK_MODULES([gio], [gio-2.0 >= 2.22])
PKG_CHECK_MODULES([unique], [unique-1.0], AC_DEFINE(HAVE_LIBUNIQUE, 1, [Define if libunique is available]))
PKG_CHECK_MODULES([notify], [libnotify])
PKG_CHECK_MODULES([libproxy], [libproxy-1.0], AC_DEFINE(HAVE_LIBPROXY, 1, [Define if libproxy is available]), AC_MSG_WARN([libproxy is required for HTTP proxy support]))
LIBCURL_CHECK_CONFIG([yes], [7.0], [], [AC_MSG_ERROR(["libcurl not found"])])

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

AC_ARG_ENABLE(debug, [AS_HELP_STRING(--enable-debug,
                                     [enable debugging])])
if test x$enable_debug = xyes; then
    AC_DEFINE([DEBUG], [],  [enable debugging])
fi

AC_OUTPUT([Makefile po/Makefile.in src/Makefile])
