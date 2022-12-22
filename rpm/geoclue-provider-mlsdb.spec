Name: geoclue-provider-mlsdb
Version: 0.2.0
Release: 1
Summary: Geoinformation Service from Mozilla Location Services Database Provider
URL: https://github.com/mer-hybris/geoclue-providers-mlsdb
License: LGPLv2
Source0: %{name}-%{version}.tar.gz
Source1: %{name}.privileges
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(qofono-qt5)
BuildRequires: pkgconfig(qofonoext)
BuildRequires: pkgconfig(connman-qt5)
BuildRequires: pkgconfig(libsailfishkeyprovider)
BuildRequires: pkgconfig(qt5-boostable)
BuildRequires: pkgconfig(mlite5)
Requires: mapplauncherd-qt5
Requires: %{name}-agreements

%description
%{summary}.

%package agreements
Summary:  Mozilla Location Services privacy policy agreements

%description agreements
%{summary}.

%package tool
Summary:   Cell id to location database tool for geoclue-provider-mlsdb
Requires:  %{name} = %{version}

%description tool
%{summary}.

%package data-in
Summary:   Cell id to location data (.in)
Requires:  %{name} = %{version}

%description data-in
%{summary}.


%package data-fi
Summary:   Cell id to location data (.fi)
Requires:  %{name} = %{version}

%description data-fi
%{summary}.


%package data-au
Summary:   Cell id to location data (.au)
Requires:  %{name} = %{version}

%description data-au
%{summary}.

%prep
%setup -q -n %{name}-%{version}


%build
qmake -qt=5 geoclue-providers-mlsdb.pro
make %{?_smp_mflags}


%install
make INSTALL_ROOT=%{buildroot} install

mkdir -p %{buildroot}%{_datadir}/mapplauncherd/privileges.d
install -m 644 -p %{SOURCE1} %{buildroot}%{_datadir}/mapplauncherd/privileges.d/
cp -a mlsdbtool/geoclue_tool_wrapper.sh %{buildroot}/usr/bin

%files
%defattr(-,root,root,-)
%license COPYING
%{_libexecdir}/geoclue-mlsdb
%{_datadir}/mapplauncherd/privileges.d/*
%dir %{_datadir}/geoclue-provider-mlsdb
%{_datadir}/dbus-1/services/org.freedesktop.Geoclue.Providers.Mlsdb.service
%{_datadir}/geoclue-providers/geoclue-mlsdb.provider

%files agreements
%defattr(-,root,root,-)
%{_datadir}/geoclue-provider-mlsdb/agreements


%files tool
%defattr(-,root,root,-)
%{_bindir}/geoclue-mlsdb-tool
%{_bindir}/geoclue_tool_wrapper.sh

%files data-in
%defattr(-,root,root,-)
%{_datadir}/geoclue-provider-mlsdb/data/404.dat
%{_datadir}/geoclue-provider-mlsdb/data/405.dat

%files data-fi
%defattr(-,root,root,-)
%{_datadir}/geoclue-provider-mlsdb/data/244.dat

%files data-au
%defattr(-,root,root,-)
%{_datadir}/geoclue-provider-mlsdb/data/505.dat
