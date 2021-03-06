#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libdescriptor, 0.1, voc@users.sourceforge.net)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(vdccm, 0.10.1)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_DISABLE_STATIC
AM_PROG_LIBTOOL

dnl Options.
AC_ARG_ENABLE(desktop-integration,
              [  --enable-desktop-integration  enable desktop integration (requires D-Bus and HAL)],
              enable_desktop_integration=$enableval, enable_desktop_integration=auto)

dnl Check for libsynce
PKG_CHECK_MODULES(LIBSYNCE, libsynce >= 0.10.0)

if ! test "$enable_desktop_integration" = "no"; then
  dnl Check for GLib
  PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.4, gobject-2.0 >= 2.4, gthread-2.0 >= 2.4],
                    glib=yes, glib=no)

  dnl Check for D-Bus
  PKG_CHECK_MODULES(DBUS, [dbus-1 >= 0.61, dbus-glib-1 >= 0.61],
                    dbus=yes, dbus=no)

  dnl Check for HAL
  PKG_CHECK_MODULES(HAL, hal, hal=yes, hal=no)

  if test "$glib" = "no" || test "$dbus" = "no" || test "$hal" = "no"; then
    if test "$enable_desktop_integration" = "auto"; then
      AC_MSG_NOTICE([GLib/D-Bus/HAL not found, building without desktop integration])
      enable_desktop_integration=no
    else
      AC_MSG_ERROR([desktop integration requested but one of GLib/D-Bus/HAL could not be found])
    fi
  else
    AC_MSG_NOTICE([Building with desktop integration])
    AC_DEFINE(ENABLE_DESKTOP_INTEGRATION, , [Define if building with desktop integration])
    enable_desktop_integration=yes
  fi
else
  AC_MSG_NOTICE([building without desktop integration])
fi

AM_CONDITIONAL(ENABLE_DESKTOP_INTEGRATION, test "$enable_desktop_integration" = "yes")

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([gethostbyaddr gethostbyname gettimeofday select socket strerror])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 src/Makefile
                 tools/Makefile])
AC_OUTPUT
