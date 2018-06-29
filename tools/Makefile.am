bin_PROGRAMS = \
	p2p \
	collective \
	collective_test \
	generate_factors \
	p2p_forward 

p2p_SOURCES = p2p.c
nodist_EXTRA_p2p_SOURCES = dummy.cpp

collective_SOURCES = collective.c
nodist_EXTRA_collective_SOURCES = dummy.cpp

collective_test_SOURCES = \
	collective_test.c \
	mpib_test.c \
	mpib_test.h

generate_factors_SOURCES = generate_factors.c
nodist_EXTRA_collective_test_SOURCES = dummy.cpp

p2p_forward_SOURCES = p2p_forward.c

AM_CPPFLAGS = -I$(top_srcdir)
LDADD = \
	$(top_builddir)/collectives/libmpib_coll.la \
	$(top_builddir)/benchmarks/libmpib.a \
	$(top_builddir)/p2p/libmpib_p2p.a

EXTRA_DIST = \
	p2p.plot \
	collective.plot
