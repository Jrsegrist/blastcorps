#!/usr/bin/env python3
# Shader DXIL must be built by the same DXC version as the dxcompiler.dll shipped at runtime
# (RT64 links shader libraries at runtime). Allow pointing the cross build at a separate Linux DXC.
import sys
p = sys.argv[1]
s = open(p).read()
if 'RT64_HOST_DXC_DIR' in s:
    print('already'); sys.exit(0)
a = '''    if (CMAKE_HOST_UNIX)
        set (DXC "LD_LIBRARY_PATH=${PROJECT_SOURCE_DIR}/src/contrib/dxc/lib/x64" "${PROJECT_SOURCE_DIR}/src/contrib/dxc/bin/x64/dxc-linux")'''
assert a in s
s = s.replace(a, '''    if (CMAKE_HOST_UNIX AND RT64_HOST_DXC_DIR)
        set (DXC "LD_LIBRARY_PATH=${RT64_HOST_DXC_DIR}/lib" "${RT64_HOST_DXC_DIR}/bin/dxc")
''' + a.replace('    if (CMAKE_HOST_UNIX)', '    elseif (CMAKE_HOST_UNIX)'), 1)
open(p, 'w').write(s)
print('patched dxc host dir')
