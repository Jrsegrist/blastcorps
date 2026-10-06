#!/bin/bash
# Stage 0 spike: cross-build RT64 (github.com/rt64/rt64, MIT) with mingw-w64 from WSL and link
# the feasibility harness (harness.cpp). Third-party sources live outside the repo.
#
#   port/rt64_spike/build.sh [i686|x86_64]       -> $TP/rt64_work/run-<arch>/rt64_harness.exe
#   (run: cd $TP/rt64_work/run-<arch>; ./rt64_harness.exe <ucode dir> [d3d12|vulkan] [frames])
#
# The ucode dir holds hd_text.bin / hd_data.bin (and optionally fe_text.bin / fe_data.bin): the
# big-endian bytes of D_802E53F0 / D_8030E390 (D_80207090 / D_80210690) taken from the user's
# build (never commit them).
#
# Needs: cmake, ninja, python3, curl, {i686,x86_64}-w64-mingw32-g++-posix (gcc 13).
set -e
ARCH=${1:-i686}
HERE=$(cd "$(dirname "$0")" && pwd)
TP=${TP:-$HOME/thirdparty}
RT64_COMMIT=43373749dac9bbc1b653e6a02aed40a9e1783bed   # 2026-09-02, the evaluated revision
DXC_TAG=v1.9.2609
DXC_ZIP=dxc_2026_09_29.zip
DXC_LINUX=linux_dxc_2026_09_28.x86_x64.tar.gz
SRC=$TP/rt64; W=$TP/rt64_work; B=$W/build-$ARCH
[ $ARCH = x86_64 ] && WA=x64 || WA=x86
mkdir -p $TP $W/shim

# --- sources -------------------------------------------------------------------------------
[ -d $SRC ] || git clone --recurse-submodules https://github.com/rt64/rt64.git $SRC
git -C $SRC checkout -q $RT64_COMMIT && git -C $SRC submodule update --init --recursive -q
[ -d $TP/DirectX-Headers ] || git clone --depth 1 https://github.com/microsoft/DirectX-Headers.git $TP/DirectX-Headers
# One DXC version for both build-time DXIL (Linux dxc) and the runtime dxcompiler.dll/dxil.dll:
# RT64 links shader libraries at runtime and refuses mixed compiler versions.
D=$TP/dxc-release; mkdir -p $D/linux
if [ ! -f $D/bin/x86/dxcompiler.dll ]; then
  curl -sL -o $D/dxc.zip https://github.com/microsoft/DirectXShaderCompiler/releases/download/$DXC_TAG/$DXC_ZIP
  python3 - "$D" <<'EOF'
import sys, zipfile, os
d = sys.argv[1]
for i in zipfile.ZipFile(d + '/dxc.zip').infolist():
    n = i.filename.replace('\\', '/')
    if n.endswith('/'): continue
    os.makedirs(os.path.dirname(os.path.join(d, n)) or d, exist_ok=True)
    open(os.path.join(d, n), 'wb').write(zipfile.ZipFile(d + '/dxc.zip').read(i))
EOF
fi
if [ ! -x $D/linux/bin/dxc ]; then
  curl -sL -o $D/linux/dxc.tgz https://github.com/microsoft/DirectXShaderCompiler/releases/download/$DXC_TAG/$DXC_LINUX
  tar xzf $D/linux/dxc.tgz -C $D/linux
fi

# --- patches for mingw / 32-bit (idempotent) -------------------------------------------------
cd $SRC
python3 $HERE/patches/patch_cmake.py CMakeLists.txt 2>/dev/null || true   # asserts if already applied
python3 $HERE/patches/patch_dxcver.py CMakeLists.txt
python3 $HERE/patches/patch_plume.py src/contrib/plume/plume_d3d12.cpp
python3 $HERE/patches/patch_thread.py src/common/rt64_thread.cpp
python3 $HERE/patches/patch_dxc.py src/render/rt64_shader_compiler.h
bash $HERE/patches/patch_win32.sh
cd $SRC
sed -i 's/#elif defined(__GNUC__) \&\& (__GNUC__ >= 4) \&\& defined(__LP64__)$/#elif defined(__GNUC__) \&\& (__GNUC__ >= 4) \/* RT64_MINGW_PATCH: was \&\& defined(__LP64__) *\//' src/common/rt64_tmem_hasher.h

