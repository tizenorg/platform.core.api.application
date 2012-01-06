Name:       capi-appfw-application
Summary:    An Application library in Tizen C API
Version:    0.0.1
Release:    1
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
BuildRequires:  pkgconfig(heynoti)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(alarm-service)
BuildRequires:  pkgconfig(capi-base-common)
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
cmake . -DCMAKE_INSTALL_PREFIX=/usr


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%{_libdir}/libcapi-appfw-application.so

%files devel
%{_includedir}/appfw/*.h
%{_libdir}/pkgconfig/*.pc


