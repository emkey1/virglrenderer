/*
 * SPDX-License-Identifier: MIT
 */

/* iSH-AOK: dma-buf external memory and DRM format modifiers for a host
 * without them (MoltenVK).
 *
 * Mesa's Venus before 26 offers the guest any WSI only when the renderer has
 * VK_EXT_external_memory_dma_buf, VK_EXT_image_drm_format_modifier and
 * VK_EXT_queue_family_foreign, and a compositor sharing buffers with a client
 * needs them anyway. They are advertised to the guest and never enabled on
 * the host device (see emulate_dma_buf), and:
 *
 *  - the only modifier is DRM_FORMAT_MOD_LINEAR, and an image created with it
 *    is a VK_IMAGE_TILING_LINEAR image;
 *  - dma-buf memory is host-visible memory, which on this host is shared
 *    memory wrapped as an MTLBuffer and exported as a SHM resource; a dma-buf
 *    image or buffer is steered to those memory types;
 *  - importing a SHM resource wraps its mapping as an MTLBuffer;
 *  - a foreign queue family needs nothing: MoltenVK has no queue family
 *    ownership to transfer.
 */

#ifndef VKR_DMA_BUF_EMUL_H
#define VKR_DMA_BUF_EMUL_H

#include "vkr_common.h"

#include "vkr_physical_device.h"

#define VKR_DRM_FORMAT_MOD_LINEAR 0ull

static inline void
vkr_remove_struct(void *chain, VkStructureType type)
{
   void *prev = vkr_find_prev_struct(chain, type);
   if (prev)
      vkr_pnext_set_next(prev, vkr_pnext_get_next(vkr_pnext_get_next(prev)));
}

static inline void
vkr_emul_external_memory_properties(VkExternalMemoryProperties *props)
{
   props->externalMemoryFeatures = VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT |
                                   VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
   props->exportFromImportedHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
   props->compatibleHandleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
}

/* Rewrite a VkImageCreateInfo for the host. Sets *is_dma_buf when the image
 * is meant to share memory, whose types must then be host-visible.
 */
static inline VkResult
vkr_emul_fix_image_create_info(const struct vkr_physical_device *physical_dev,
                               VkImageCreateInfo *info,
                               bool *is_dma_buf)
{
   *is_dma_buf = false;
   if (!physical_dev->emulate_dma_buf)
      return VK_SUCCESS;

   VkExternalMemoryImageCreateInfo *ext =
      vkr_find_struct(info->pNext, VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);
   if (ext && (ext->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT)) {
      *is_dma_buf = true;
      vkr_remove_struct(info, VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);
   }

   if (info->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
      const VkImageDrmFormatModifierListCreateInfoEXT *list = vkr_find_struct(
         info->pNext, VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT);
      const VkImageDrmFormatModifierExplicitCreateInfoEXT *explicit = vkr_find_struct(
         info->pNext, VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT);
      bool linear = false;
      if (list) {
         for (uint32_t i = 0; i < list->drmFormatModifierCount; i++)
            linear |= list->pDrmFormatModifiers[i] == VKR_DRM_FORMAT_MOD_LINEAR;
      } else if (explicit) {
         linear = explicit->drmFormatModifier == VKR_DRM_FORMAT_MOD_LINEAR;
      }
      if (!linear)
         return VK_ERROR_FORMAT_NOT_SUPPORTED;
      vkr_remove_struct(info, VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT);
      vkr_remove_struct(info,
                        VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT);
      info->tiling = VK_IMAGE_TILING_LINEAR;
      *is_dma_buf = true;
   }
   return VK_SUCCESS;
}

static inline void
vkr_emul_fix_buffer_create_info(const struct vkr_physical_device *physical_dev,
                                VkBufferCreateInfo *info,
                                bool *is_dma_buf)
{
   *is_dma_buf = false;
   if (!physical_dev->emulate_dma_buf)
      return;
   VkExternalMemoryBufferCreateInfo *ext =
      vkr_find_struct(info->pNext, VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO);
   if (ext && (ext->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT)) {
      *is_dma_buf = true;
      vkr_remove_struct(info, VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO);
   }
}

/* Dma-buf memory is host-visible memory here. */
static inline void
vkr_emul_mask_memory_types(const struct vkr_physical_device *physical_dev,
                           uint32_t *memory_type_bits)
{
   if (*memory_type_bits & physical_dev->host_visible_memory_type_bits)
      *memory_type_bits &= physical_dev->host_visible_memory_type_bits;
}

#endif /* VKR_DMA_BUF_EMUL_H */
