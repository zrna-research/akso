#!/bin/sh
if ! command -v make &> /dev/null; then
    # If make isn't available, try the bundled version.
    ${external}/make/mac/make -f Makefile.patch $1
else
    # If make is available, just use it.
    make -f Makefile.patch $1
fi
