include_HEADERS = \
	mpib_coll.h \
	mpib_basic_collectives.h \
	mpib_tree_collectives.h

lib_LTLIBRARIES = libmpib_coll.la
libmpib_coll_la_SOURCES = \
	libmpib_coll.c \
	libmpib_coll.h \
	sg_flat.c \
	sgv_flat.h \
	sgv_flat.c \
	comm_tree.hpp \
	tree_algorithms.hpp \
	br_tree_algorithms.hpp \
	sg_tree_algorithms.hpp \
	sgv_tree_algorithms.hpp \
	brsg_tree_builders.hpp \
	sgv_tree_builders.hpp \
	mpib_tree_collectives.cpp

AM_CPPFLAGS = -I$(top_srcdir)
libmpib_coll_la_LIBADD = \
	$(top_builddir)/benchmarks/libmpib.a \
	$(top_builddir)/p2p/libmpib_p2p.a

EXTRA_DIST = \
	tree_collectives.dot
