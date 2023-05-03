


# Installation

## Required software:

1. any C/C++ and MPI (MPICH-1 does not support shared libraries)
2. GSL (GNU Scientific Library)
3. Boost (The Boost C++ libraries: Graph)

## Optional software:

4. Gnuplot (An Interactive Plotting Program): for performance diagrams.
5. Graphviz (Graph Visualization Software: dot): for tree visualization.

------------------------------------------------------------------------

### 1. GSL

(example for *gsl-2.7.1*):
Default installing:

``$ cd gsl-2.7.1``

``$ ./configure``

``$ make``

``$ sudo make install``

Otherwise, if GSL is installed in a non-default directory:

``$ export LD_LIBRARY_PATH=DIR/lib:$LD_LIBRARY_PATH``


### 2. Boost

``$ ./bootstrap.sh``

``$ ./b2``

Previous should be sufficient for mpiblib. In case installation fails, try the following:

1. Boost should be configured with at least the Graph library
   (default: all)

``$ ./configure --prefix=DIR --with-libraries=graph``

2. Default installation:
 - DIR/include/boost_version/boost
 - DIR/lib/libboost_library_versions.*

Create symbolic links:

``$ cd DIR/include; ln -s boost_version/boost``

``$ cd DIR/lib; ln -s libboost_[library]_[version].[a/so] libboost_[library].[a/so]``

``$ export LD_LIBRARY_PATH=DIR/lib:$LD_LIBRARY_PATH``


## For users

Download the latest package from <http://hcl.ucd.ie/project/mpiblib>

``$ tar -zxvf mpiblib-X.X.X.tar.gz``

``$ cd mpiblib-X.X.X``

``$ ./configure``

``$ make all install``


Configuration
-------------

Packages:
  --with-gsl-dir=DIR      GNU Scientific Library directory
  --with-boost-dir=DIR    The Boost C++ libraries directory

Check configure options:

``$ ./configure -h``


## For developers:

Required software:
1. Subversion
2. GNU autotools
3. Doxygen, Graphviz and any TeX - optional (for reference manual)

``$ svn co https://hcl.ucd.ie/repos/CPM/trunk/MPIBlib``

``$ cd MPIBlib``

``$ autoreconf -i``

``$ ./configure --enable-debug``

``$ make all``


Patching:
------------

This repository constains a patch for MPIBlib tool developed by Vladimir Rychkov, Alexey Lastovetsky and Kiril Dichev.
To apply this patch download original mpiblib 1.2.0 version from https://hcl.ucd.ie/project/mpiblib.
After that, include this patch file into mpiblib folder and use the patch command to apply the patch.

Instructions:

``$ cp -r ~/mpiblib.patch ~/mpiblib-1.2.0``

``$ cd ~/mpiblib-1.2.0``

``$ patch -p1 < mpiblib.patch``

Now we can proceed with the next steps.


How to use:
------------

Go to your prefix location binary path, and then run:

``$ mpirun -n 2 p2p -m 2 -M 500000``


Where *-n* are number of processes, *p2p* is the binary created, *-m* is the minimun message size and *-M* is the maximum. You can configure stride between message with *-s*

To get the tau-lop parameters using mpiblib, please, visit: <https://github.com/hpc-unex/libtaulop> for downloading and intructions.

To create a package:
$ svn log -v > ChangeLog
$ make dist
