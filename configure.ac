AC_PREREQ(2.61)
AC_INIT([MPIBlib], [1.2.0], [http://hcl.ucd.ie/project/MPIBlib])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE

AC_PROG_CC([mpicc])
AC_PROG_CXX([mpic++ mpicxx])
AC_PROG_CXX_C_O
AM_PROG_CC_C_O
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_MSG_CHECKING([whether debug is enabled])
AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug], [Enable debug (default: disabled)])], [debug="yes"], [debug="no"])
if test "$debug" = "yes"; then
	AC_DEFINE([MPIB_DEBUG], [1], [Enable debug])
	CFLAGS="-g -Wall -Wno-deprecated"
	CXXFLAGS="-g -Wall -Wno-deprecated"
fi
AC_MSG_RESULT([$debug])

AC_CHECK_LIB([m], [main])
AC_CHECK_LIB([dl], [main])
AC_DEFINE([_GNU_SOURCE], [1], [GNU])

AC_ARG_WITH([gsl_dir], [AC_HELP_STRING([--with-gsl-dir=DIR], [GNU Scientific Library directory])])
if test "$with_gsl_dir" != ""; then
	CPPFLAGS="$CPPFLAGS -I$with_gsl_dir/include"
	LDFLAGS="$LDFLAGS -L$with_gsl_dir/lib"
fi
AC_CHECK_LIB([gslcblas], [main])
AC_CHECK_LIB([gsl], [main], [], AC_MSG_ERROR([gsl is required but not found]))

AC_ARG_WITH([boost_dir], [AC_HELP_STRING([--with-boost-dir=DIR], [The Boost C++ libraries directory (optional)])])
if test "$with_boost_dir" != ""; then
	CPPFLAGS="$CPPFLAGS -I$with_boost_dir/include"
	LDFLAGS="$LDFLAGS -L$with_boost_dir/lib"
fi
AC_CHECK_LIB([boost_graph], [main], [], AC_MSG_ERROR([boost_graph is required but not found]))

AC_CONFIG_FILES([
	Makefile
	p2p/Makefile
	collectives/Makefile
	benchmarks/Makefile
	tools/Makefile
	tests/Makefile
])

DX_PS_FEATURE([OFF])
DX_INIT_DOXYGEN([MPIBlib])
define([svn_revision], esyscmd([sh -c "svnversion -n"]))dnl
DX_ENV_APPEND([SVN_REVISION], [svn_revision])
DX_ENV_APPEND([PREDEFINED], [HAVE_LIBBOOST_GRAPH])

AC_OUTPUT
