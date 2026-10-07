#!/bin/bash
# THIRD_PARTY_LICENSES.txt for the player package (make -C port dist): every
# third-party component compiled into bc.exe or shipped next to it, with its
# licence text, gathered from the source trees the build used.
#   TP=~/thirdparty RT64_SRC=$TP/rt64_bc/src bash dist/third_party.sh > OUT
set -euo pipefail
TP=${TP:-$HOME/thirdparty}
RT64_SRC=${RT64_SRC:-$TP/rt64_bc/src}
C=$RT64_SRC/src/contrib
P=$C/plume/contrib

rule() { printf '\n%s\n' "================================================================================"; }
# component NAME WHAT FILE...: a header and the licence file(s)
component() {
    local name=$1 what=$2
    shift 2
    rule
    printf '%s\n%s\n' "$name" "$what"
    printf '%s\n\n' "--------------------------------------------------------------------------------"
    local f
    for f in "$@"; do
        [ -f "$f" ] || { echo "third_party.sh: missing $f" >&2; exit 1; }
        sed -e 's/\r$//' "$f"
        printf '\n'
    done
}
mit() {   # mit "Copyright line": the MIT licence text
    cat <<EOF
MIT License

$1

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
EOF
}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
mit "Copyright (c) 2013-2025 Niels Lohmann <https://nlohmann.me>" > "$TMP/json.txt"
mit "Copyright (C) 2016-2022 Giovanni Dicanio" > "$TMP/utf8conv.txt"
# miniz: the licence comment at the top of miniz.c
awk 'NR==1,/\*\//' "$C/miniz/miniz.c" | sed -n '/Copyright/,$p' | sed -e 's/^ \* \{0,1\}//; s/^ *\*\**\/$//' > "$TMP/miniz.txt"
# mingw-w64 runtime (CRT, winpthreads): Debian's copyright file, the parts linked
CR=/usr/share/doc/mingw-w64-common/copyright
awk '/^Files: \*$/{p=1} p&&/^Files: mingw-w64-crt\/cfguard/{exit} p' "$CR" > "$TMP/mingw_crt.txt"
awk '/^Files: mingw-w64-libraries\/winpthreads\/\*/{p=1} p&&/^Files: mingw-w64-libraries\/winpthreads\/tests_pthread/{exit} p' \
    "$CR" > "$TMP/winpthreads.txt"
# ... and the texts of the licences those stanzas name (stand-alone "License:" paragraphs)
for l in expat BSD-3-clause; do
    awk -v L="License: $l" 'BEGIN{RS=""} $0 ~ "^" L "\n" {print; print ""; exit}' "$CR" >> "$TMP/winpthreads.txt"
done
grep -q "Permission is hereby granted" "$TMP/winpthreads.txt" || { echo "third_party.sh: no expat text" >&2; exit 1; }
# GCC runtime (libgcc, libstdc++, linked statically): the GCC Runtime Library Exception
GC=/usr/share/doc/gcc-mingw-w64-base/copyright
awk '/^GCC RUNTIME LIBRARY EXCEPTION/{p=1} p{print} p&&/requirements of the license of GCC\./{exit}' "$GC" > "$TMP/gccrt.txt"
grep -q "requirements of the license of GCC" "$TMP/gccrt.txt" ||
    { echo "third_party.sh: no GCC runtime exception text in $GC" >&2; exit 1; }
for f in json utf8conv miniz mingw_crt winpthreads; do
    [ -s "$TMP/$f.txt" ] || { echo "third_party.sh: empty $f licence" >&2; exit 1; }
done

cat <<'EOF'
Third-party software in this package
====================================

bc.exe is built from the Blast Corps decompilation project's code (the game's
decompiled C code, the decompiled N64 SDK libraries it uses, and the code written
for this Windows port). It contains no game data: the game's data, graphics and
sounds are read from the player's own ROM at run time.

