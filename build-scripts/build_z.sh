#!/bin/bash

cd "$(dirname "$0")"
BUILD_SCRIPT_DIR=$(pwd)

if [ "$#" -ne 1 ]; then
  >&2 echo "Need a version number."
  exit 1
fi

echo "Trying to build libz $1..."
cd ../libzips/
if [ ! -f zlib-$1.tar.gz ]; then
  echo "Couldn't find zipped source. Downloading..."
  curl -L -C - -O "http://www.zlib.net/zlib-$1.tar.gz"
  if [[ $? -ne 0 ]]; then
    2&> echo "Couldn't download zlib. :-("
    exit 1
  fi
fi

tar -xzf zlib-$1.tar.gz

cd zlib-$1

./configure --prefix=$PREFIX --static

make -j$MK_JOBS; make install
