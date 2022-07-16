#!/bin/bash
# Build and install for Arch linux.
set -e

mkdir build
pushd build
cmake ..
mkdir temp
mv PKGBUILD temp
cd temp
makepkg -f
yay -U *.pkg.*
popd
