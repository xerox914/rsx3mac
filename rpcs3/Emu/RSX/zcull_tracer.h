#pragma once
#include "util/logs.hpp"
#include "Emu/RSX/Common/time.hpp"
#include "util/types.hpp" // for u32, u64

// --------------------------------------------------------------------------
//  Helper Macros (ALWAYS ENABLED)
// --------------------------------------------------------------------------
#define ZCULL_TRACE_THROTTLE 128
#define ZCULL_NOTICE(...) zcull_log.always()(__VA_ARGS__)
#define ZCULL_WARNING(...) zcull_log.warning(__VA_ARGS__)
#define ZCULL_ERROR(...) zcull_log.error(__VA_ARGS__)
#define ZCULL_TRACE_NO_PREFIX(...) zcull_log.always()(__VA_ARGS__)

#define ZCULL_TRACE_THROTTLED(counter_var, ...)          \
	do                                                   \
	{                                                    \
		if (++(counter_var) % ZCULL_TRACE_THROTTLE == 0) \
		{                                                \
			zcull_log.always()(__VA_ARGS__);             \
		}                                                \
	} while (0)

#define ZCULL_TRACE(...) \
	zcull_log.always()(__VA_ARGS__)

namespace vk
{
	extern u64 g_zcull_frame_index;
}

namespace zcull_tracer
{
	inline u32 g_last_clear_arg = 0;
}

namespace zcull_tracer
{
	// ----------------------------------------------------------------------
	//  Core State (header-only, ODR-safe via static inline)
	// ----------------------------------------------------------------------
	static inline void* volatile g_tracked_main_depth_id = nullptr;

	static inline u32 g_tracked_main_depth_addr = 0;
	static inline u32 g_depth_instance_id = 0;
	static inline u32 g_current_depth_instance_id = 0;

	static inline u32 g_last_frame_count = 0;
	static inline u32 g_current_pass_index = 0;

	static inline u32 g_last_vblank_hz = 0;
	static inline u64 g_last_vblank_target_ns = 0;

	static inline u32 g_depth_invalidates = 0;
	static inline u32 g_depth_layout_changes = 0;
	static inline u32 g_occlusion_zero_results = 0;
	static inline u32 g_barrier_count = 0;

	static inline bool g_depth_was_invalidated_this_frame = false;
	static inline bool g_toxic_ordering_triggered = false;

	static inline u32 g_phase_trace_counter = 0;
	static inline u32 g_baseline_trace_counter = 0;

	// ----------------------------------------------------------------------
	//  Episode State
	// ----------------------------------------------------------------------
	inline bool g_episode_active = false;
	inline u32 g_episode_start_frame = 0;

	// ----------------------------------------------------------------------
	//  Episode Severity Grading
	// ----------------------------------------------------------------------
	inline char ComputeEpisodeSeverity(u32 peak, u32 avg)
	{
		// Catastrophic: sustained max hazard
		if (peak >= 7 && avg >= 5)
			return 'X';

		// Severe: multiple high-hazard frames
		if (peak >= 6 && avg >= 4)
			return '!';

		// Moderate: visible but not constant
		if (peak >= 4 && avg >= 2)
			return '^';

		// Mild: short or low-intensity
		if (peak >= 2)
			return '=';

		// Trivial blip
		return '~';
	}

	// ----------------------------------------------------------------------
	//  Hazard scoring (optional helper)
	// ----------------------------------------------------------------------
	inline u32 ComputeHazardLevel()
	{
		u32 score = 0;

		score += g_depth_invalidates * 2;
		score += g_depth_layout_changes;
		score += g_occlusion_zero_results * 3;

		if (g_toxic_ordering_triggered)
			score += 4;

		if (g_depth_was_invalidated_this_frame && g_occlusion_zero_results > 0)
			score += 5;

		return std::min(score, 7u);
	}

	// ----------------------------------------------------------------------
	//  Pipe bar generator (ASCII, fixed max width)
	// ----------------------------------------------------------------------
	inline void BuildPipeBar(char* out_buf, u32 buf_size, u32 value, u32 max_value)
	{
		if (buf_size == 0)
			return;

		if (max_value == 0)
		{
			out_buf[0] = '\0';
			return;
		}

		if (value > max_value)
			value = max_value;

		const u32 max_width = buf_size - 1; // leave room for '\0'
		u32 bar_len = (value * max_width) / max_value;

		for (u32 i = 0; i < bar_len; i++)
			out_buf[i] = '|';

		out_buf[bar_len] = '\0';
	}
	// ----------------------------------------------------------------------
	//  Episode block → for clean analysis of hazard bursts
	// ----------------------------------------------------------------------
	inline void PipeEpisodeBlock(u32 level)
	{
		// Print one pipe per hazard level
		for (u32 i = 0; i < level; i++)
			ZCULL_TRACE_NO_PREFIX("|");

		// Space between frames
		ZCULL_TRACE_NO_PREFIX(" ");
	}

