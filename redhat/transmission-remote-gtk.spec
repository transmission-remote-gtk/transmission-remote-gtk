Name:           transmission-remote-gtk
Version:        1.1
Release:        1%{?dist}
Summary:        Remotely manage the Transmission BitTorrent client

License:        GPLv2+
URL:            http://code.google.com/p/%{name}
Source0:        http://%{name}.googlecode.com/files/%{name}-%{version}.tar.gz

BuildRequires:  gettext
BuildRequires:  json-glib-devel
BuildRequires:  intltool
BuildRequires:  gtk2-devel
BuildRequires:  libproxy-devel
BuildRequires:  glib2-devel
BuildRequires:  unique-devel
BuildRequires:  libcurl-devel
BuildRequires:  libnotify-devel
BuildRequires:  GeoIP-devel

Requires(post):  info
Requires(preun): info

%description
transmission-remote-gtk is a GTK client for remote management of
the Transmission BitTorrent client using its HTTP RPC protocol.

%prep
%setup -q
for i in README COPYING AUTHORS ChangeLog; do
  sed -i "s|\r||g" "$i";
done

%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%find_lang %{name}

%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

%preun
if [ $1 = 0 ] ; then
  /sbin/install-info --delete %{_infodir}/%{name}.info %{_infodir}/dir || :
fi

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%files -f %{name}.lang
%{_bindir}/%{name}
%{_datadir}/applications/*.desktop
%{_datadir}/icons/*


%doc README COPYING AUTHORS ChangeLog
%_mandir/man1/transmission-remote-gtk.1.gz


%changelog
* Fri Jun 29 2012 Alan Fitton <alan@eth0.org.uk> 1.0.2-1
- New release

* Fri Jan 15 2012 Alan Fitton <alan@eth0.org.uk> 1.0-1
- New release.

* Fri Dec 09 2011 Alan Fitton <alan@eth0.org.uk> 0.8-1
- New release.

* Thu Nov 20 2011 Praveen Kumar <kumarpraveen.nitdgp@gmail.com> 0.7-3
- Minor changes according to review

* Thu Oct 27 2011 Praveen Kumar <kumarpraveen.nitdgp@gmail.com> 0.7-2
- Added icon cache

* Tue Oct 18 2011 Praveen Kumar <kumarpraveen.nitdgp@gmail.com> 0.7-1
- Initial version of the package
