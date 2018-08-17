#!/bin/bash

cd "$(dirname "$0")"
BUILD_SCRIPT_DIR=$(pwd)

if [ "$#" -ne 1 ]; then
  >&2 echo "Need a version number."
  exit 1
fi

echo "Trying to build GLU $1..."
cd ../libzips/
if [ ! -f glu-$1.tar.bz2 ]; then
  echo "Couldn't find zipped source. Downloading..."
  curl -L -C - -O "ftp://ftp.freedesktop.org/pub/mesa/glu/glu-$1.tar.bz2"
  if [[ $? -ne 0 ]]; then
    2&> echo "Couldn't download GLU. :-("
    exit 1
  fi
fi

tar -xjf glu-$1.tar.bz2

cd glu-$1

PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig ./configure \
      --disable-dependency-tracking \
      --enable-static \
      --disable-shared \
      --enable-osmesa \
      --prefix=$PREFIX

make -j$MK_JOBS; make install
