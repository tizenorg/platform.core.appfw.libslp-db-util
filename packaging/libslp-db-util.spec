
Name:       libslp-db-util
Summary:    DB Utility
Version:    0.1.0
Release:    2.1
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    libslp-db-util-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(sqlite3)


%description
libslp-db-util-dev package



%package devel
Summary:    Devel package for libslp-db-util (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(sqlite3)

%description devel
heynoti API (devel)


%prep
%setup -q -n %{name}-%{version}


%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install




%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig





%files
%defattr(-,root,root,-)
%doc COPYING
%{_libdir}/libSLP-db-util.so.0
%{_libdir}/libSLP-db-util.so.0.1.0


%files devel
%defattr(-,root,root,-)
#%{_prefix}/bin/test_db
%{_includedir}/db-util/*.h
%{_libdir}/pkgconfig/db-util.pc
%{_libdir}/libSLP-db-util.so

