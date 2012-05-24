%define _qtmodule_snapshot_version 5~5.0.0~alpha1
Name:       qt5-qtmultimedia
Summary:    Qt Multimedia module
Version:    %{_qtmodule_snapshot_version}
Release:    1%{?dist}
Group:      Qt/Qt
License:    LGPLv2.1 with exception or GPLv3
URL:        http://qt.nokia.com
Source0:    %{name}-%{version}.tar.gz
Patch0:     disable_demos_and_examples.patch
Patch1:     create_prl_and_pc_files.patch
BuildRequires:  qt5-qtcore-devel
BuildRequires:  qt5-qtgui-devel
BuildRequires:  qt5-qtwidgets-devel
BuildRequires:  qt5-qtopengl-devel
BuildRequires:  qt5-qtnetwork-devel
BuildRequires:  qt5-qtqml-devel
BuildRequires:  qt5-qtqml-qtquick-devel
BuildRequires:  qt5-qmake
BuildRequires:  pkgconfig(alsa)
BuildRequires:  fdupes

%description
Qt is a cross-platform application and UI framework. Using Qt, you can
write web-enabled applications once and deploy them across desktop,
mobile and embedded systems without rewriting the source code.
.
This package contains the QtMultimedia module


%package devel
Summary:    Qt Multimedia - development files
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}

%description devel
Qt is a cross-platform application and UI framework. Using Qt, you can
write web-enabled applications once and deploy them across desktop,
mobile and embedded systems without rewriting the source code.
.
This package contains the QtMultimedia module development files

%package -n qt5-qtqml-import-multimedia
Summary:    QtQml multimedia import
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtqml

%description -n qt5-qtqml-import-multimedia
This package contains the Multimedia import for QtQml

%package plugin-mediaservice-audioengine
Summary:    Qt Multimedia - audio engine media-service
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}

%description plugin-mediaservice-audioengine
This package contains the audio engine plugin for media-service

%package plugin-playlistformats-m3u
Summary:    Qt Multimedia - M3U playlist support
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}

%description plugin-playlistformats-m3u
This package contains the M3U playlist support



#### Build section

%prep
%setup -q -n %{name}
%patch0 -p1
%patch1 -p1


%build
export QTDIR=/usr/share/qt5
qmake
make %{?_smp_flags}

%install
rm -rf %{buildroot}
%qmake_install
# Remove unneeded .la files
rm -f %{buildroot}/%{_libdir}/*.la
# Fix wrong path in pkgconfig files
find %{buildroot}%{_libdir}/pkgconfig -type f -name '*.pc' \
-exec perl -pi -e "s, -L%{_builddir}/?\S+,,g" {} \;
# Fix wrong path in prl files
find %{buildroot}%{_libdir} -type f -name '*.prl' \
-exec sed -i -e "/^QMAKE_PRL_BUILD_DIR/d;s/\(QMAKE_PRL_LIBS =\).*/\1/" {} \;
#
%fdupes %{buildroot}/%{_includedir}




#### Pre/Post section

%post
/sbin/ldconfig
%postun
/sbin/ldconfig




#### File section


%files
%defattr(-,root,root,-)
%{_libdir}/libQtMultimedia.so.5
%{_libdir}/libQtMultimedia.so.5.*
%{_libdir}/libQtMultimediaWidgets.so.5
%{_libdir}/libQtMultimediaWidgets.so.5.*
%{_libdir}/libQtMultimediaQuick_p.so.1
%{_libdir}/libQtMultimediaQuick_p.so.1.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/libQtMultimedia.so
%{_libdir}/libQtMultimediaWidgets.so
%{_libdir}/libQtMultimediaQuick_p.so
%{_libdir}/libQtMultimedia.prl
%{_libdir}/libQtMultimediaWidgets.prl
%{_libdir}/libQtMultimediaQuick_p.prl
%{_libdir}/pkgconfig/*
%{_includedir}/qt5/*
%{_datadir}/qt5/mkspecs/
%{_libdir}/cmake/


%files -n qt5-qtqml-import-multimedia
%defattr(-,root,root,-)
%{_libdir}/qt5/imports/QtMultimedia/

%files plugin-mediaservice-audioengine
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/mediaservice/libqtmedia_audioengine.so

%files plugin-playlistformats-m3u
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/playlistformats/libqtmultimedia_m3u.so


#### No changelog section, separate $pkg.changelog contains the history
