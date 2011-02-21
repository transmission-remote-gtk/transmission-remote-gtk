Name: transmission-remote-gtk
Version: 0.2.1
Release: 1%{?dist:%{dist}}
Summary: Remote control client for Transmission BitTorrent

Group: Applications/Internet
License: GPLv2+
URL: http://code.google.com/p/transmission-remote-gtk/
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: gtk2
Requires: glib2
Requires: unique
Requires: GConf2
Requires: libnotify
Requires: json-glib
Requires: libcurl

BuildRequires: gtk2-devel
BuildRequires: glib2-devel
BuildRequires: unique-devel
BuildRequires: GConf2-devel
BuildRequires: json-glib-devel
BuildRequires: libcurl-devel
BuildRequires: libnotify-devel

Requires(pre): GConf2
Requires(post): GConf2
Requires(preun): GConf2
Requires(post): desktop-file-utils
Requires(postun): desktop-file-utils

%description
transmission-remote-gtk is a GTK application for remote management of the
Transmission BitTorrent client via its RPC interface. 

%prep
%setup -q

%build
%configure --without-libgeoip
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
make install  DESTDIR=$RPM_BUILD_ROOT
unset GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL

%clean
rm -rf $RPM_BUILD_ROOT

%pre
if [ "$1" -gt 1 ]; then
    export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
    gconftool-2 --makefile-uninstall-rule \
      %{_sysconfdir}/gconf/schemas/%{name}.schemas > /dev/null || :
fi

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule \
  %{_sysconfdir}/gconf/schemas/%{name}.schemas > /dev/null || :

update-desktop-database %{_datadir}/applications >/dev/null 2>&1

%postun
update-desktop-database %{_datadir}/applications >/dev/null 2>&1

%preun
if [ "$1" -eq 0 ]; then
    export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
    gconftool-2 --makefile-uninstall-rule \
      %{_sysconfdir}/gconf/schemas/%{name}.schemas > /dev/null || :
fi

%files
%defattr(-,root,root,-)
%doc README COPYING AUTHORS
%{_sysconfdir}/gconf/schemas/%{name}.schemas
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/transmission-remote-gtk.svg
%{_datadir}/icons/hicolor/16x16/apps/transmission-remote-gtk.png
%{_datadir}/icons/hicolor/22x22/apps/transmission-remote-gtk.png
%{_datadir}/icons/hicolor/24x24/apps/transmission-remote-gtk.png
%{_datadir}/icons/hicolor/32x32/apps/transmission-remote-gtk.png
%{_datadir}/icons/hicolor/48x48/apps/transmission-remote-gtk.png

%changelog
* Mon Feb 07 2011 Alan Fitton <alan@eth0.org.uk> - 0.1.1
- Squash a couple of nasty first release bugs.

* Sun Jan 30 2011 Alan Fitton <alan@eth0.org.uk> - 0.1.0
- Initial RPM Build.
