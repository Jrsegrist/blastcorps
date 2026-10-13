# RT64 for bc.exe: the pinned commit and the port's patches (port/rt64/patches),
# applied in this order.  The one place both are listed: CMakeLists.txt reads
# it, and so does fetch_rt64.cmake when it runs on its own (tools/msvc_build.sh
# prepares the tree from WSL with it, so the Windows side needs no git).
#   0003  RDRAM in the port's own layout (RT64_NATIVE_HOST_LAYOUT)
#   0004  the L3D line microcode, and fixes
#   0005  vsync pacing: with vsync, pictures reach the display on its refreshes
#         (each held for the refreshes it is due) instead of RT64's present timer
#   0006  a host view hook per rendered picture (the orbit camera,
#         src/live/live_orbit.cpp)
# (0001/0002 served the retired mingw cross build.)
set(BC_RT64_COMMIT 43373749dac9bbc1b653e6a02aed40a9e1783bed)
set(BC_RT64_PATCH_NAMES
    0003-native-host-layout.patch
    0004-l3d-and-fixes.patch
    0005-vsync-pacing.patch
    0006-view-hook.patch
)
