#
# spec file for package opm-porsol
#

%define tag rc3

Name:           opm-porsol
Version:        2013.10
Release:        0
Summary:        Open Porous Media - porsol library
License:        GPL-3.0
Group:          Development/Libraries/C and C++
Url:            http://www.opm-project.org/
Source0:        https://github.com/OPM/%{name}/archive/release/%{version}/%{tag}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:  blas-devel lapack-devel dune-common-devel
BuildRequires:  git suitesparse-devel cmake28 doxygen bc opm-material-devel
BuildRequires:  tinyxml-devel dune-istl-devel superlu-devel opm-core-devel
%{?el5:BuildRequires: gcc44 gcc44-gfortran gcc44-c++}
%{!?el5:BuildRequires: gcc gcc-gfortran gcc-c++}
%{?el5:BuildRequires: boost141-devel}
%{!?el5:BuildRequires: boost-devel}
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
cmake28 -DBUILD_SHARED_LIBS=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_INSTALL_DOCDIR=share/doc/%{name}-%{version} -DUSE_RUNPATH=OFF %{?el5:-DCMAKE_CXX_COMPILER=g++44 -DCMAKE_C_COMPILER=gcc44 -DCMAKE_Fortran_COMPILER=gfortran44 -DBOOST_LIBRARYDIR=%{libdir}/boost141 -DBOOST_INCLUDEDIR=/usr/include/boost141}
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
