#!/bin/sh -e
#
test -d m4 || mkdir m4
autoreconf -fi
rm -rf autom4te.cache
