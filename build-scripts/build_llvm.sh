#!/bin/bash

cd "$(dirname "$0")"

if [ "$#" -ne 1 ]; then
  >&2 echo "Need a version number."
  exit 1
fi

echo "Trying to build LLVM $1..."
cd ../libzips/
if [ ! -f llvm-$1.src.tar.xz ]; then
  echo "Couldn't find zipped source. Downloading..."
  curl -L -C - -O "http://www.llvm.org/releases/$1/llvm-$1.src.tar.xz"
  if [[ $? -ne 0 ]]; then
    2&> echo "Couldn't download LLVM. :-("
    exit 1
  fi
fi

xzcat llvm-$1.src.tar.xz | tar xf -
cd llvm-$1.src
mkdir -p build
cd build

if [ `uname` == 'Darwin' ]; then
  config_options="-DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET -DCMAKE_OSX_SYSROOT=$SDKROOT "
else
  config_options=""
fi

REQUIRES_RTTI=1 cmake $config_options \
  -G "Unix Makefiles" .. \
  -DLLVM_ENABLE_LIBCXX=ON \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DCMAKE_INSTALL_PREFIX=$PREFIX \
  -DLLVM_TARGETS_TO_BUILD="host" \
  -DLLVM_ENABLE_RTTI=ON \
  -DLLVM_REQUIRES_RTTI=ON \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_STATIC_LIBS=ON \
  -DLLVM_ENABLE_FFI=OFF \
  -DLLVM_BINDINGS_LIST=none \
  -DLLVM_ENABLE_PEDANTIC=OFF \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_ENABLE_BACKTRACES=OFF \
  -DLLVM_ENABLE_TERMINFO=OFF \
  -DLLVM_ENABLE_ZLIB=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_ASSERTIONS=OFF \
  -DLLVM_INCLUDE_EXAMPLES=OFF


make -j$MK_JOBS; make install
