Name:           wiredpanda
Version:        5.0.1
Release:        1%{?dist}
Summary:        Logic circuit simulator focused on education

License:        GPL-3.0-or-later
URL:            https://github.com/GIBIS-UNIFESP/wiRedPanda
Source0:        %{url}/archive/refs/tags/%{version}.tar.gz#/%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  ninja-build
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtsvg-devel
BuildRequires:  qt6-qtmultimedia-devel
BuildRequires:  qt6-qttools-devel
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

Requires:       qt6-qtbase
Requires:       qt6-qtsvg
Requires:       qt6-qtmultimedia
Requires:       hicolor-icon-theme

%description
wiRedPanda is free software designed to help students learn about logic
circuits and simulate them in an easy and friendly way.

Features include real-time logic simulation, a user-friendly interface,
and support for exporting circuits as images or PDFs.

%prep
%autosetup -n wiRedPanda-%{version}

%build
%cmake -GNinja -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/wiredpanda.desktop
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/io.github.gibis_unifesp.wiRedPanda.metainfo.xml

%files
%license LICENSE
%doc README.md
%{_bindir}/wiredpanda
%{_datadir}/applications/wiredpanda.desktop
%{_datadir}/icons/hicolor/26x26/apps/wpanda.png
%{_datadir}/icons/hicolor/32x32/apps/wpanda.png
%{_datadir}/icons/hicolor/48x48/apps/wpanda.png
%{_datadir}/icons/hicolor/64x64/apps/wpanda.png
%{_datadir}/icons/hicolor/128x128/apps/wpanda.png
%{_datadir}/icons/hicolor/256x256/apps/wpanda.png
%{_datadir}/icons/hicolor/scalable/apps/wpanda.svg
%{_datadir}/mime/packages/wiredpanda.xml
%{_datadir}/metainfo/io.github.gibis_unifesp.wiRedPanda.metainfo.xml
%{_datadir}/wiredpanda/

%changelog
* Wed May 13 2026 Rodrigo Torres <torres.dark@gmail.com> - 5.0.1-1
- Initial COPR package
