
Summary: WiRed Panda is a real time logic circuits simulator.
Name: wiredpanda
Version: 1.7
Release: 1
Copyright: GPL
Group: Applications/Engineering
Source: https://github.com/GIBIS-UNIFESP/wiRedPanda/archive/master.zip
/*Patch: eject-2.0.2-buildroot.patch*/
BuildRoot: /var/tmp/%{name}-buildroot

%description
WiRed Panda is a free software designed to help students
to learn about logic circuits and simulate them in an easy
and friendly way.

%prep
%setup -q
%patch -p1 -b .buildroot

%build
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/man/man1

install -s -m 755 eject $RPM_BUILD_ROOT/usr/bin/eject
install -m 644 eject.1 $RPM_BUILD_ROOT/usr/man/man1/eject.1

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README TODO COPYING ChangeLog

/usr/bin/eject
/usr/man/man1/eject.1

%changelog
* Sun Mar 21 1999 Cristian Gafton <gafton@redhat.com>
- auto rebuild in the new build environment (release 3)

* Wed Feb 24 1999 Preston Brown <pbrown@redhat.com>
- Injected new description and group.

[ Some changelog entries trimmed for brevity.  -Editor. ]
