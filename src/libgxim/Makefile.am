##
# Global definitions
NULL =
INCLUDES =							\
	$(GXIM_CFLAGS)						\
	$(NULL)
LIBS =								\
	@LDFLAGS@						\
	$(GXIM_LIBS)						\
	$(NULL)
EXTRA_DIST =							\
	marshal.list						\
	mkacc.rb						\
	$(NULL)
MAINTAINERCLEANFILES =						\
	$(gxim_built_public_headers)				\
	$(gxim_built_private_headers)				\
	$(gxim_built_sources)					\
	$(stamp_files)						\
	$(NULL)
CLEANFILES =							\
	xgen-gxh						\
	xgen-gxc						\
	$(NULL)


##
# Local definitions
gxim_public_headers =						\
	gximattr.h						\
	gximclconn.h						\
	gximcltmpl.h						\
	gximconnection.h					\
	gximcore.h						\
	gximerror.h						\
	gximmessage.h						\
	gximmisc.h						\
	gximprotocol.h						\
	gximsrvconn.h						\
	gximsrvtmpl.h						\
	gximtransport.h						\
	gximtypes.h						\
	libgxim.h						\
	$(NULL)
gxim_private_headers =						\
	gximprotocol10.h					\
	$(NULL)
gxim_built_public_headers =					\
	$(NULL)
gxim_built_private_headers =					\
	gximacc.h						\
	gximmarshal.h						\
	$(NULL)
#
gxim_built_sources =						\
	gximacc.c						\
	gximmarshal.c						\
	$(NULL)
gxim_sources =							\
	gximattr.c						\
	gximclconn.c						\
	gximcltmpl.c						\
	gximconnection.c					\
	gximcore.c						\
	gximerror.c						\
	gximmessage.c						\
	gximmisc.c						\
	gximprotocol.c						\
	gximprotocol10.c					\
	gximsrvconn.c						\
	gximsrvtmpl.c						\
	gximtransport.c						\
	$(gxim_built_sources)					\
	$(NULL)
#
stamp_files =							\
	stamp-gximacc.h						\
	stamp-gximmarshal.h					\
	$(NULL)


##
# Local Rules
gximcore.c: gximmarshal.h gximacc.h
	@true
gximcltmpl.c: gximacc.h gximmarshal.h
	@true
gximmarshal.h: stamp-gximmarshal.h
	@true
stamp-gximmarshal.h: @REBUILD@ marshal.list Makefile
	$(GLIB_GENMARSHAL) --prefix=gxim_marshal $(srcdir)/marshal.list --header > xgen-gxh \
	&& (cmp -s xgen-gxh gximmarshal.h || cp xgen-gxh gximmarshal.h) \
	&& rm -f xgen-gxh \
	&& echo timestamp > $(@F)
gximmarshal.c: @REBUILD@ gximmarshal.h Makefile
	(echo "#include \"gximmarshal.h\""; \
	$(GLIB_GENMARSHAL) --prefix=gxim_marshal $(srcdir)/marshal.list --body) > xgen-gxc \
	&& cp xgen-gxc gximmarshal.c \
	&& rm -f xgen-gxc
gximacc.h: stamp-gximacc.h
	@true
stamp-gximacc.h: @REBUILD@ marshal.list mkacc.rb Makefile
	./mkacc.rb --prefix=_gxim_acc --mode=header $(srcdir)/marshal.list > xgen-gxah \
	&& (cmp -s xgen-gxah gximacc.h || cp xgen-gxah gximacc.h) \
	&& rm -f xgen-gxah \
	&& echo timestamp > $(@F)
gximacc.c: @REBUILD@ gximacc.h mkacc.rb Makefile
	(echo "#include \"gximacc.h\""; \
	./mkacc.rb --prefix=_gxim_acc --mode=body $(srcdir)/marshal.list) > xgen-gxac \
	&& cp xgen-gxac gximacc.c \
	&& rm -f xgen-gxac

##
# Target platform
lib_LTLIBRARIES =						\
	libgxim.la						\
	$(NULL)
#
gximincludedir = $(includedir)/libgxim
gximinclude_HEADERS =						\
	$(gxim_public_headers)					\
	$(gxim_built_public_headers)				\
	$(NULL)
noinst_HEADERS =						\
	$(gxim_private_headers)					\
	$(gxim_built_private_headers)				\
	$(NULL)
#
libgxim_la_SOURCES = $(gxim_sources)
libgxim_la_CFLAGS =						\
	-DG_LOG_DOMAIN="\"GXIM\""				\
	$(NULL)
libgxim_la_LDFLAGS =						\
	$(LDFLAGS)						\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)	\
	$(NULL)
