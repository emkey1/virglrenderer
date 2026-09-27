/*
 * Copyright 2020 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VKR_PHYSICAL_DEVICE_H
#define VKR_PHYSICAL_DEVICE_H

#include "vkr_common.h"

#include "vn_protocol_renderer_util.h"

struct vkr_physical_device {
   struct vkr_object base;

   struct vn_physical_device_proc_table proc_table;

   VkPhysicalDeviceProperties properties;
   uint32_t api_version;

   VkExtensionProperties *extensions;
   uint32_t extension_count;

   bool KHR_external_memory_fd;
   bool EXT_external_memory_dma_buf;
   bool KHR_portability_subset;

   bool KHR_external_fence_fd;
   bool KHR_external_semaphore_fd;
   /* iSH-AOK: sync-fd binary semaphores emulated for a host without them
    * (MoltenVK): advertised to the guest, never enabled on the host device,
    * and the renderer's two uses replayed as empty queue submissions. Venus
    * exposes VK_KHR_swapchain only when these import. */
   bool emulate_semaphore_sync_fd;
   /* iSH-AOK: see vkr_dma_buf_emul.h. */
   bool emulate_dma_buf;
   uint32_t host_visible_memory_type_bits;

   bool EXT_external_memory_metal;
   bool EXT_metal_objects;

   VkPhysicalDeviceMemoryProperties memory_properties;
   VkPhysicalDeviceIDProperties id_properties;
   bool is_dma_buf_fd_export_supported;
   bool is_opaque_fd_export_supported;
   void *gbm_device;
   int udmabuf_dev_fd;

   VkQueueFamilyProperties *queue_family_properties;
   uint32_t queue_family_property_count;

   struct list_head devices;
};
VKR_DEFINE_OBJECT_CAST(physical_device, VK_OBJECT_TYPE_PHYSICAL_DEVICE, VkPhysicalDevice)

void
vkr_context_init_physical_device_dispatch(struct vkr_context *ctx);

void
vkr_physical_device_destroy(struct vkr_context *ctx,
                            struct vkr_physical_device *physical_dev);

#endif /* VKR_PHYSICAL_DEVICE_H */