The following third-party components are compiled into bc.exe or shipped next to
it. Their licences follow, each in full.

  Component                       Licence                      Where
  RT64 (N64 renderer)             MIT                          bc.exe
  plume (render hardware layer)   MIT                          bc.exe
  re-spirv                        MIT                          bc.exe
  SPIRV-Headers                   MIT-style (Khronos)          bc.exe
  D3D12 Memory Allocator          MIT                          bc.exe
  Vulkan Memory Allocator         MIT                          bc.exe
  volk                            MIT                          bc.exe
  Vulkan-Headers                  Apache-2.0 or MIT            bc.exe
  DirectX-Headers                 MIT                          bc.exe
  Dear ImGui                      MIT                          bc.exe
  ImPlot                          MIT                          bc.exe
  im3d                            MIT                          bc.exe
  HLSL++                          MIT                          bc.exe
  ddspp                           MIT                          bc.exe
  stb                             MIT or public domain         bc.exe
  xxHash                          BSD 2-Clause                 bc.exe
  Zstandard                       BSD 3-Clause                 bc.exe
  miniz                           MIT                          bc.exe
  JSON for Modern C++             MIT                          bc.exe
  utf8conv                        MIT                          bc.exe
  Native File Dialog Extended     zlib                         bc.exe
  mingw-w64 runtime, winpthreads  ZPL-2.1 / MIT / BSD          bc.exe
  GCC runtime (libgcc, libstdc++) GPL-3.0 with the GCC Runtime bc.exe
                                  Library Exception
  SDL2 2.26.3                     zlib                         SDL2.dll
  DirectX Shader Compiler         LLVM Release License (NCSA)  dxcompiler.dll
                                  and MIT
  DirectX Shader Compiler (DXIL)  Microsoft Software License   dxil.dll
                                  Terms (redistributable)
EOF

component "RT64" "https://github.com/rt64/rt64 (commit 43373749, with the port's patches)" "$RT64_SRC/LICENSE"
component "plume" "https://github.com/renderbag/plume" "$C/plume/LICENSE"
component "re-spirv" "https://github.com/renderbag/re-spirv" "$C/re-spirv/LICENSE"
component "SPIRV-Headers" "https://github.com/KhronosGroup/SPIRV-Headers" "$C/re-spirv/external/SPIRV-Headers/LICENSE"
component "D3D12 Memory Allocator" "https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator" \
    "$P/D3D12MemoryAllocator/LICENSE.txt"
component "Vulkan Memory Allocator" "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator" \
    "$P/VulkanMemoryAllocator/LICENSE.txt"
component "volk" "https://github.com/zeux/volk" "$P/volk/LICENSE.md"
component "Vulkan-Headers" "https://github.com/KhronosGroup/Vulkan-Headers (used under the MIT licence)" \
    "$P/Vulkan-Headers/LICENSE.md" "$P/Vulkan-Headers/LICENSES/MIT.txt"
component "DirectX-Headers" "https://github.com/microsoft/DirectX-Headers" "$TP/DirectX-Headers/LICENSE"
component "Dear ImGui" "https://github.com/ocornut/imgui" "$C/imgui/LICENSE.txt"
component "ImPlot" "https://github.com/epezent/implot" "$C/implot/LICENSE"
component "im3d" "https://github.com/john-chapman/im3d" "$C/im3d/LICENSE"
component "HLSL++" "https://github.com/redorav/hlslpp" "$C/hlslpp/LICENSE"
component "ddspp" "https://github.com/redorav/ddspp" "$C/ddspp/LICENSE"
component "stb" "https://github.com/nothings/stb" "$C/stb/LICENSE"
component "xxHash" "https://github.com/Cyan4973/xxHash" "$C/xxHash/LICENSE"
component "Zstandard" "https://github.com/facebook/zstd (used under the BSD licence)" "$C/zstd/LICENSE"
component "miniz" "https://github.com/richgel999/miniz" "$TMP/miniz.txt"
component "JSON for Modern C++" "https://github.com/nlohmann/json (version 3.12.0)" "$TMP/json.txt"
component "utf8conv" "https://github.com/GiovanniDicanio/Utf8Conv" "$TMP/utf8conv.txt"
component "Native File Dialog Extended" "https://github.com/btzy/nativefiledialog-extended" \
    "$C/nativefiledialog-extended/LICENSE"
component "mingw-w64 runtime" "https://www.mingw-w64.org (C runtime startup and support code)" "$TMP/mingw_crt.txt"
component "winpthreads" "https://www.mingw-w64.org (POSIX threads for Windows)" "$TMP/winpthreads.txt"
component "GCC runtime libraries (libgcc, libstdc++)" \
    "https://gcc.gnu.org - GPL-3.0 with the GCC Runtime Library Exception, which permits distributing programs compiled with GCC under any terms. The exception:" \
    "$TMP/gccrt.txt"
component "SDL2 2.26.3 (SDL2.dll)" "https://www.libsdl.org" "$C/mupen64plus-win32-deps/SDL2-2.26.3/COPYING.txt"
component "DirectX Shader Compiler (dxcompiler.dll)" "https://github.com/microsoft/DirectXShaderCompiler (release v1.9.2609)" \
    "$TP/dxc-release/LICENSE-LLVM.txt" "$TP/dxc-release/LICENCE-MIT.txt"
component "DirectX Shader Compiler: DXIL signing library (dxil.dll)" \
    "Microsoft, redistributable with applications under these terms" "$TP/dxc-release/LICENSE-MS.txt"
