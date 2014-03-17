Name:       capi-appfw-application
Summary:    An Application library in SLP C API
Version:    0.1.0
Release:    55
Group:      API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: 	capi-appfw-application.manifest
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
BuildRequires:  pkgconfig(libtzplatform-config)



%description
An Application library in SLP C API

%package devel
Summary:  An Application library in SLP C API (Development)
Group:    API
Requires: %{name} = %{version}-%{release}

%description devel
An Application library in SLP C API (DEV)

%prep
%setup -q
cp %{SOURCE1001} .

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%{_libdir}/libcapi-appfw-application.so.*
%manifest capi-appfw-application.manifest
/usr/share/license/%{name}

%files devel
%manifest %{name}.manifest
%{_includedir}/appfw/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-appfw-application.so



