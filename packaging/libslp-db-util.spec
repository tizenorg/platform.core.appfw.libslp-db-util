Name:       libslp-db-util
Summary:    DB Utility
Version:    0.1.0
Release:    49
Group:      TO_BE/FILLED_IN
License:    Apache License, Version 2.0
Source0:    libslp-db-util-%{version}.tar.gz
Source1001: packaging/libslp-db-util.manifest 
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(dlog)

%description
libslp-db-util-dev package

%package devel
Summary:    Devel package for libslp-db-util (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(sqlite3)

%description devel

%prep
%setup -q -n %{name}-%{version}

%build
cp %{SOURCE1001} .
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

if [ ! -d %{buildroot}/opt/dbspace ]; then
        mkdir -p %{buildroot}/opt/dbspace
fi

%post
/sbin/ldconfig

chown :5000 /opt/dbspace
chmod 775 /opt/dbspace

%postun -p /sbin/ldconfig

%files
%manifest libslp-db-util.manifest
%defattr(-,root,root,-)
/opt/dbspace
%{_libdir}/libSLP-db-util.so.0
%{_libdir}/libSLP-db-util.so.0.1.0

%files devel
%manifest libslp-db-util.manifest
%defattr(-,root,root,-)
%{_includedir}/db-util/*.h
%{_libdir}/pkgconfig/db-util.pc
%{_libdir}/libSLP-db-util.so

