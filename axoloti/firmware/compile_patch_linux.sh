#!/bin/sh
if ! command -v make &> /dev/null; then
    # If make isn't available, try the bundled version.
    echo ${external}/make/linux/usr/bin/make
    ${external}/make/linux/usr/bin/make -f Makefile.patch $1
else
    # If make is available, just use it.
    make -f Makefile.patch $1
fi
