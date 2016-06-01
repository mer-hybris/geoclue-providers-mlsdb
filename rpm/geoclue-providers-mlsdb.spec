Name: geoclue-provider-mlsdb
Version: 0.0.9
Release: 1
Summary: Geoinformation Service from Mozilla Location Services Database Provider
Group: System/Libraries
URL: https://github.com/mer-hybris/geoclue-providers-mlsdb
License: LGPLv2.1
Source: %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: pkgconfig(qofono-qt5)
BuildRequires: pkgconfig(qofonoext)
BuildRequires: pkgconfig(connman-qt5)
BuildRequires: pkgconfig(libsailfishkeyprovider)
BuildRequires: pkgconfig(qt5-boostable)
Requires: mapplauncherd-qt5
Requires: %{name}-agreements

%description
%{summary}.

%package agreements
Summary:  Mozilla Location Services privacy policy agreements
Group:    System/Libraries

%description agreements
%{summary}.

%package tool
Summary:   Cell id to location database tool for geoclue-provider-mlsdb
Group:     System/Application
Requires:  %{name} = %{version}

%description tool
%{summary}.

%package data-in
Summary:   Cell id to location data (.in)
Group:     System/Data
Requires:  %{name} = %{version}

%description data-in
%{summary}.


%package data-fi
Summary:   Cell id to location data (.fi)
Group:     System/Data
Requires:  %{name} = %{version}

%description data-fi
%{summary}.


%package data-au
Summary:   Cell id to location data (.au)
Group:     System/Data
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

%files
%defattr(-,root,root,-)
%{_libexecdir}/geoclue-mlsdb
%{_datadir}/dbus-1/services/org.freedesktop.Geoclue.Providers.Mlsdb.service
%{_datadir}/geoclue-providers/geoclue-mlsdb.provider

%files agreements
%defattr(-,root,root,-)
%dir %{_datadir}/geoclue-provider-mlsdb/agreements
%{_datadir}/geoclue-provider-mlsdb/agreements/mls_consent_en_GB.txt


%files tool
%defattr(-,root,root,-)
%{_bindir}/geoclue-mlsdb-tool

%files data-in
%defattr(-,root,root,-)
%{_datadir}/geoclue-provider-mlsdb/in/1/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/in/2/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/in/3/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/in/4/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/in/5/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/in/6/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/in/7/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/in/8/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/in/9/mlsdb.data

%files data-fi
%defattr(-,root,root,-)
%{_datadir}/geoclue-provider-mlsdb/fi/1/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/fi/2/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/fi/3/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/fi/4/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/fi/5/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/fi/6/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/fi/7/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/fi/8/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/fi/9/mlsdb.data

%files data-au
%defattr(-,root,root,-)
%{_datadir}/geoclue-provider-mlsdb/au/1/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/au/2/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/au/3/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/au/4/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/au/5/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/au/6/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/au/7/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/au/8/mlsdb.data
%{_datadir}/geoclue-provider-mlsdb/au/9/mlsdb.data
