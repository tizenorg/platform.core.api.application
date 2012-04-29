Name:       capi-appfw-application
Summary:    An Application library in Tizen C API
Version:    0.1.0
Release:    1
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
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
An Application library in Tizen C API

%package devel
Summary:  An Application library in Tizen C API (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel
An Application library in Tizen C API (DEV)

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

%files devel
%{_includedir}/appfw/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-appfw-application.so