	// ----------------------------------------------------------------------
	//  Episode Accumulators
	// ----------------------------------------------------------------------
	inline u32 g_episode_invalidates = 0;
	inline u32 g_episode_layout_changes = 0;
	inline u32 g_episode_zero_queries = 0;
	inline u32 g_episode_peak_level = 0;
	inline u32 g_episode_hazard_sum = 0;
	inline u32 g_episode_frame_count = 0;

	// ----------------------------------------------------------------------
	//  Episode Histogram Bins (0–7 hazard levels)
	// ----------------------------------------------------------------------
	inline u32 g_episode_histogram[8] = {0};

	// ----------------------------------------------------------------------
	//  API
	// ----------------------------------------------------------------------
	inline void banner()
	{
		ZCULL_NOTICE("[ZCULL] telemetry_armed=1");
	}

	inline void ResetSummary()
	{
		g_depth_invalidates = 0;
		g_depth_layout_changes = 0;
		g_occlusion_zero_results = 0;
		g_barrier_count = 0;

		g_depth_was_invalidated_this_frame = false;
		g_toxic_ordering_triggered = false;
	}

	inline void PrintSummary(u32 frame)
	{
		// Baseline throttled trace
		ZCULL_TRACE_THROTTLED(
			g_baseline_trace_counter,
			"[BASELINE] frame=%u pass=%u depth_ptr=%p depth_addr=0x%X depth_id=%u",
			frame,
			g_current_pass_index,
			g_tracked_main_depth_id,
			g_tracked_main_depth_addr,
			g_current_depth_instance_id);

		const bool clear_hazard =
			g_depth_was_invalidated_this_frame &&
			g_occlusion_zero_results > 0;

		const bool layout_hazard =
			g_depth_layout_changes > 0 &&
			g_toxic_ordering_triggered;

		const bool hazard = clear_hazard || layout_hazard;

		//// ----------------------------------------------------------------------
		//// EPISODE LOGIC BEGIN
		//// ----------------------------------------------------------------------

		if (hazard && !g_episode_active)
		{
			g_episode_active = true;
			g_episode_start_frame = frame;

			// Reset episode accumulators
			g_episode_invalidates = 0;
			g_episode_layout_changes = 0;
			g_episode_zero_queries = 0;
			g_episode_peak_level = 0;
			g_episode_hazard_sum = 0;
			g_episode_frame_count = 0;

			// Reset histogram bins
			for (u32 i = 0; i < 8; i++)
				g_episode_histogram[i] = 0;

			ZCULL_ERROR("[EPISODE_BEGIN] frame=%u", frame);
		}

		//// ----------------------------------------------------------------------
		//// EPISODE STATE MACHINE (begin/end detection only)
		//// ----------------------------------------------------------------------

		if (!hazard && g_episode_active)
		{
			g_episode_active = false;

			ZCULL_ERROR(
				"[EPISODE_END] start=%u end=%u length=%u",
				g_episode_start_frame,
				frame - 1,
				(frame - g_episode_start_frame));

			// Episode summary
			const u32 avg_level =
				(g_episode_frame_count > 0) ? (g_episode_hazard_sum / g_episode_frame_count) : 0;

			ZCULL_ERROR(
				"[EPISODE_SUMMARY] frames=%u invalidates=%u layout=%u zero=%u peak=%u avg=%u",
				g_episode_frame_count,
				g_episode_invalidates,
				g_episode_layout_changes,
				g_episode_zero_queries,
				g_episode_peak_level,
				avg_level);

			// Episode severity grading
			const char grade = ComputeEpisodeSeverity(g_episode_peak_level, avg_level);

			ZCULL_ERROR(
				"[EPISODE_SEVERITY] [%c] peak=%u avg=%u",
				grade,
				g_episode_peak_level,
				avg_level);

			// ------------------------------------------------------------------
			// Episode Histogram Output (ASCII bars + counts + percentages)
			// ------------------------------------------------------------------
			ZCULL_ERROR("[EPISODE_HISTOGRAM]");

			for (int lvl = 7; lvl >= 0; lvl--)
			{
				const u32 count = g_episode_histogram[lvl];
				const u32 total = g_episode_frame_count;

				// Compute percentage (integer)
				const u32 pct = (total > 0) ? (count * 100 / total) : 0;

				// Build pipe bar for episode graph
				char bar[9];
				const u32 bar_len = (count > 8) ? 8 : count;
				for (u32 i = 0; i < bar_len; i++)
					bar[i] = '|';
				bar[bar_len] = '\0';

				ZCULL_ERROR(
					" %u %-8s |%u|   %3u%%",
					lvl,
					bar,
					count,
					pct);
			}
		}

		//// ----------------------------------------------------------------------
		//// EPISODE OUTPUT (heatmap strip)
		//// ----------------------------------------------------------------------

		if (g_episode_active)
		{
			const u32 level = ComputeHazardLevel();
			PipeEpisodeBlock(level);

			// Episode accumulation
			g_episode_invalidates += g_depth_invalidates;
			g_episode_layout_changes += g_depth_layout_changes;
			g_episode_zero_queries += g_occlusion_zero_results;
			g_episode_hazard_sum += level;
			g_episode_frame_count++;

			if (level > g_episode_peak_level)
				g_episode_peak_level = level;

			// Histogram accumulation
			g_episode_histogram[level]++;
		}

		//// ----------------------------------------------------------------------
		//// PER-FRAME DIAGNOSTICS
		//// ----------------------------------------------------------------------

		if (hazard)
		{
			ZCULL_ERROR(
				"[FLICKER_EPISODE] frame=%u "
				"depth_ptr=%p depth_addr=0x%X depth_id=%u "
				"invalidates=%u layout_changes=%u zero_queries=%u "
				"diagnosis=\"%s\"",
				frame,
				g_tracked_main_depth_id,
				g_tracked_main_depth_addr,
				g_current_depth_instance_id,
				g_depth_invalidates,
				g_depth_layout_changes,
				g_occlusion_zero_results,
				clear_hazard ? "async_clear_desync" : "toxic_layout_ordering");
		}

		// Summary for non-hazard frames
		if (!hazard)
		{
			char inv_bar[33];
			char layout_bar[33];
			char zero_bar[33];

			const u32 max_inv = 16;
			const u32 max_layout = 16;
			const u32 max_zero = 16;

			BuildPipeBar(inv_bar, sizeof(inv_bar), g_depth_invalidates, max_inv);
			BuildPipeBar(layout_bar, sizeof(layout_bar), g_depth_layout_changes, max_layout);
			BuildPipeBar(zero_bar, sizeof(zero_bar), g_occlusion_zero_results, max_zero);

			ZCULL_TRACE(
				"[SUMMARY] frame=%u "
				"invalidates=%u %s "
				"layout_changes=%u %s "
				"zero_queries=%u %s "
				"diagnosis=\"ok\"",
				frame,
				g_depth_invalidates, inv_bar,
				g_depth_layout_changes, layout_bar,
				g_occlusion_zero_results, zero_bar);
		}
	}

