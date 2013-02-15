#sbs-git:slp/api/application capi-appfw-application 0.1.0 56d9b8e057f022f0e7fdb1853587158452e7ae1b
Name:       capi-appfw-application
Summary:    An Application library in SLP C API
Version: 0.1.0
Release:    49
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(appcore-common)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(ail)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(alarm-service)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(sqlite3)


Requires(post): /sbin/ldconfig  
Requires(postun): /sbin/ldconfig

%description
An Application library in SLP C API

%package devel
Summary:  An Application library in SLP C API (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel
An Application library in SLP C API (DEV)

%prep
%setup -q

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%{_libdir}/libcapi-appfw-application.so.*
%manifest capi-appfw-application.manifest

%files devel
%{_includedir}/appfw/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-appfw-application.so



