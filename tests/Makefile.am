noinst_PROGRAMS = substitute sgv tree_builders sg-based-p2p p2p-eager fragment-omp-for

substitute_SOURCES = substitute.c
substitute_CPPFLAGS = -I$(top_srcdir)/collectives

sgv_SOURCES = sgv.c

tree_builders_SOURCES = tree_builders.cpp

sg_based_p2p_SOURCES = sg-based-p2p.c

p2p_eager_SOURCES = p2p-eager.c

fragment_omp_for_SOURCES = fragment-omp-for.c
fragment_omp_for_CFLAGS = -fopenmp 

AM_CPPFLAGS = -I$(top_srcdir)
LDADD = \
	$(top_builddir)/collectives/libmpib_coll.la \
	$(top_builddir)/benchmarks/libmpib.a \
	$(top_builddir)/p2p/libmpib_p2p.a
