# virglrenderer for iSH-AOK

This is the `ish-aok` branch of a fork of
[virglrenderer](https://gitlab.freedesktop.org/virgl/virglrenderer), used by
[iSH-AOK](https://github.com/emkey1/ish-AOK) as `deps/virglrenderer`.

iSH-AOK builds only the Venus renderer (`-Dvrend=false -Dvenus=true`) and calls
its `vkr_renderer_*` API in the same process -- no render server -- from the
kernel's virtio-gpu render node, `/dev/dri/renderD128` (`fs/virtgpu.c`), with
MoltenVK as the host Vulkan. `tools/build-gpu-renderer.sh` in iSH-AOK builds it
for the Mac CLI and for iOS.

On top of upstream 8167744:

- `vkr_metal_helpers.m` includes `vulkan/vulkan_metal.h`; the Darwin build did
  not compile otherwise.
- `os_create_anonymous_file` falls back to an unlinked temp file when
  `shm_open` is refused, as an iOS app's sandbox may.
- `subprojects/venus-protocol-1.1.3` is committed (upstream fetches it at
  configure time), so a build needs no network.

`main` tracks upstream unchanged.
