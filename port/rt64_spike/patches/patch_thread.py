#!/usr/bin/env python3
import sys
p = sys.argv[1]
s = open(p).read()
if 'RT64_MINGW_PATCH' in s:
    print('already'); sys.exit(0)
a = '#if defined(_WIN64)\n#   include <Windows.h>'
assert a in s
s = s.replace(a, '#if defined(_WIN32) // RT64_MINGW_PATCH: was _WIN64 (32-bit Windows build)\n#   include <Windows.h>', 1)
b = '        SetThreadDescription(GetCurrentThread(), nameWide.c_str());'
assert b in s
s = s.replace(b, '''#       ifdef __MINGW32__
        typedef HRESULT (WINAPI *SetThreadDescriptionFn)(HANDLE, PCWSTR);
        static SetThreadDescriptionFn fn = (SetThreadDescriptionFn)(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetThreadDescription");
        if (fn != nullptr) fn(GetCurrentThread(), nameWide.c_str());
#       else
        SetThreadDescription(GetCurrentThread(), nameWide.c_str());
#       endif''', 1)
open(p, 'w').write(s)
print('patched thread')
