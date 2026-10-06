#!/bin/bash
# RT64/plume gate their Windows code on _WIN64; for a 32-bit (i686) build switch those to _WIN32.
cd ${TP:-$HOME/thirdparty}/rt64/src
for f in contrib/plume/plume_vulkan.cpp contrib/plume/plume_render_interface_types.h contrib/plume/plume_vulkan.h \
         common/rt64_user_configuration.cpp hle/rt64_application.cpp hle/rt64_application_window.cpp; do
  sed -i -E 's/(#\s*(if|ifdef|elif)\b.*)\b_WIN64\b/\1_WIN32/' $f
done
# 32-bit Vulkan: non-dispatchable handles are uint64_t, nullptr does not convert.
sed -i 's/vkCreateRayTracingPipelinesKHR(device->vk, nullptr, nullptr, 1/vkCreateRayTracingPipelinesKHR(device->vk, VK_NULL_HANDLE, VK_NULL_HANDLE, 1/' contrib/plume/plume_vulkan.cpp
grep -rn '_WIN64' contrib/plume/plume_vulkan.cpp contrib/plume/plume_render_interface_types.h contrib/plume/plume_vulkan.h common/rt64_user_configuration.cpp hle/rt64_application.cpp hle/rt64_application_window.cpp
# x86 stdcall: the header declaration lacks CALLBACK.
sed -i 's/static LRESULT windowHookCallback(/static LRESULT CALLBACK windowHookCallback(/' hle/rt64_application_window.h
echo done-win32-patch
