AC_INIT([xcowsay], [1.4],
        [Nick Gasson <nick@nick.me.uk>],
        [xcowsay])
AM_INIT_AUTOMAKE([-Wall])

# Check for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_LN_S
AC_PROG_CPP
AC_PROG_MKDIR_P

# Checks for libraries.

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h libintl.h locale.h stdlib.h string.h limits.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strtol setlocale strcasecmp strdup strchr \
                getcwd strerror realpath])

# Check for pkg-config packages
modules="gtk+-2.0 gdk-pixbuf-2.0"
xcowsayd_modules="$modules dbus-glib-1 gthread-2.0"
AC_ARG_ENABLE(dbus,
        [AS_HELP_STRING([--enable-dbus], [Build the DBus daemon.])],
        [if test "$enableval" = "yes" ; then
           modules="$xcowsayd_modules"
           CFLAGS="$CFLAGS -DWITH_DBUS"
         fi
	 AC_FUNC_FORK])

PKG_CHECK_MODULES(XCOWSAY, $modules)

# Not sure why autoconf doesn't define this itself
pkgdatadir=$datadir/xcowsay

AX_DEFINE_DIR([DATADIR], [pkgdatadir], [Location of data files])
AX_DEFINE_DIR([LOCALEDIR], [localedir], [Location of locale files])

AC_CONFIG_HEADERS([config.h])
AC_OUTPUT([Makefile src/Makefile po/Makefile.in])
