#include "stdafx.h"
#include "image_helpers.h"
#include "image.h"
#include "../VKRenderPass.h"
#include "../../color_utils.h"
#include "Emu/RSX/rsx_methods.h"
#include "Emu/RSX/zcull_tracer.h"

namespace vk
{
	VkComponentMapping default_component_map =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A};

	VkImageAspectFlags get_aspect_flags(VkFormat format)
	{
		switch (format)
		{
		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}

	VkComponentMapping apply_swizzle_remap(const std::array<VkComponentSwizzle, 4>& base_remap, const rsx::texture_channel_remap_t& remap_vector)
	{
		const auto final_mapping = remap_vector.remap(base_remap, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ONE);
		return {final_mapping[1], final_mapping[2], final_mapping[3], final_mapping[0]};
	}

	void change_image_layout(const vk::command_buffer& cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout, const VkImageSubresourceRange& range,
		u32 src_queue_family, u32 dst_queue_family, u32 src_access_mask_bits, u32 dst_access_mask_bits)
	{
		// SANITIZE LAYOUTS FIRST — ALWAYS
		// Convert layout to integer safely
		uint32_t old_u32 = static_cast<uint32_t>(current_layout);
		uint32_t new_u32 = static_cast<uint32_t>(new_layout);

		// Vulkan valid layouts are 0 → VK_IMAGE_LAYOUT_PREINITIALIZED
		constexpr uint32_t MAX_VALID_LAYOUT = VK_IMAGE_LAYOUT_PREINITIALIZED;

		// If layout is outside valid Vulkan range, sanitize it
		if (old_u32 > MAX_VALID_LAYOUT) {
			current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			old_u32 = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		if (new_u32 > MAX_VALID_LAYOUT) {
			new_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			new_u32 = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		// END SANITIZE LAYOUTS

		// Fallback for NULL color images (hardware-accurate)
		static VkImage s_last_valid_color_image_global = VK_NULL_HANDLE;
		
		// Color Fallback
		if (range.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
		{
			if (image != VK_NULL_HANDLE)
			{
				// Remember last valid color image
				s_last_valid_color_image_global = image;
			}
			else if (s_last_valid_color_image_global != VK_NULL_HANDLE)
			{
				// Intercept NULL color RT: reuse last valid
				image = s_last_valid_color_image_global;
			}
		}
		
		// >>> BEGIN FILTERED COLOR LOGGING <<<

		// Only color transitions
		if (!(range.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT))
			goto continue_color;

		// Only real layout changes
		if (old_u32 == new_u32)
			goto continue_color;

		// Only when renderpass is open (optional but recommended)
		if (!vk::is_renderpass_open(cmd))
			goto continue_color;

		// Only log the main 1280x720 buffers (avoid spam from tiny UI buffers)
		if (!((range.baseMipLevel == 0) && (range.levelCount == 1)))
			goto continue_color;

		// Now log (safe, fmt-style)
		ZCULL_TRACE("[ZCULL_VK] color_layout_change image={} old={} new={} aspect=0x{:X} mip={} layers={}",
			reinterpret_cast<void*>(image),
			old_u32,
			new_u32,
			static_cast<uint32_t>(range.aspectMask),
			range.baseMipLevel,
			range.layerCount);

		continue_color:
		// >>> END FILTERED COLOR LOGGING <<<
		
		// If we *still* have no image, treat as a no-op (RSX-style forgiveness)
		if (image == VK_NULL_HANDLE)
			return;

		if (range.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
		{
//			//ZCULL_TRACE("[ZCULL_VK] layout_transition_COLOR "
//						"image={} old={} new={} aspect=0x{:X} mip={} layers={}",
//				reinterpret_cast<void*>(image),
//				old_u32,
//				new_u32,
//				static_cast<uint32_t>(range.aspectMask),
//				range.baseMipLevel,
//				range.layerCount);

		}

		if (range.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		{
			const auto& regs = rsx::method_registers;
			const u32 depth_addr = regs.surface_z_offset();
			const bool rp_open = vk::is_renderpass_open(cmd);
			
			if (depth_addr == 0)
			{
//				//ZCULL_TRACE("[ZCULL_VK] layout_transition SKIP rsx_addr=0 (UI/null depth) "
//							"image={} old={} new={} aspect=0x{:X} mip={} layers={} rp_open={}",
//					reinterpret_cast<void*>(image),
//					old_u32,
//					new_u32,
//					static_cast<uint32_t>(range.aspectMask),
//					range.baseMipLevel,
//					range.layerCount,
//					static_cast<uint32_t>(rp_open));

				return;
			}

//			//ZCULL_TRACE("[ZCULL_VK] layout_transition "
//						"image={} old={} new={} aspect=0x{:X} mip={} layers={} depth_addr=0x{:X} rp_open={}",
//				reinterpret_cast<void*>(image),
//				old_u32,
//				new_u32,
//				static_cast<uint32_t>(range.aspectMask),
//				range.baseMipLevel,
//				range.layerCount,
//				depth_addr,
//				static_cast<uint32_t>(rp_open));
		}

		if (range.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		{
			const bool rp_open = vk::is_renderpass_open(cmd);

//			//ZCULL_TRACE("[ZCULL_VK] layout_transition_rp "
//						"image={} old={} new={} aspect=0x{:X} rp_open={}",
//				reinterpret_cast<void*>(image),
//				old_u32,
//				new_u32,
//				static_cast<uint32_t>(range.aspectMask),
//				static_cast<uint32_t>(rp_open));
		}

		if (vk::is_renderpass_open(cmd))
		{
			vk::end_renderpass(cmd);
		}

		// Prepare an image to match the new layout..
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.newLayout = new_layout;
		barrier.oldLayout = current_layout;
		barrier.image = image;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;
		barrier.srcQueueFamilyIndex = src_queue_family;
		barrier.dstQueueFamilyIndex = dst_queue_family;
		barrier.subresourceRange = range;

		VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		const bool is_color_surface = !!(range.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT);

		switch (+new_layout)
		{
		case VK_IMAGE_LAYOUT_GENERAL:
			// Avoid this layout as it is unoptimized
			barrier.dstAccessMask =
				{
					VK_ACCESS_TRANSFER_READ_BIT |
					VK_ACCESS_TRANSFER_WRITE_BIT |
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
					VK_ACCESS_SHADER_READ_BIT |
					VK_ACCESS_INPUT_ATTACHMENT_READ_BIT};
			dst_stage =
				{
					VK_PIPELINE_STAGE_TRANSFER_BIT |
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT};
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			if (is_color_surface)
			{
				barrier.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dst_stage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else
			{
				barrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dst_stage |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			break;
		case VK_IMAGE_LAYOUT_UNDEFINED:
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Temporarily: don’t kill the process, just ignore this transition
				// (or treat it as GENERAL if you prefer)
				// Optional: add a lightweight printf here if you want to see it.
				return;
		default:
				// Same idea: don’t abort, just log and bail
				// std::printf("Invalid layout transition old=%u new=%u\n", old_u32, new_u32);
				return;
		}

		switch (+current_layout)
		{
		case VK_IMAGE_LAYOUT_GENERAL:
			// Avoid this layout as it is unoptimized
			if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
				new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				if (range.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT)
				{
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				}
				else
				{
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				}
			}
			else if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
					 new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				// Finish reading before writing
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
				src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else
			{
				barrier.srcAccessMask =
					{
						VK_ACCESS_TRANSFER_READ_BIT |
						VK_ACCESS_TRANSFER_WRITE_BIT |
						VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
						VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
						VK_ACCESS_SHADER_READ_BIT |
						VK_ACCESS_INPUT_ATTACHMENT_READ_BIT};
				src_stage =
					{
						VK_PIPELINE_STAGE_TRANSFER_BIT |
						VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
						VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
						VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT};
			}
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			src_stage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			if (is_color_surface)
			{
				barrier.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				src_stage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else
			{
				barrier.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				src_stage |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			break;
		default:
			break;
		}

		barrier.srcAccessMask &= src_access_mask_bits;
		barrier.dstAccessMask &= dst_access_mask_bits;

		if (!barrier.srcAccessMask)
			src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		if (!barrier.dstAccessMask)
			dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		vkCmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	void change_image_layout(const vk::command_buffer& cmd, vk::image* image, VkImageLayout new_layout, const VkImageSubresourceRange& range)
	{
		if (image->current_layout == new_layout)
			return;

		change_image_layout(cmd, image->value, image->current_layout, new_layout, range);
		image->current_layout = new_layout;
	}

	void change_image_layout(const vk::command_buffer& cmd, vk::image* image, VkImageLayout new_layout)
	{
		if (image->current_layout == new_layout)
			return;

		change_image_layout(cmd, image->value, image->current_layout, new_layout, {image->aspect(), 0, image->mipmaps(), 0, image->layers()});
		image->current_layout = new_layout;
	}
} // namespace vk
