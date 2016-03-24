Name: geoclue-provider-mlsdb
Version: 0.0.4
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

%description
%{summary}.

%package tool
Summary:   Cell tower id to location database tool for geoclue-provider-mlsdb
Group:     System/Application
Requires:  %{name} = %{version}

%description tool
%{summary}.

%package data
Summary:   Cell tower id to location data
Group:     System/Data
Requires:  %{name} = %{version}

%description data
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

%files tool
%defattr(-,root,root,-)
%{_bindir}/geoclue-mlsdb-tool

%files data
%defattr(-,root,root,-)
%{_datadir}/geoclue-provider-mlsdb/mlsdb.data

