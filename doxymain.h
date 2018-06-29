#ifndef DOXYMAIN_H_
#define DOXYMAIN_H_

/*!
 * \mainpage Introduction
 * Accurate estimation of the execution time of MPI communication operations plays an important role
 * in optimization of parallel applications. A priori information about the performance of
 * each MPI operation allows a software developer to design a parallel application in such a way that
 * it will have maximum performance. This data can also be useful for tuning
 * collective communication operations and for the evaluation of different available implementations.
 * The choice of collective algorithms becomes even more important in heterogeneous environments.
 * 
 * A typical MPI benchmarking suite uses only one timing method to estimate the execution time of
 * the MPI communications. The method provides a certain accuracy and efficiency.
 * The efficiency of the timing method is particularly important in self-adaptable parallel applications
 * using runtime benchmarking of communication operations to optimize their performance
 * on the executing platform. In this case, less accurate results can be acceptable
 * in favor of a rapid response from the benchmark. We design a new MPI benchmarking suite called
 * MPIBlib \latexonly\cite{Lastovetsky2008MPIBlib}\endlatexonly that provides a variety of timing methods.
 * This suite supports both fast measurement of collective operations and exhaustive benchmarking.
 * 
 * In addition to general timing methods that are universally applicable to all communication operations,
 * MPIBlib includes methods that can only be used for measurement of one or more specific operations.
 * Where applicable, these operation-specific methods work faster than their universal counterparts and
 * can be used as their time-efficient alternatives.
 * 
 * Most of the MPI benchmarking suites are designed in the form of a standalone executable program
 * that takes the parameters of communication experiments and produce a lot of output data
 * for further analysis. As such, they cannot be integrated easily and efficiently
 * into application-level software. Therefore, there is a need for a benchmarking library
 * that can be used in parallel applications or programming systems for communication performance modeling
 * and tuning communication operations. MPIBlib is such a library that can be linked to other applications
 * and used at runtime. 
 * 
 * \section authors Authors
 * Alexey Lastovetsky, Vladimir Rychkov, Maureen O'Flynn, Kiril Dichev
 * 
 * Heterogeneous Computing Laboratory\n
 * School of Computer Science and Informatics, University College Dublin\n
 * Belfield, Dublin 4, Ireland\n
 * http://hcl.ucd.ie
 * 
 * {alexey.lastovetsky, vladimir.rychkov}\@ucd.ie
 * 
 * \latexonly
 * \bibliographystyle{plain}
 * \bibliography{mpib}
 * \endlatexonly
 * 
 * \page installation Installation
 * \include README
 * 
 * \page design The software design
 * MPIBlib is implemented in C/C++ on top of MPI.
 * The package consists of libraries, tools and tests.
 * The libraries implement point-to-point and collective benchmarks and
 * different algorithms of point-to-point and collective communication operations.
 * The tools verify algorithms and perform benchmarks. Their output includes
 * the results of measurements and communication trees
 * (for tree-based algorithms of collective operations).
 * The results of measurements can be visualized by the gnuplot utility;
 * MPIBlib provides the basic gnuplot scripts.
 * The communication trees built in the tree-based collective algorithms
 * can be visualized by the dot utility (a part of Graphviz).
 *
 * \dotfile mpib_design.dot
 * 
 */

#endif /*DOXYMAIN_H_*/