# --- shims: case-sensitive header names, SAL macros, D3D12 GUIDs for GCC --------------------
SH=$W/shim
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
} > $SH/rt64_mingw_compat.h
[ -x $W/file_to_c ] || g++ -O2 -std=c++17 -o $W/file_to_c src/tools/file_to_c/file_to_c.cpp

# --- configure + build RT64 (static) ---------------------------------------------------------
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
cmake -S $SRC -B $B -G Ninja -DCMAKE_TOOLCHAIN_FILE=$W/mingw-$ARCH.cmake -DCMAKE_BUILD_TYPE=Release \
  -DHOST_FILE_TO_C=$W/file_to_c -DRT64_WIN_ARCH=$WA -DRT64_STATIC=ON -DRT64_HOST_DXC_DIR=$D/linux \
  -DCMAKE_CXX_FLAGS="-DImTextureID=ImU64 -include $SH/rt64_mingw_compat.h $COMMON $DXH" -DCMAKE_C_FLAGS="-I$SH" > $W/cmake-$ARCH.log
ninja -C $B -j8 rt64

# --- harness -----------------------------------------------------------------------------------
OUT=$W/run-$ARCH; mkdir -p $OUT
R=$SRC
CXX=$ARCH-w64-mingw32-g++-posix
$CXX -DSDL_MAIN_HANDLED -DFFX_GCC -DHLSL_CPU -DIMGUI_IMPL_VULKAN_NO_PROTOTYPES -DNOMINMAX -D__PRFCHWINTRIN_H -DImTextureID=ImU64 \
  -include $SH/rt64_mingw_compat.h $COMMON \
  -I$R/src/contrib/mupen64plus-win32-deps/SDL2-2.26.3/include -I$R/src -I$R/src/contrib -I$R/src/contrib/plume -I$R/src/contrib/imgui \
  -I$R/src/contrib/hlslpp/include -I$R/src/contrib/dxc/inc -I$B/src -I$R/src/contrib/plume/contrib/volk \
  -I$R/src/contrib/plume/contrib/Vulkan-Headers/include -I$R/src/contrib/plume/contrib/VulkanMemoryAllocator/include \
  -I$R/src/contrib/plume/contrib/D3D12MemoryAllocator/include $DXH -O2 -std=gnu++17 \
  -c $HERE/harness.cpp -o $OUT/harness.o
cp $D/bin/$WA/dxcompiler.dll $D/bin/$WA/dxil.dll $R/src/contrib/mupen64plus-win32-deps/SDL2-2.26.3/lib/$WA/SDL2.dll $OUT/
printf "LIBRARY dxcompiler.dll\nEXPORTS\nDxcCreateInstance@12\nDxcCreateInstance2@16\n" > $OUT/dxcompiler.def
if [ $ARCH = i686 ]; then
  $ARCH-w64-mingw32-dlltool -k -d $OUT/dxcompiler.def -l $OUT/libdxcompiler.a; LAA=-Wl,--large-address-aware
else
  sed -i "s/@[0-9]*//" $OUT/dxcompiler.def; $ARCH-w64-mingw32-dlltool -d $OUT/dxcompiler.def -l $OUT/libdxcompiler.a; LAA=
fi
$CXX -o $OUT/rt64_harness.exe $OUT/harness.o $B/rt64.a $B/src/contrib/plume/libplume.a $B/src/contrib/re-spirv/libre-spirv.a \
  $B/src/contrib/nativefiledialog-extended/src/libnfd.a $B/src/contrib/zstd/build/cmake/lib/libzstd.a \
  $OUT/SDL2.dll $OUT/libdxcompiler.a -ld3d12 -ld3dcompiler -ldxgi -lshcore -ldxguid -lole32 -loleaut32 -luuid -lshell32 \
  -lshlwapi -lcomdlg32 -lgdi32 -luser32 -ldwmapi -limm32 -lversion -lsetupapi -lwinmm \
  -static -static-libgcc -static-libstdc++ $LAA
echo "built $OUT/rt64_harness.exe"
