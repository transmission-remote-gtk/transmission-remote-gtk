# ABOUT

transmission-remote-gtk is a GTK client for remote management of
the Transmission BitTorrent client, using its HTTP RPC protocol.


# DEPENDENCIES

### Runtime/Build Required Dependencies
The following packages are required dependencies for building and running
transmission-remote-gtk:

 - gtk >= 3.22
 - glib >= 2.56 (including gio and gthread)
 - json-glib >= 1.2.8
 - libcurl
 - GNU gettext

An example command for getting the packages required for an Ubuntu/Debian
system:

```
# apt install libgtk-3-dev libgeoip-dev gettext libcurl4-openssl-dev libjson-glib-dev
```

### Optional Dependencies
The following packages are optional dependencies:

 - libmrss >= 0.18, RSS feed support
 - libproxy, HTTP proxy support
 - libgeoip, country of origin of peers
 - libappindicator, Application tray support

If these libraries are installed at build time they will be automatically
detected and linked for additional functionality.

### Build Only Dependencies
transmission-remote-gtk uses meson for its build system. A relatively new
version of meson is required.

 - meson >= 0.59.0
 - appstream-util (optional, appdata validation)
 - desktop-file-utils (optional, desktop file validation)
 - pod2man (optional, manpage generation)

An example of getting the build dependencies for an Ubuntu/Debian system:

```
# apt install gcc meson appstream-util perl
```

**NOTE:** Ubuntu and other stable or LTS distros may have outdated versions of
meson that will not work for compiling transmission-remote-gtk. Meson can also
be installed via pip:

```
# apt-get install python3-pip
$ pip install meson
```


# BUILDING

Building transmission-remote-gtk is simple. The following commands will clone
the repository, build transmission-remote-gtk, and install it on your system:

```
$ git clone https://github.com/transmission-remote-gtk/transmission-remote-gtk.git
$ cd transmission-remote-gtk
$ meson --prefix="$prefix" "$builddir"
$ meson compile -C "$builddir"
# meson install -C "$builddir"
```

[Tarball releases](https://github.com/transmission-remote-gtk/transmission-remote-gtk/releases)
are also available to download.

## LICENSE

transmission-remote-gtk is released under GNU GPLv2.
See the [COPYING](./COPYING) file for details.

## CONTRIBUTING

This project is under active development, and help is always appreciated regardless of skill level. Current
project goals and refactors are being tracked in [the Wiki](https://github.com/transmission-remote-gtk/transmission-remote-gtk/wiki/TODOs).
Take a look if you're interested in helping. Other PRs are also welcome!

## CONTACT INFO

Development happens on github (this repository). Issues and pull requests should be submitted here. If
you would like to discuss development or ask general questions about the project, you can try `#transmission-remote-gtk`
on [irc.libera.chat](ircs://irc.libera.chat:6697).
