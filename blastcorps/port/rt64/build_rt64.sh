#!/bin/bash
# Build RT64 (github.com/rt64/rt64, MIT) for the windowed port (bc.exe), cross-compiled with
# mingw-w64 from WSL as a static 32-bit library.  Called by `make -C port game`; safe to rerun
# (does nothing when the build is up to date).
#
#   bash port/rt64/build_rt64.sh            -> $TP/rt64_bc/build-i686/rt64.a (+ deps), $TP/rt64_bc/dll/*.dll
#
# Third-party sources live outside the repo, in $TP (default ~/thirdparty):
#   $TP/rt64_bc/src         RT64 at RT64_COMMIT with the patches in port/rt64/patches applied
#   $TP/DirectX-Headers     Microsoft DirectX-Headers (MIT), for mingw's D3D12 GUIDs
#   $TP/dxc-release         DXC $DXC_TAG: the Linux dxc compiles the shaders at build time and the
#                           Windows x86 dxcompiler.dll/dxil.dll run next to bc.exe.  Both must be the
#                           same version: RT64 links shader libraries at run time.
# Patches (port/rt64/patches, applied in order; all small and upstreamable):
#   0001 mingw/i686 build (CMake for a Linux host, _WIN64 -> _WIN32, SAL/GUID shims, thread names)
#   0002 the same for plume (src/contrib/plume)
#   0003 RT64_NATIVE_HOST_LAYOUT: RDRAM is the port's own memory (src/common/rt64_rdram_layout.h)
#   0004 the Fast3D line ucode (L3D, the front end's globe) and later fixes
#
# Needs: git, cmake, ninja, python3, curl, g++ (host), i686-w64-mingw32-g++-posix (gcc 13).
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
TP=${TP:-$HOME/thirdparty}
JOBS=${JOBS:-2}
ARCH=i686
WA=x86
RT64_URL=https://github.com/rt64/rt64.git
RT64_COMMIT=43373749dac9bbc1b653e6a02aed40a9e1783bed   # 2026-09-02
DXC_TAG=v1.9.2609
DXC_ZIP=dxc_2026_09_29.zip
DXC_LINUX=linux_dxc_2026_09_28.x86_x64.tar.gz
W=$TP/rt64_bc
SRC=$W/src
B=$W/build-$ARCH
SH=$W/shim
mkdir -p $W $SH

