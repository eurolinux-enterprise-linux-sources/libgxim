NULL =
AUTOMAKE_OPTIONS = dist-bzip2
SUBDIRS = libgxim m4macros po tests docs

ACLOCAL_AMFLAGS =			\
	-I m4macros			\
	$(NULL)

CONFIGURE_DEPENDENCIES =		\
	requires			\
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS =		\
	--enable-gtk-doc		\
	--disable-rebuilds		\
	$(NULL)

EXTRA_DIST =				\
	$(pkgconfig_DATA)		\
	intltool-extract.in		\
	intltool-merge.in		\
	intltool-update.in		\
	requires			\
	$(NULL)

MAINTAINERCLEANFILES =			\
	$(srcdir)/ChangeLog		\
	$(srcdir)/INSTALL		\
	$(srcdir)/aclocal.m4		\
	$(srcdir)/config.guess		\
	$(srcdir)/config.h.in		\
	$(srcdir)/config.sub		\
	$(srcdir)/depcomp		\
	$(srcdir)/gtk-doc.make		\
	$(srcdir)/install-sh		\
	$(srcdir)/ltmain.sh		\
	$(srcdir)/missing		\
	$(srcdir)/mkinstalldirs		\
	$(srcdir)/m4macros/gtk-doc.m4	\
	$(srcdir)/m4macros/intltool.m4	\
	$(srcdir)/m4macros/libtool.m4	\
	$(srcdir)/m4macros/lt*.m4	\
	`find "$(srcdir)" -type f -name Makefile.in -print`	\
	$(NULL)
DISTCLEANFILES =			\
	$(pkgconfig_DATA)		\
	$(noinst_DATA)			\
	$(NULL)

#
#
#
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =			\
	libgxim.pc			\
	$(NULL)
$(pkgconfig_DATA): $(top_builddir)/config.status
noinst_DATA =				\
	libgxim-uninstalled.pc		\
	$(NULL)
$(noinst_DATA): $(top_builddir)/config.status

-include $(top_srcdir)/git.mk
