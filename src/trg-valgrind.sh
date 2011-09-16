#!/bin/sh
G_SLICE=always-malloc G_DEBUG=gc-friendly,resident-modules valgrind --suppressions=gtk.suppression --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20 --log-file=valgrind.log ./transmission-remote-gtk
