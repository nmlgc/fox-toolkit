## Process this file with automake to produce Makefile.in

CXXFLAGS = @CXXFLAGS@ @X_CFLAGS@
INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include
RESWRAP  = $(top_builddir)/utils/reswrap
SUFFIXES = .gif .bmp
LDADD	 = ../src/libFOX.la -lm

bin_PROGRAMS	= PathFinder

man_MANS = PathFinder.1

ICONS = \
        bigicons.bmp \
        bigpenguin.bmp \
        clrbook.gif \
        copy.bmp \
        copyit.gif \
        cut.bmp \
        deleteit.bmp \
        desktop.bmp \
        details.bmp \
        dirup.bmp \
        goback.bmp \
        goforw.bmp \
        gotodir.bmp \
        home.gif \
        hosts.bmp \
        linkit.gif \
        maphost.bmp \
        moveit.gif \
        minipenguin.bmp \
        paste.bmp \
        properties.bmp \
        setbook.gif \
        smallicons.bmp \
        unmaphost.bmp \
        work.gif

PathFinder_SOURCES =  \
        icons.h \
        icons.cpp \
        PathFinder.cpp \
        PathFinder.h \
        PropertyDialog.cpp \
        PropertyDialog.h \
        $(ICONS)

PathFinder.cpp: icons.h icons.cpp

icons.h: $(ICONS)
	$(RESWRAP) -i -o icons.h $^

icons.cpp: $(ICONS)
	$(RESWRAP) -e -o icons.cpp $^

CLEANFILES	= icons.h icons.cpp

EXTRA_DIST = $(man_MANS) \
             Makefile.bc

