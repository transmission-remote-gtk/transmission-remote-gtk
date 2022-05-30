#!/bin/sh

if [ -z "$1" ]; then
    echo "Must provide a path to transmission-remote-gtk"
    exit 1
fi

export G_SLICE=always-malloc
export G_DEBUG=gc-friendly,resident-modules
valgrind \
    --tool=memcheck \
    --leak-check=full \
    --leak-resolution=high \
    --num-callers=20 \
    --log-file=valgrind-$$.log \
    --suppressions=/usr/share/glib-2.0/valgrind/glib.supp \
    --suppressions=/usr/share/gtk-3.0/valgrind/gtk.supp \
    "$1"
