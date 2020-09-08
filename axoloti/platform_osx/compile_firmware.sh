#!/bin/sh
set -e
platformdir="$(cd $(dirname $0); pwd -P)"

cd "${axoloti_firmware}"
"${axoloti_firmware}/compile_firmware_osx.sh" $1
