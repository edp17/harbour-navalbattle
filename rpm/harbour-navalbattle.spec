Name:       harbour-navalbattle
Summary:    Naval Battle
Version:    1.1
Release:    0
Group:      Applications/Games
License:    GPLv3+
URL:        https://github.com/edp17/harbour-navalbattle
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  qt5-qttools-linguist
BuildRequires:  cmake
BuildRequires:  make
BuildRequires:  gcc-c++

Requires:       sailfishsilica-qt5
Requires:       qt5-qtmultimedia
Requires:       qt5-qtdeclarative-import-multimedia
Requires:       qt5-qtmultimedia-plugin-audio-pulseaudio
Requires:       qt5-qtmultimedia-plugin-mediaservice-gstmediaplayer
Requires:       qt5-qtmultimedia-plugin-resourcepolicy-resourceqt

%description
Battleship-style naval battle game for Sailfish OS. Single-player vs AI.

%prep
%setup -q -n %{name}

%build
%cmake -DNAVALBATTLE_VERSION=%{version} -DNAVALBATTLE_BUILD_TESTS=OFF .
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
%{_datadir}/%{name}/translations
%{_datadir}/applications/%{name}.desktop
%{_datadir}/metainfo/%{name}.appdata.xml

%changelog
