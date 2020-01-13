#!/bin/bash -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd "${DIR}"

rm -rf build
mkdir -p build

rgbasm -o build/test.o test.asm
rgblink -o build/test.gb -m build/test.gb.map -n build/test.gb.sym build/test.o
rgbfix -v -p 0 build/test.gb