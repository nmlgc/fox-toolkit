## Process this file with automake to produce Makefile.in

CXXFLAGS = @CXXFLAGS@ @X_CFLAGS@
INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include

bin_PROGRAMS	= reswrap

man_MANS        = reswrap.1

reswrap_SOURCES	= reswrap.cpp

EXTRA_DIST      = $(man_MANS) \
                  maketree \
                  Makefile.bc \
                  Makefile.icc
