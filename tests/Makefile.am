## Process this file with automake to produce Makefile.in

CXXFLAGS = @CXXFLAGS@ @X_CFLAGS@
INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include

LDADD		= ../src/libFOX.la -lm

#gltest_LDADD	= $(LDADD) $(GL_LIBS)
#glviewer_LDADD  = $(LDADD) $(GL_LIBS)

EXTRA_PROGRAMS	= \
gltest \
glviewer


noinst_PROGRAMS	= \
rex \
table \
minheritance \
imageviewer \
button \
datatarget \
dctest \
dialog \
dirlist \
foursplit \
groupbox \
header \
hello \
hello2 \
iconlist \
image \
mditest \
scribble \
shutter \
splitter \
switcher \
tabbook \
$(GL_TESTS)

rex_SOURCES	        = rex.cpp
minheritance_SOURCES	= minheritance.cpp
gltest_SOURCES	        = gltest.cpp
glviewer_SOURCES        = glviewer.cpp
mditest_SOURCES		= mditest.cpp
table_SOURCES		= table.cpp
groupbox_SOURCES	= groupbox.cpp
foursplit_SOURCES	= foursplit.cpp
datatarget_SOURCES	= datatarget.cpp
imageviewer_SOURCES	= imageviewer.cpp
scribble_SOURCES	= scribble.cpp
splitter_SOURCES	= splitter.cpp
shutter_SOURCES		= shutter.cpp
image_SOURCES		= image.cpp
header_SOURCES		= header.cpp
hello_SOURCES		= hello.cpp
hello2_SOURCES		= hello2.cpp
switcher_SOURCES	= switcher.cpp
iconlist_SOURCES	= iconlist.cpp
tabbook_SOURCES		= tabbook.cpp
dialog_SOURCES		= dialog.cpp
dirlist_SOURCES		= dirlist.cpp
button_SOURCES		= button.cpp
dctest_SOURCES		= dctest.cpp dippy.h



EXTRA_DIST = Makefile.bc \
             Makefile.icc \
             Makefile.dmc
