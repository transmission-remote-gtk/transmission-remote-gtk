# ABOUT

transmission-remote-gtk is a GTK client for remote management of
the Transmission BitTorrent client, using its HTTP RPC protocol.


# DEPENDENCIES

The following packages are required dependencies (debian/ubuntu).

```bash
libgtk-3-dev automake autoconf gcc libgeoip-dev gettext autoconf-archive libappstream-dev appstream-util libcurl4-openssl-dev libjson-glib-dev
```

# BUILDING

Optionally install `libgeoip` and its headers to see the country
origin of peers.


##  Building a transmission-remote-gtk release from the command line:

```bash
$ tar zxfv transmission-remote-gtk-1.2.tar.gz
$ cd transmission-remote-gtk-1.2
$ ./configure
$ make
# make install
```

##  Building from git clone:

```bash
$ git clone https://github.com/transmission-remote-gtk/transmission-remote-gtk.git
$ ./autogen.sh
$ ./configure
$ make
# make install
```


## LICENSE

transmission-remote-gtk is released under GNU GPLv2.
See the [COPYING](./COPYING) file for details.

