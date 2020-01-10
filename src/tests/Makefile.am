##
# Global definitions
NULL =
INCLUDES =					\
	-I$(top_srcdir)/libgxim			\
	-I$(top_srcdir)				\
	$(GXIM_CFLAGS)				\
	$(CHECK_CFLAGS)				\
	$(NULL)
DEPS =						\
	$(top_builddir)/libgxim/libgxim.la	\
	$(NULL)
LIBS =						\
	@LDFLAGS@				\
	$(top_builddir)/libgxim/libgxim.la	\
	$(CHECK_LIBS)				\
	$(NULL)
EXTRA_DIST =					\
	$(NULL)
TESTS =						\
	$(NULL)
if ENABLE_UNIT_TEST
TESTS += $(test_programs)
endif

##
# Local definitions
common_private_headers =			\
	main.h					\
	$(NULL)
common_sources =				\
	main.c					\
	$(NULL)
test_programs =					\
	test-gximmessages			\
	test-gximmisc				\
	test-gximtransport			\
	$(NULL)

##
# Local Rules


##
# Target platform
noinst_HEADERS =				\
	$(common_private_headers)		\
	$(NULL)
noinst_PROGRAMS =				\
	$(NULL)
#	sample-server				\
#	sample-client				\
#
if ENABLE_UNIT_TEST
noinst_PROGRAMS += $(test_programs)
endif
#
#sample_server_SOURCES =				\
#	server.c				\
#	$(NULL)
#
#sample_client_SOURCES =				\
#	client.c				\
#	$(NULL)
#
test_gximmessages_SOURCES =			\
	gximmessages.c				\
	$(common_sources)			\
	$(NULL)
#
test_gximmisc_SOURCES =				\
	gximmisc.c				\
	$(common_sources)			\
	$(NULL)
#
test_gximtransport_SOURCES =			\
	gximtransport.c				\
	$(common_sources)			\
	$(NULL)

-include $(top_srcdir)/git.mk
