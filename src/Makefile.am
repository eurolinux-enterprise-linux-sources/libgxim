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