# --- sources at the pinned commit, patched -------------------------------------------------------
STAMP=$( (echo $RT64_COMMIT; cat $HERE/patches/*.patch) | sha1sum | cut -c1-16)
if [ "$(cat $SRC/.bc_stamp 2>/dev/null)" != "$STAMP" ]; then
  echo "rt64: preparing $SRC at $RT64_COMMIT + patches"
  [ -d $SRC/.git ] || git clone -q $RT64_URL $SRC
  git -C $SRC cat-file -e $RT64_COMMIT^{commit} 2>/dev/null || git -C $SRC fetch -q origin
  git -C $SRC checkout -q -f --detach $RT64_COMMIT
  git -C $SRC submodule -q sync --recursive
  git -C $SRC submodule update -q --init --recursive -f
  git -C $SRC submodule -q foreach --recursive 'git reset -q --hard; git clean -q -fdx'
  git -C $SRC clean -q -fdx
  # each patch becomes a local commit ("bc: 000N"), so later edits diff cleanly against them
  bc_commit() { git -C "$1" -c user.name=bc -c user.email=bc@localhost commit -q -a --no-verify -m "bc: $2"; }
  git -C $SRC/src/contrib/plume apply $HERE/patches/0002-plume-mingw-i686.patch
  bc_commit $SRC/src/contrib/plume 0002
  for p in 0001-mingw-i686 0003-native-host-layout 0004-l3d-and-fixes; do
    git -C $SRC apply $HERE/patches/$p.patch
    git -C $SRC add -A -- . ':!src/contrib'
    bc_commit $SRC ${p%%-*}
  done
  rm -rf $B
  echo $STAMP > $SRC/.bc_stamp
fi

# --- DXC (one version for the build-time DXIL and the run-time DLLs) -------------------------------
[ -d $TP/DirectX-Headers ] || git clone -q --depth 1 https://github.com/microsoft/DirectX-Headers.git $TP/DirectX-Headers
D=$TP/dxc-release; mkdir -p $D/linux
if [ ! -f $D/bin/$WA/dxcompiler.dll ]; then
  curl -sL -o $D/dxc.zip https://github.com/microsoft/DirectXShaderCompiler/releases/download/$DXC_TAG/$DXC_ZIP
  python3 - "$D" <<'EOF'
import sys, zipfile, os
d = sys.argv[1]
z = zipfile.ZipFile(d + '/dxc.zip')
for i in z.infolist():
    n = i.filename.replace('\\', '/')
    if n.endswith('/'): continue
    os.makedirs(os.path.dirname(os.path.join(d, n)) or d, exist_ok=True)
    open(os.path.join(d, n), 'wb').write(z.read(i))
EOF
fi
if [ ! -x $D/linux/bin/dxc ]; then
  curl -sL -o $D/linux/dxc.tgz https://github.com/microsoft/DirectXShaderCompiler/releases/download/$DXC_TAG/$DXC_LINUX
  tar xzf $D/linux/dxc.tgz -C $D/linux
fi

# --- shims: case-sensitive header names, SAL macros, D3D12 GUIDs for GCC ---------------------------
echo '#include <windows.h>' > $SH/Windows.h
echo '#include <shlwapi.h>' > $SH/Shlwapi.h
echo '#include <shellscalingapi.h>' > $SH/ShellScalingAPI.h
echo '#include <shlobj.h>' > $SH/Shlobj.h
printf '#pragma once\n#include_next <d3d12.h>\n#include <dxguids.h>\n' > $SH/d3d12.h
{
  echo '#pragma once'; echo '#ifdef __cplusplus'; echo '#include <sal.h>'; echo '#endif'
  for m in _COM_Outptr_ _COM_Outptr_opt_ _COM_Outptr_opt_result_maybenull_ _COM_Outptr_result_maybenull_ _In_ _In_opt_ \
           _In_opt_z_ _In_z_ _Maybenull_ _Out_ _Outptr_opt_result_z_ _Outptr_result_nullonfailure_ _Outptr_result_z_; do
    echo "#ifndef $m"; echo "#define $m"; echo "#endif"; done
  for m in _In_bytecount_ _In_count_ _In_opt_count_; do echo "#ifndef $m"; echo "#define $m(x)"; echo "#endif"; done
  echo '#define _declspec __declspec'
} > $SH/rt64_mingw_compat.h.new
cmp -s $SH/rt64_mingw_compat.h.new $SH/rt64_mingw_compat.h || mv $SH/rt64_mingw_compat.h.new $SH/rt64_mingw_compat.h
rm -f $SH/rt64_mingw_compat.h.new
[ -x $W/file_to_c ] || g++ -O2 -std=c++17 -o $W/file_to_c $SRC/src/tools/file_to_c/file_to_c.cpp

# --- configure + build (static) -------------------------------------------------------------------
cat > $W/mingw-$ARCH.cmake <<EOF
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR $ARCH)
set(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF
DXH="-I$SH -I$TP/DirectX-Headers/include/directx -I$TP/DirectX-Headers/include/dxguids -I$TP/DirectX-Headers/include"
COMMON="-D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00 -D__REQUIRED_RPCNDR_H_VERSION__=475 -Wno-attributes -Wno-unknown-pragmas"
if [ ! -f $B/build.ninja ]; then
  cmake -S $SRC -B $B -G Ninja -DCMAKE_TOOLCHAIN_FILE=$W/mingw-$ARCH.cmake -DCMAKE_BUILD_TYPE=Release \
    -DHOST_FILE_TO_C=$W/file_to_c -DRT64_WIN_ARCH=$WA -DRT64_STATIC=ON -DRT64_HOST_DXC_DIR=$D/linux \
    -DRT64_NATIVE_HOST_LAYOUT=ON \
    -DCMAKE_CXX_FLAGS="-DImTextureID=ImU64 -include $SH/rt64_mingw_compat.h $COMMON $DXH" -DCMAKE_C_FLAGS="-I$SH" \
    > $W/cmake-$ARCH.log
fi
ninja -C $B -j$JOBS rt64

# --- run-time DLLs next to the exe: dxcompiler.dll + dxil.dll (DXC), SDL2.dll (zlib licence) --------
mkdir -p $W/dll
cp -u $D/bin/$WA/dxcompiler.dll $D/bin/$WA/dxil.dll $SRC/src/contrib/mupen64plus-win32-deps/SDL2-2.26.3/lib/$WA/SDL2.dll $W/dll/
printf "LIBRARY dxcompiler.dll\nEXPORTS\nDxcCreateInstance@12\nDxcCreateInstance2@16\n" > $W/dll/dxcompiler.def
[ -f $W/dll/libdxcompiler.a ] || $ARCH-w64-mingw32-dlltool -k -d $W/dll/dxcompiler.def -l $W/dll/libdxcompiler.a
echo "rt64: $B/rt64.a ready"
