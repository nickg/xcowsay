AC_INIT([xcowsay], [1.1],
        [Nick Gasson <nick@nick.me.uk>],
        [xcowsay])
AM_INIT_AUTOMAKE([-Wall])
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

AM_GNU_GETTEXT([external])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strtol])

# Check for pkg-config packages
modules="gtk+-2.0"
xcowsayd_modules="$modules dbus-glib-1 gthread-2.0"
AC_ARG_ENABLE(dbus,
        [AS_HELP_STRING([--disable-dbus], [Do not build DBus daemon.])],
        [if test "$enableval" != "no" ; then
           modules="$xcowsayd_modules"
         fi
         CFLAGS="$CFLAGS -DWITHOUT_DBUS"],
        [modules="$xcowsayd_modules"])

PKG_CHECK_MODULES(XCOWSAY, $modules)

AC_CONFIG_HEADERS([config.h])
AC_OUTPUT([Makefile debian/Makefile src/Makefile po/Makefile.in])