	inline void UpdateFrameReset(u32 current_frame)
	{
		if (current_frame != g_last_frame_count)
		{
			if (g_last_frame_count != 0)
				PrintSummary(g_last_frame_count);

			g_current_pass_index = 0;
			ResetSummary();
		}

		if (current_frame < g_last_frame_count ||
			(current_frame - g_last_frame_count) > 5)
		{
			g_tracked_main_depth_id = nullptr;
			g_tracked_main_depth_addr = 0;
			g_current_depth_instance_id = 0;
		}

		g_last_frame_count = current_frame;
	}

	inline void RecordVblankChange(u32 new_hz)
	{
		if (new_hz != g_last_vblank_hz)
		{
			ZCULL_NOTICE("[VBLANK] old=%u new=%u", g_last_vblank_hz, new_hz);
			g_last_vblank_hz = new_hz;
		}
	}

	inline void RecordFlipDeadline(u64 deadline_ns, u64 now_ns)
	{
		g_last_vblank_target_ns = deadline_ns;

		ZCULL_TRACE(
			"[VBLANK] deadline=%llu now=%llu delta=%lld",
			static_cast<unsigned long long>(deadline_ns),
			static_cast<unsigned long long>(now_ns),
			static_cast<long long>(deadline_ns - now_ns));
	}

	inline float GetCurrentVblankPhase(u64 now_ns)
	{
		if (g_last_vblank_hz == 0 || g_last_vblank_target_ns == 0)
			return 0.0f;

		const u64 frame_ns = 1'000'000'000ull / g_last_vblank_hz;

		if (now_ns > g_last_vblank_target_ns)
			return 1.0f;

		const u64 start_ns = g_last_vblank_target_ns - frame_ns;
		const u64 into = now_ns - start_ns;

		return float(into) / float(frame_ns);
	}

	inline void RecordLayoutChange()
	{
		g_depth_layout_changes++;
	}

	inline void RecordInvalidate()
	{
		g_depth_invalidates++;
		g_depth_was_invalidated_this_frame = true;
	}

	inline void RecordZeroQuery()
	{
		g_occlusion_zero_results++;
		if (g_depth_was_invalidated_this_frame)
			g_toxic_ordering_triggered = true;
	}

	inline bool IsPrimaryDepth(void* img_id)
	{
		return (g_tracked_main_depth_id == nullptr) ||
		       (img_id == g_tracked_main_depth_id);
	}
} // namespace zcull_tracer
