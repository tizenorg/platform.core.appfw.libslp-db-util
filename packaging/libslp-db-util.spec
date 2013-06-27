Name:           libslp-db-util
Version:        0.1.1
Release:        5
License:        Apache-2.0
Summary:        DB Utility
Group:          Application Framework/Database
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(sqlite3)

%description
DB Utility.

%package devel
Summary:        Devel package for libslp-db-util (devel)
Requires:       %{name} = %{version}
%description devel
Devel package for libslp-db-util (devel)

%prep
%setup -q

%build
%cmake .
make %{?_smp_mflags}

%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest libSLP-db-util.manifest
%license LICENSE
%defattr(-,root,root,-)
%{_libdir}/libSLP-db-util.so.0
%{_libdir}/libSLP-db-util.so.0.1.0

%files devel
%defattr(-,root,root,-)
%{_includedir}/db-util/*.h
%{_libdir}/pkgconfig/db-util.pc
%{_libdir}/libSLP-db-util.so

