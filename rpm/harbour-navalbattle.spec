Name:       harbour-navalbattle
Summary:    Naval Battle
Version:    1.0
Release:        0
Group:      Applications/Games
License:    GPLv3+
URL:        https://example.invalid/
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  cmake
BuildRequires:  make
BuildRequires:  gcc-c++

Requires:       sailfishsilica-qt5

%description
Battleship-style naval battle game for Sailfish OS. Single-player vs AI.

%prep
%setup -q -n %{name}

%build
%cmake .
%cmake_build

%install
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/86x86/apps
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/108x108/apps
install -m 0644 icons/86x86/harbour-navalbattle.png %{buildroot}%{_datadir}/icons/hicolor/86x86/apps/harbour-navalbattle.png
install -m 0644 icons/108x108/harbour-navalbattle.png %{buildroot}%{_datadir}/icons/hicolor/108x108/apps/harbour-navalbattle.png
%cmake_install

%files
%defattr(-,root,root,-)
%{_datadir}/icons/hicolor/86x86/apps/harbour-navalbattle.png
%{_datadir}/icons/hicolor/108x108/apps/harbour-navalbattle.png
%{_bindir}/%{name}
%{_datadir}/%{name}/qml
%{_datadir}/applications/%{name}.desktop
%{_datadir}/metainfo/%{name}.appdata.xml

%changelog
