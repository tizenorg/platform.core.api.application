Name:       capi-appfw-application
Summary:    An Application library in SLP C API
Version:    0.1.0
Release:    0
Group:      System/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001:     capi-appfw-application.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(appcore-common)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(alarm-service)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(vconf-internal-keys)
BuildRequires:  pkgconfig(eventsystem)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(glib-2.0)

%description
An Application library in SLP C API package.

%define appfw_feature_process_pool 1

%package devel
Summary:  An Application library in SLP C API (Development)
Group:    System/API
Requires: %{name} = %{version}-%{release}

%description devel
An Application library in SLP C API (Development) package.

%prep
%setup -q
cp %{SOURCE1001} .

%build
%if 0%{?appfw_feature_process_pool}
_APPFW_FEATURE_PROCESS_POOL=ON
%endif

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER} \
	 -D_APPFW_FEATURE_PROCESS_POOL:BOOL=${_APPFW_FEATURE_PROCESS_POOL}
%__make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}%{_datadir}/license
cp LICENSE %{buildroot}%{_datadir}/license/%{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%{_libdir}/libcapi-appfw-application.so.*
%{_libdir}/libcapi-appfw-app-control.so.*
%{_libdir}/libcapi-appfw-app-common.so.*
%{_libdir}/libcapi-appfw-alarm.so.*
%{_libdir}/libcapi-appfw-preference.so.*
%{_libdir}/libcapi-appfw-event.so.*

%{_datadir}/license/%{name}

%files devel
%manifest %{name}.manifest
%{_includedir}/appfw/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-appfw-application.so
%{_libdir}/libcapi-appfw-app-control.so
%{_libdir}/libcapi-appfw-app-common.so
%{_libdir}/libcapi-appfw-alarm.so
%{_libdir}/libcapi-appfw-preference.so
%{_libdir}/libcapi-appfw-event.so

