Summary: Shared libraries for the FOX 1.0.x toolkit.
Name: fox
Version: 1.0.43
Release: 1
Copyright: LGPL
Group: System Environment/Libraries
Source: ftp://ftp.fox-toolkit.org/pub/fox-1.0.43.tar.gz
URL: http://www.fox-toolkit.org
Packager: Lyle Johnson (lyle@users.sourceforge.net)
BuildRoot: %{_tmppath}/%{name}-buildroot

%define prefix %{_prefix}

%description
FOX is a C++-based library for graphical user interface development
FOX supports modern GUI features, such as drag-and-drop, tooltips, tab
books, tree lists, icons, multiple document interfaces (MDI), timers,
idle processing, automatic GUI updating, as well as OpenGL/Mesa for
3D graphics. Subclassing of basic FOX widgets allows for easy
extension beyond the built-in widgets by application writers.

%package devel
Summary: Development files and documentation for the FOX GUI toolkit.
Group: Development/Libraries

%description devel
The fox-devel package contains the files necessary to develop applications
using the FOX GUI toolkit: the header files, the reswrap resource compiler,
manual pages, and HTML documentation.

%package static
Summary: A version of the FOX GUI toolkit for static linking.
Group: Development/Libraries

%description static
The fox-static package contains the files necessary to link applications
to the FOX GUI toolkit statically (rather than dynamically). Statically
linked applications do not require the library to be installed on the system
running the application.

%package example-apps
Summary: FOX example applications
Group: X11/Applications

%description example-apps
The fox-example-apps package contains executables for several FOX-based
applications, including Adie, calculator and PathFinder.

%prep
%setup -q

%build
CPPFLAGS="$RPM_OPT_FLAGS -frtti" CFLAGS="$RPM_OPT_FLAGS -frtti" \
./configure --prefix=%{prefix} --with-opengl=opengl --enable-release

%install
rm -rf $RPM_BUILD_ROOT
make install prefix=$RPM_BUILD_ROOT/usr
cp -p pathfinder/PathFinder $RPM_BUILD_ROOT/usr/bin
rm -f doc/Makefile.am doc/Makefile.in doc/Makefile
rm -r doc/art/Makefile.am doc/art/Makefile.in doc/art/Makefile
rm -f doc/screenshots/Makefile.am doc/screenshots/Makefile.in doc/screenshots/Makefile

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/lib/libFOX.so
/usr/lib/libFOX-1.0.so.0
/usr/lib/libFOX-1.0.so.0.0.43
%doc doc
%doc ADDITIONS AUTHORS INSTALL LICENSE README TRACING index.html

%files devel
%defattr(-,root,root)
/usr/bin/reswrap
/usr/include/fox
/usr/lib/libFOX.la
/usr/man/*/*

%files static
%defattr(-,root,root)
/usr/lib/libFOX.a

%files example-apps
%defattr(-,root,root)
/usr/bin/adie
/usr/bin/PathFinder
/usr/bin/calculator

%changelog
* Wed Aug 27 2002 Lyle Johnson <lyle@users.sourceforge.net>
- remove Makefile scraps from the doc subdirectories

* Wed Aug 21 2002 Lyle Johnson <lyle@users.sourceforge.net>
- added the fox-devel and fox-static subpackages.

* Tue Oct 10 2000 David Sugar <dyfet@ostel.com> 0.99.132-3
- rtti forced for rpm build specs that use -fno-rtti.

* Fri Mar 24 2000 José Romildo Malaquias <romildo@iceb.ufpo.b> 0.99.122-1
- new version

* Fri Mar 24 2000 José Romildo Malaquias <romildo@iceb.ufpo.b> 0.99.119-1
- new version

* Sun Mar 05 2000 José Romildo Malaquias <romildo@iceb.ufpo.b>
- some adaptations

* Tue Nov 10 1998 René van Paassen <M.M.vanPaassen@lr.tudelft.nl>
- initial package
