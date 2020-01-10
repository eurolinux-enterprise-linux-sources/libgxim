dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.62)
AC_INIT([libgxim], 0.5.0, [http://code.google.com/p/libgxim/issues/entry])

. `dirname $0`/requires

AM_INIT_AUTOMAKE([1.11 -Wno-portability])
dnl silent build rules, requires at least automake-1.11.
dnl by either passing --enable-silent-rules to configure or
dnl V=0 to make.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE
AM_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4macros])

AC_PROG_LIBTOOL
AC_PROG_INTLTOOL

AC_PROG_CC

GNOME_COMPILE_WARNINGS

GNOME_COMMON_INIT
GNOME_DEBUG_CHECK
GNOME_MAINTAINER_MODE_DEFINES
GTK_DOC_CHECK([$GTKDOC_REQUIRED])

CFLAGS="$CFLAGS $WARN_CFLAGS"

dnl ======================================================================
dnl Libraries versioning
dnl ======================================================================
dnl Quote from Updating library version information at libtool.info
dnl and renumbering
dnl
dnl 1. Update the version information only immediately before a public
dnl    release of your software.  More frequent updates are unnecessary,
dnl    and only guarantee that the current interface number gets larger
dnl    faster.
dnl 2. If the library source code has changed at all since the last
dnl    update, then increment REVISION (`C:R:A' becomes `C:r+1:A')
dnl 3. If any interfaces have been added, removed, or changed since the
dnl    last update, increment CURRENT, and set REVISION to 0.
dnl 4. If any interfaces have been added since the last public release,
dnl    then increment AGE.
dnl 5. If any interfaces have been removed since the last public release,
dnl    then set AGE to 0.
dnl
LT_CURRENT=4
LT_REVISION=0
LT_AGE=0

AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

dnl ======================================================================
dnl define variables
dnl ======================================================================

dnl ======================================================================
dnl functionalities testing
dnl ======================================================================
AX_CFLAGS_GCC_OPTION([-fno-strict-aliasing])
if test "x$enable_debug"; then
	AX_CFLAGS_GCC_OPTION([-fdiagnostics-show-option])
fi

dnl ======================================================================
dnl functions testing
dnl ======================================================================

dnl ======================================================================
dnl gettext stuff
dnl ======================================================================
GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT

dnl ======================================================================
dnl check pkg-config stuff
dnl ======================================================================
PKG_CHECK_MODULES(GXIM, gobject-2.0 >= $GOBJECT_REQUIRED
			gthread-2.0 >= $GTHREAD_REQUIRED
			gio-2.0 >= $GIO_REQUIRED
			gdk-2.0 >= $GDK_REQUIRED
			x11)
PKG_CHECK_MODULES(CHECK, check >= $CHECK_REQUIRED,
	[use_check="yes"],
	[use_check="no"])

AC_SUBST(GXIM_CFLAGS)
AC_SUBST(GXIM_LIBS)
AC_SUBST(CHECK_CFLAGS)
AC_SUBST(CHECK_LIBS)

AM_CONDITIONAL(ENABLE_UNIT_TEST, test x$use_check != xno)

dnl ======================================================================
dnl check another libraries
dnl ======================================================================
AM_PATH_GLIB_2_0($GLIB_REQUIRED, :,
	AC_MSG_ERROR([
*** GLIB $GLIB_REQUIRED_VERSION or better is required. The latest version of
*** GLIB is always available from ftp://ftp.gtk.org/.]),
    glib)

dnl ======================================================================
dnl options
dnl ======================================================================
AC_ARG_ENABLE(rebuilds,
	[AC_HELP_STRING([--disable-rebuilds],
		[disable all source autogeneration rules])],,
	[enable_rebuilds=yes])

dnl ======================================================================
dnl options - rebuilds
dnl ======================================================================
REBUILD=\#
if test "x$enable_rebuilds" = "xyes"; then
	REBUILD=
fi
AC_SUBST(REBUILD)

dnl ======================================================================
dnl output
dnl ======================================================================
AC_CONFIG_FILES([
	Makefile
	docs/Makefile
	docs/reference/Makefile
	docs/reference/version.xml
	libgxim.pc
	libgxim-uninstalled.pc
	libgxim/Makefile
	m4macros/Makefile
	po/Makefile.in
	tests/Makefile
])
AC_OUTPUT

dnl ======================================================================
dnl result
dnl ======================================================================
echo ""
echo "========== Build Information =========="
echo " CFLAGS:                 $CFLAGS $GXIM_CFLAGS"
echo " LDFLAGS:                $LDFLAGS $GXIM_LIBS"
echo " Unit testing:           $use_check"
echo ""
