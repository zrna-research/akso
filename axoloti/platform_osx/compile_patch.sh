#!/bin/sh
set -e
platformdir="$(cd $(dirname $0); pwd -P)"

"${axoloti_firmware}/compile_patch_osx.sh" "$@"
