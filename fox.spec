Summary: The FOX C++ GUI Toolkit
Name: fox
Version: 1.0.0
Release: 3
Copyright: GNU LGPL
Group: System Environment/Libraries
URL: http://www.fox-toolkit.org/fox.html
Source0: ftp://ftp.fox-toolkit.org/pub/fox-1.0.0.tar.gz
BuildRoot: %{_tmppath}/%{name}-buildroot
Requires: Mesa

%define prefix %{_prefix}

%description
FOX is a C++-Based Library for Graphical User Interface Development
FOX supports modern GUI features, such as Drag-and-Drop, Tooltips, Tab
Books, Tree Lists, Icons, Multiple-Document Interfaces (MDI), timers,
idle processing, automatic GUI updating, as well as OpenGL/Mesa for
3D graphics.  Subclassing of basic FOX widgets allows for easy
extension beyond the built-in widgets by application writers.

%package example-apps
Summary: FOX example applications
Group: X11/Applications

%description example-apps
editor and file browser, written with FOX

%prep
%setup -q

%build
CPPFLAGS="$RPM_OPT_FLAGS -frtti" CFLAGS="$RPM_OPT_FLAGS -frtti" \
./configure --prefix=%{prefix} --with-opengl=opengl --enable-release
make GL_LIBS="-lGL -lGLU"

%install
rm -rf $RPM_BUILD_ROOT
make install prefix=$RPM_BUILD_ROOT/usr
cp -p pathfinder/PathFinder $RPM_BUILD_ROOT/usr/bin

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/bin/reswrap
/usr/include/fox
/usr/lib/*
/usr/man/*/*
%doc doc
%doc ADDITIONS AUTHORS BUGS INSTALL LICENSE README TRACING

%files example-apps
%defattr(-,root,root)
/usr/bin/adie
/usr/bin/PathFinder
/usr/bin/calculator

%changelog
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





