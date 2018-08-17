#!/bin/bash

cd "$(dirname "$0")"
BUILD_SCRIPT_DIR=$(pwd)

if [ "$#" -ne 1 ]; then
  >&2 echo "Need a version number."
  exit 1
fi

echo "Trying to build OSMesa $1..."
cd ../libzips/
if [ ! -f mesa-$1.tar.gz ]; then
  echo "Couldn't find zipped source. Downloading..."
  curl -L -C - -O "ftp://ftp.freedesktop.org/pub/mesa/mesa-$1.tar.gz" || curl -L -C - -O "ftp://ftp.freedesktop.org/pub/mesa/$1/mesa-$1.tar.gz"
  if [[ $? -ne 0 ]]; then
    2&> echo "Couldn't download OSMesa. :-("
    exit 1
  fi
fi

tar -xzf mesa-$1.tar.gz

for p in $BUILD_SCRIPT_DIR/mesa-patches/*.patch; do
  echo "* Applying $p"
  patch -p1 -d $BUILD_SCRIPT_DIR/../libzips/mesa-$1 < $p
done

gles=""
for h in GLES/gl.h GLES2/gl2.h GLES3/gl3.h GLES3/gl31.h GLES3/gl32.h; do
  if [ -f $BUILD_SCRIPT_DIR/../libzips/mesa-$1/include/$h ]; then
    gles="$gles ../$h"
  fi
done
(cd $BUILD_SCRIPT_DIR/../libzips/mesa-$1/include/GL; sed -e 's@gl.h glext.h@gl.h glext.h '"$gles"'@' -e 's@\^GLAPI@^GL_\\?API@' -i.orig gl_mangle.h)
(cd $BUILD_SCRIPT_DIR/../libzips/mesa-$1/include/GL; sh ./gl_mangle.h > gl_mangle.h.new && mv gl_mangle.h.new gl_mangle.h)
sed -i.bak -e 's/MANGLE/MANGLE_disabled/' $BUILD_SCRIPT_DIR/../libzips/mesa-$1/src/mapi/glapi/glapi_getproc.c

cd $BUILD_SCRIPT_DIR/../libzips/mesa-$1


autoreconf -fi
./configure \
  --disable-dependency-tracking \
  --enable-static \
  --disable-shared \
  --enable-texture-float \
  --disable-gles1 \
  --disable-gles2 \
  --disable-dri \
  --disable-dri3 \
  --disable-glx \
  --disable-glx-tls \
  --disable-egl \
  --disable-gbm \
  --disable-xvmc \
  --disable-vdpau \
  --disable-omx \
  --disable-va \
  --disable-opencl \
  --disable-shared-glapi \
  --disable-driglx-direct \
  --with-dri-drivers= \
  --with-osmesa-bits=32 \
  --with-egl-platforms= \
  --prefix=$PREFIX \
  --disable-osmesa \
  --enable-gallium-osmesa \
  --enable-gallium-llvm=yes \
  --with-llvm-prefix=$PREFIX \
  --disable-llvm-shared-libs \
  --with-gallium-drivers=swrast \
  CC="$CC" CFLAGS="$CFLAGS" \
  CXX="$CXX" CXXFLAGS="$CXXFLAGS"

make -j$MK_JOBS; make install

if [ `uname` == 'Darwin' ]; then
  for f in $PREFIX/lib/libOSMesa*.a; do
    ranlib -c "$f"
  done
fi

