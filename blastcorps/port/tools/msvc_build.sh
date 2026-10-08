#!/bin/bash
# Build the MSVC exes (port/CMakeLists.txt) from WSL, for the make targets in
# port/msvc.mk: the sources are mirrored to a folder on the Windows side (MSVC
# compiling over \\wsl.localhost is slow), configured once and built with
# CMake + Ninja in Visual Studio's x64 developer environment.
#
#   tools/msvc_build.sh DIR CONFIG [TARGET...]
#     DIR      a folder on a Windows drive (a WSL path: /mnt/c/...)
#     CONFIG   Release or Debug  -> DIR/CONFIG
#   environment: MSVC_PYTHON  a Windows python.exe (default: the one CMake finds)
#                MSVC_THIRDPARTY  BC_THIRDPARTY (a Windows path)
#                BC_VERSION   the version the exes report (default: CMake asks git)
#                JOBS         parallel compiles (default 2)
set -e
port=$(cd "$(dirname "$0")/.." && pwd)
root=$(cd "$port/.." && pwd)
dir=$1
cfg=$2
shift 2
case "$dir" in /mnt/[a-z]/*) ;; *) echo "msvc_build: $dir is not on a Windows drive" >&2; exit 1 ;; esac
[ -f "$port/inputs/addrs.txt" ] || { echo "msvc_build: no port/inputs (make -C port inputs)" >&2; exit 1; }
mkdir -p "$dir/src/blastcorps/port" "$dir/src/lib"
rsync -a --delete --exclude build --exclude __pycache__ "$root/src.us.v11" "$root/include" "$dir/src/blastcorps/"
# (port/inputs, the committed inputs folder, comes along)
rsync -a --delete --exclude /build --exclude /out --exclude __pycache__ "$port/" "$dir/src/blastcorps/port/"
rsync -aL --delete --exclude .git "$root/../lib/ultralib/" "$dir/src/lib/ultralib/"

vswhere="/mnt/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
[ -x "$vswhere" ] || { echo "msvc_build: no Visual Studio (vswhere.exe not found)" >&2; exit 1; }
vs=$("$vswhere" -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
     -property installationPath | tr -d '\r')
[ -n "$vs" ] || { echo "msvc_build: no Visual Studio with the x64 C++ tools" >&2; exit 1; }
wdir=$(wslpath -w "$dir")
b=$cfg
# (BC_INPUTS given each time: build folders configured before port/inputs was
# committed have the old default, port/build/inputs, in their cache)
defs="-DCMAKE_BUILD_TYPE=$cfg -DBC_INPUTS=$wdir\\src\\blastcorps\\port\\inputs"
[ -n "$MSVC_PYTHON" ] && defs="$defs -DPython3_EXECUTABLE=$MSVC_PYTHON"
[ -n "$MSVC_THIRDPARTY" ] && defs="$defs -DBC_THIRDPARTY=$MSVC_THIRDPARTY"
[ -n "$BC_VERSION" ] && defs="$defs -DBC_GIT_VERSION=$BC_VERSION"
tgt=""
[ $# -gt 0 ] && tgt="--target $*"
bat="$dir/build.bat"
{
  echo '@echo off'
  echo "call \"$vs\\Common7\\Tools\\VsDevCmd.bat\" -arch=amd64 -host_arch=amd64 -no_logo >nul 2>nul"
  echo "cd /d \"$wdir\""
  echo "if not exist $b\\build.ninja cmake -G Ninja -S src\\blastcorps\\port -B $b $defs || exit /b 1"
  echo "cmake $defs $b >nul || exit /b 1"
  echo "cmake --build $b -j ${JOBS:-2} $tgt"
} | sed 's/$/\r/' > "$bat"
cd "$dir"
cmd.exe /c "$(wslpath -w "$bat")" 2>&1 | grep -v "vswhere.exe' is not recognized\|^operable program or batch file"
exit "${PIPESTATUS[0]}"
