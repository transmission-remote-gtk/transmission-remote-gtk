#AC_PREREQ([2.65])
AC_INIT(transmission-remote-gtk, 0.1, alan@eth0.org.uk)
AM_INIT_AUTOMAKE(transmission-remote-gtk, 0.1.0)
AC_OUTPUT(Makefile src/Makefile)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

# Checks for header files.

AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for programs.
PKG_CHECK_MODULES([jsonglib], [json-glib-1.0])
PKG_CHECK_MODULES([gthread], [gthread-2.0])
PKG_CHECK_MODULES([gconf], [gconf-2.0])
PKG_CHECK_MODULES([gtk], [gtk+-2.0])
PKG_CHECK_MODULES([gio], [gio-2.0])
PKG_CHECK_MODULES([unique], [unique-1.0])
PKG_CHECK_MODULES([notify], [libnotify])
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

AC_TYPE_SIZE_T

AM_GCONF_SOURCE_2

AC_OUTPUT
