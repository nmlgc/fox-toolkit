#
#  Top-level makefile for Digital Mars C++
#

all:
	cd ..\..\utils
	smake -f Makefile.dmc
	cd ..\src
	smake -f Makefile.dmc
	cd ..\tests
	smake -f Makefile.dmc
	cd ..\adie
	smake -f Makefile.dmc
	cd ..\pathfinder
	smake -f Makefile.dmc
	cd ..\calculator
	smake -f Makefile.dmc

clean:
	cd ..\..\utils
	smake -f Makefile.dmc clean
	cd ..\src
	smake -f Makefile.dmc clean
	cd ..\tests
	smake -f Makefile.dmc clean
	cd ..\adie
	smake -f Makefile.dmc clean
	cd ..\pathfinder
	smake -f Makefile.dmc clean
	cd ..\calculator
	smake -f Makefile.dmc clean
