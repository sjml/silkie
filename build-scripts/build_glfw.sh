#!/bin/bash

cd "$(dirname "$0")"

if [ "$#" -ne 1 ]; then
  >&2 echo "Need a version number."
  exit 1
fi

echo "Trying to build GLFW $1..."
cd ../libzips/
if [ ! -f glfw-$1.zip ]; then
  echo "Couldn't find zipped source. Downloading..."
  curl -L -C - -O "https://github.com/glfw/glfw/releases/download/$1/glfw-$1.zip"
  if [[ $? -ne 0 ]]; then
    2&> echo "Couldn't download GLFW. :-("
    exit 1
  fi
fi

unzip glfw-$1.zip
cd glfw-$1
mkdir build
cd build

cmake .. \
  -DCMAKE_INSTALL_PREFIX=$PREFIX \
  -DBUILD_SHARED_LIBS=OFF \
  -DGLFW_BUILD_EXAMPLES=OFF \
  -DGLFW_BUILD_TESTS=OFF \
  -DGLFW_BUILD_DOCS=OFF

make -j$MK_JOBS; make install
