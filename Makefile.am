## Process this file with automake to produce Makefile.in

# For non-gcc compilers, change the options to "no-dependencies"
AUTOMAKE_OPTIONS = foreign dist-zip
#AUTOMAKE_OPTIONS = foreign dist-zip no-dependencies

SUBDIRS 	= utils include src doc tests adie pathfinder calculator
EXTRA_DIST	= ADDITIONS AUTHORS INSTALL LICENSE TRACING README fox.lsm index.html aclocal.m4

CLEANFILES = fox.tar.gz

snapshot: dist-all
	cp $(distdir).tar.gz /home/ftp/pub/fox-snapshot.tar.gz
	cp $(distdir).zip /home/ftp/pub/fox-snapshot.zip


web: distdir
	cp -r $(distdir)/doc/* /home/html

drop: dist-all
	cp $(distdir).tar.gz /home/ftp/pub
	cp $(distdir).zip /home/ftp/pub
	cd /home/ftp/pub && ln -sf $(distdir).tar.gz fox-latest.tar.gz
	cd /home/ftp/pub && ln -sf $(distdir).zip fox-latest.zip


bak:
	tar cvzf fox-cvs.tar.gz /mnt/zip/FOX

dist-hook:
	cp fox.spec $(distdir)
	cp -dpR windows $(distdir)/windows



