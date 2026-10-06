#!/usr/bin/env python3
# Patch RT64's CMakeLists.txt (evaluation copy in ~/thirdparty) for a mingw-w64 cross build from Linux.
import re, sys
p = sys.argv[1]
s = open(p).read()
orig = s

# 1. Shader compiler: when the host is Linux, run dxc-linux even if the target is WIN32 (libdxil.so signs DXIL).
s = s.replace('if (WIN32)\n    set (DXC "${PROJECT_SOURCE_DIR}/src/contrib/dxc/bin/x64/dxc.exe")',
              'if (WIN32)\n    if (CMAKE_HOST_UNIX)\n        set (DXC "LD_LIBRARY_PATH=${PROJECT_SOURCE_DIR}/src/contrib/dxc/lib/x64" "${PROJECT_SOURCE_DIR}/src/contrib/dxc/bin/x64/dxc-linux")\n    else()\n        set (DXC "${PROJECT_SOURCE_DIR}/src/contrib/dxc/bin/x64/dxc.exe")\n    endif()', 1)

# 2. file_to_c must run on the host: use a prebuilt host binary when cross compiling.
s = s.replace('add_subdirectory(src/tools/file_to_c)',
              'if (CMAKE_CROSSCOMPILING)\n    add_executable(file_to_c IMPORTED)\n    set_property(TARGET file_to_c PROPERTY IMPORTED_LOCATION "${HOST_FILE_TO_C}")\nelse()\n    add_subdirectory(src/tools/file_to_c)\nendif()', 1)

# 3. MSVC-only link names -> mingw names; drop delayimp and SDL2main.
s = s.replace('        delayimp.lib\n', '')
s = s.replace('Shcore.lib', 'shcore')
s = s.replace('set(SDL2_LIBRARIES "SDL2" "SDL2main")', 'set(SDL2_LIBRARIES "SDL2")')
s = s.replace('link_directories("${PROJECT_SOURCE_DIR}/src/contrib/mupen64plus-win32-deps/SDL2-2.26.3/lib/x64")',
              'link_directories("${PROJECT_SOURCE_DIR}/src/contrib/mupen64plus-win32-deps/SDL2-2.26.3/lib/${RT64_WIN_ARCH}")')

# 4. Skip the texture tools (not needed by a port).
s = s.replace('add_subdirectory(src/tools/texture_hasher)\nadd_subdirectory(src/tools/texture_packer)', '# texture tools skipped for cross build')

assert s != orig
open(p, 'w').write(s)
print('patched')
