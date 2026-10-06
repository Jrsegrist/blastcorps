#!/usr/bin/env python3
# MinGW ABI: D3D12 methods returning structs take an out-param (RetVal) under non-MSVC Windows compilers.
import sys
p = sys.argv[1]
s = open(p).read()
if 'RT64_MINGW_PATCH' in s:
    print('already'); sys.exit(0)
reps = [
 ('        cpuDescriptorHandle = heap->GetCPUDescriptorHandleForHeapStart();',
  '#ifdef __MINGW32__ // RT64_MINGW_PATCH\n        heap->GetCPUDescriptorHandleForHeapStart(&cpuDescriptorHandle);\n#else\n        cpuDescriptorHandle = heap->GetCPUDescriptorHandleForHeapStart();\n#endif'),
 ('            gpuDescriptorHandle = heap->GetGPUDescriptorHandleForHeapStart();',
  '#ifdef __MINGW32__\n            heap->GetGPUDescriptorHandleForHeapStart(&gpuDescriptorHandle);\n#else\n            gpuDescriptorHandle = heap->GetGPUDescriptorHandleForHeapStart();\n#endif'),
 ('            poolDesc.HeapProperties = device->d3d->GetCustomHeapProperties(0, D3D12_HEAP_TYPE_UPLOAD);',
  '#ifdef __MINGW32__\n            device->d3d->GetCustomHeapProperties(&poolDesc.HeapProperties, 0, D3D12_HEAP_TYPE_UPLOAD);\n#else\n            poolDesc.HeapProperties = device->d3d->GetCustomHeapProperties(0, D3D12_HEAP_TYPE_UPLOAD);\n#endif'),
]
for a, b in reps:
    assert a in s, a
    s = s.replace(a, b)
open(p, 'w').write(s)
print('patched plume')
