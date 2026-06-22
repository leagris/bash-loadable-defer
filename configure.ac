#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Copyright (C) 2026 Léa Gris <lea.gris@noiraude.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

AC_PREREQ([2.69])
AC_INIT([bash-loadable-defer], [1.0])
AC_CONFIG_SRCDIR([defer.c])

AC_PROG_CC

# Discover the bash loadable build settings through pkg-config (bash.pc).
AC_CHECK_PROG([PKG_CONFIG], [pkg-config], [pkg-config])
AS_IF([test -z "$PKG_CONFIG"],
  [AC_MSG_ERROR([pkg-config is required to locate the bash loadable headers])])

AC_MSG_CHECKING([for the bash loadables directory])
loadablesdir=`$PKG_CONFIG --variable=loadablesdir bash 2>/dev/null`
AS_IF([test -z "$loadablesdir"],
  [AC_MSG_ERROR([cannot find bash.pc; install the bash loadable development files])])
AC_MSG_RESULT([$loadablesdir])
AC_SUBST([loadablesdir])

AC_CHECK_FILE([$loadablesdir/Makefile.inc], [],
  [AC_MSG_ERROR([$loadablesdir/Makefile.inc is missing])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
