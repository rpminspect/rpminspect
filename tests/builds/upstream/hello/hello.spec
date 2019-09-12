# SPEC file overview:
# https://docs.fedoraproject.org/en-US/quick-docs/creating-rpm-packages/#con_rpm-spec-file-overview
# Fedora packaging guidelines:
# https://docs.fedoraproject.org/en-US/packaging-guidelines/


Name:    hello
Version: 1.47.0
Release: 1%{?dist}
Summary: Hello - an example package

License: GPLv3+
URL:     https://github.com/rpminspect
Source0: %{name}.tar.gz

BuildRequires: gcc


%description
This is the Hello description.


%prep
%setup -q -n %{name}


%build
make %{?_smp_mflags}


%install
%make_install


%files
%doc README
%license COPYING
%{_bindir}/hello
