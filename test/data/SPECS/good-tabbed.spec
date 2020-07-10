# Used to test macro expansion in the Release field.

%define main_release 4.7

%if "x%{?pre_release}" != "x"
 %define rpminspect_release 0.%{main_release}.%{pre_release}%{?dist}
 %else
 %define rpminspect_release %{main_release}%{?dist}
%endif

Name:		example
Version:	0.1
Release:	%{rpminspect_release}
Summary:	Example summary
License:	GPLv3+
URL:		http://www.example.net/

Source0:	rpminspect

%description
Example description.

%prep

%build

%install
install -D -m 0755 %{SOURCE0} %{buildroot}%{_bindir}/rpminspect

%files
%{_bindir}/rpminspect
