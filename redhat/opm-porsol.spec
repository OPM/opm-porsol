#
# spec file for package opm-porsol
#

%define tag rc1

Name:           opm-porsol
Version:        2015.10
Release:        0
Summary:        Open Porous Media - porsol library
License:        GPL-3.0
Group:          Development/Libraries/C and C++
Url:            http://www.opm-project.org/
Source0:        https://github.com/OPM/%{name}/archive/release/%{version}/%{tag}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:  blas-devel lapack-devel dune-common-devel boost148-devel
BuildRequires:  git suitesparse-devel doxygen bc opm-material-devel opm-common-devel opm-material-devel
BuildRequires:  tinyxml-devel dune-istl-devel opm-core-devel opm-parser-devel
%{?el6:BuildRequires: cmake28 devtoolset-2 superlu-devel}
%{!?el6:BuildRequires: cmake gcc gcc-c++ SuperLU-devel}
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Requires:       libopm-porsol1 = %{version}

%description
This module provides semi-implicit pressure and transport solvers using the IMPES method.

%package -n libopm-porsol1
Summary:        Open Porous Media - porsol library
Group:          System/Libraries

%description -n libopm-porsol1
This module provides semi-implicit pressure and transport solvers using the IMPES method.

%package devel
Summary:        Development and header files for opm-porsol
Group:          Development/Libraries/C and C++
Requires:       %{name} = %{version}
Requires:       blas-devel
Requires:       lapack-devel
Requires:       suitesparse-devel
Requires:       libopm-porsol1 = %{version}

%description devel
This package contains the development and header files for opm-porsol

%package doc
Summary:        Documentation files for opm-porsol
Group:          Documentation
BuildArch:	noarch

%description doc
This package contains the documentation files for opm-porsol

%package bin
Summary:        Applications in opm-porsol
Group:          Scientific
Requires:       %{name} = %{version}
Requires:       libopm-porsol1 = %{version}

%description bin
This package contains the applications for opm-porsol

%prep
%setup -q -n %{name}-release-%{version}-%{tag}

# consider using -DUSE_VERSIONED_DIR=ON if backporting
%build
%{?el6:scl enable devtoolset-2 bash}
%{?el6:cmake28} %{!?el6:cmake} -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_INSTALL_DOCDIR=share/doc/%{name}-%{version} -DUSE_RUNPATH=OFF %{?el6:-DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-2/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-2/root/usr/bin/gcc} -DBOOST_LIBRARYDIR=%{_libdir}/boost148 -DBOOST_INCLUDEDIR=/usr/include/boost148
make

%install
make install DESTDIR=${RPM_BUILD_ROOT}
make install-html DESTDIR=${RPM_BUILD_ROOT}

%clean
rm -rf %{buildroot}

%post -n libopm-porsol1 -p /sbin/ldconfig

%postun -n libopm-porsol1 -p /sbin/ldconfig

%files
%doc README

%files doc
%{_docdir}/*

%files -n libopm-porsol1
%defattr(-,root,root,-)
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_libdir}/dunecontrol/*
%{_libdir}/pkgconfig/*
%{_includedir}/*
%{_datadir}/cmake/*

%files bin
%{_bindir}/*
