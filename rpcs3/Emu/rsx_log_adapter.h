#pragma once

#include <cstdint>
#include "Emu/RSX/rsx_methods.h"

namespace rsx
{
	using u32 = std::uint32_t;

	// Hash a surface offset into a stable 32‑bit ID for semantic logging.
	u32 surf_hash(u32 offset);

	// High‑level semantic classification for fallback logging.
	inline const char* classify_method(u32 reg)
	{
		switch (reg)
		{
			//
			// DRAW — geometry + draw calls
			//
			case NV4097_DRAW_ARRAYS:
			case NV4097_DRAW_INDEX_ARRAY:
			case NV4097_INLINE_ARRAY:
				return "DRAW";

			//
			// SURF — textures, render targets, surfaces
			//
			case NV4097_SET_SURFACE_FORMAT:
			case NV4097_SET_SURFACE_PITCH_A:
			case NV4097_SET_SURFACE_PITCH_B:
			case NV4097_SET_SURFACE_COLOR_TARGET:
			case NV4097_SET_SURFACE_ZETA_OFFSET:
			case NV4097_SET_TEXTURE_OFFSET:
			case NV4097_SET_TEXTURE_FORMAT:
			case NV4097_SET_TEXTURE_CONTROL0:
			case NV4097_SET_TEXTURE_CONTROL1:
			case NV4097_SET_TEXTURE_CONTROL3:
			case NV4097_SET_TEXTURE_FILTER:
			case NV4097_SET_TEXTURE_IMAGE_RECT:
			case NV4097_SET_TEXTURE_BORDER_COLOR:
			case NV4097_SET_VERTEX_TEXTURE_OFFSET:
			case NV4097_SET_VERTEX_TEXTURE_FORMAT:
			case NV4097_SET_VERTEX_TEXTURE_CONTROL0:
			case NV4097_SET_VERTEX_TEXTURE_CONTROL3:
			case NV4097_SET_VERTEX_TEXTURE_FILTER:
			case NV4097_SET_VERTEX_TEXTURE_IMAGE_RECT:
			case NV4097_SET_VERTEX_TEXTURE_BORDER_COLOR:
				return "SURF";

			//
			// SYNC — fences, semaphores, flushes, waits
			//
			case NV4097_WAIT_FOR_IDLE:
			case NV4097_INVALIDATE_ZCULL:
			case NV4097_INVALIDATE_VERTEX_CACHE_FILE:
			case NV4097_INVALIDATE_VERTEX_FILE:
			case NV4097_INVALIDATE_L2:
			case NV406E_SEMAPHORE_ACQUIRE:
			case NV406E_SEMAPHORE_RELEASE:
			case NV4097_BACK_END_WRITE_SEMAPHORE_RELEASE:
			case NV4097_TEXTURE_READ_SEMAPHORE_RELEASE:
				return "SYNC";

			//
			// STATE — everything else
			//
			default:
				return "STATE";
		}
	}
}

