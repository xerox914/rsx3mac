#include "fastlog.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <fcntl.h>
#include <mach/mach_time.h>
#include <mutex>
#include <pthread.h>
#include <string>
#include <string_view>
#include <unistd.h>

// ============================================================
//  Global State
// ============================================================

// GLOBAL Master fastlog toggle and expiration time for burst logging
std::atomic<bool> g_fastlog_enabled{false};
u64 g_fastlog_end_time = 0;

namespace fastlog {

  // =========================
  //  INTERNAL STATE
  // =========================
  static std::atomic<bool> g_burst_requested{false};
  static std::atomic<uint64_t> g_burst_end_ns{0};
  static FILE *g_fastlog = nullptr;
  static bool g_fastlog_disabled = false;
  static std::mutex g_fastlog_mutex;
  static std::string g_fastlog_last_line;
  static bool g_fastlog_has_last = false;
  static size_t g_fastlog_repeat_count = 0;
  static std::atomic<uint64_t> g_frame_index{0};

  // =========================
  //  HELPERS
  // =========================
  static inline uint64_t fastlog_thread_id() {
	return reinterpret_cast<uintptr_t>(pthread_self());
  }

  // ========================================================
  //  Time helper
  // ========================================================
  static inline uint64_t hardware_ticks() { return mach_absolute_time(); }

  static inline uint64_t nanosecond_burst_time() noexcept {
	using namespace std::chrono;
	return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch())
		.count();
  }

  uint64_t fastlog_timestamp() noexcept { return hardware_ticks(); }

  // ========================================================
  //  Low-level write (your fast logger)
  // ========================================================
  static inline void write_fifo(uint32_t addr, uint32_t cmd,
								uint32_t value) noexcept {
	char buf[128];
	int len = std::snprintf(buf, sizeof(buf),
							"RSX_FIFO addr=0x%08x cmd=0x%08x val=0x%08x", addr,
							cmd, value);

	if (len > 0)
	  fastlog_write(buf, static_cast<size_t>(len));
  }

  // ========================================================
  //  Public FIFO logging entry point
  // ========================================================
  void fifo(uint32_t addr, uint32_t cmd, uint32_t value) noexcept {
	if (!g_fastlog_enabled.load(std::memory_order_relaxed))
	  return;

	char buf[160];
	int len =
		std::snprintf(buf, sizeof(buf),
					  "[frame %llu] RSX_FIFO addr=0x%08x cmd=0x%08x val=0x%08x",
					  static_cast<unsigned long long>(
						  g_frame_index.load(std::memory_order_relaxed)),
					  addr, cmd, value);

	if (len > 0)
	  fastlog_write(buf, static_cast<size_t>(len));
  }

  // ========================================================
  //  Signal handler (async-signal-safe)
  // ========================================================
static void sigusr1_handler(int signo)
{
	if (signo != SIGUSR1)
		return;

	// Toggle global logging
	bool old = g_fastlog_enabled.load(std::memory_order_relaxed);
	bool now = !old;
	g_fastlog_enabled.store(now, std::memory_order_relaxed);

	if (now) {
		// Arm burst mode
		g_burst_requested.store(true, std::memory_order_relaxed);
	} else {
		// Hard off
		g_burst_requested.store(false, std::memory_order_relaxed);
		g_burst_end_ns.store(0, std::memory_order_relaxed);
	}
}

  // ========================================================
  //  Install signal handler
  // ========================================================
  void init_signal_handler() {
	struct sigaction sa{};
	sa.sa_handler = sigusr1_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGUSR1, &sa, nullptr);
  }

  // ========================================================
  //  Burst mode update + auto-off
  // ========================================================
  void heartbeat() {
	const uint64_t now = nanosecond_burst_time();

	// If a burst was requested, set the end time
	if (g_burst_requested.load(std::memory_order_relaxed)) {
	  g_burst_requested.store(false, std::memory_order_relaxed);
	  constexpr uint64_t burst_ns =
		  6ull * 1000ull * 1000ull * 1000ull; // 6 seconds
	  g_burst_end_ns.store(now + burst_ns, std::memory_order_relaxed);
	}

	// Auto-disable when burst expires
	if (g_fastlog_enabled.load(std::memory_order_relaxed)) {
	  uint64_t end = g_burst_end_ns.load(std::memory_order_relaxed);
	  if (end != 0 && now >= end)
		g_fastlog_enabled.store(false, std::memory_order_relaxed);
	}
  }

  void next_frame() noexcept {
	g_frame_index.fetch_add(1, std::memory_order_relaxed);
  }

  // =========================
  //  INITIALIZATION
  // =========================

  void fastlog_init() {
	if (g_fastlog)
	  return;

	g_fastlog = fopen("/Volumes/RSX3MAC/debug.log", "a");

	if (!g_fastlog)
	  g_fastlog_disabled = true;
  }

  void fastlog_shutdown() {
	if (g_fastlog) {
	  fclose(g_fastlog);
	  g_fastlog = nullptr;
	}
  }

  void fastlog_bootstrap() {
	g_fastlog_enabled = false;
	FILE *f = fopen("/Volumes/RSX3MAC/debug.log", "w");
	if (!f)
	  return;

	std::fprintf(f, "◤◢◤◢◤◢ [ RSX3MAX DEBUG LOG ] ◤◢◤◢◤◢");
	std::fclose(f);
	fastlog::init_signal_handler();
  }

  // =========================
  //  LOW-LEVEL WRITE
  // =========================

  void fastlog_write(const char *msg, size_t len) {
	if (!g_fastlog_enabled.load(std::memory_order_relaxed))
	  return;

	if (!g_fastlog)
	  fastlog_init();

	if (!g_fastlog)
	  return;

	std::string_view current(msg, len);

	std::lock_guard<std::mutex> lock(g_fastlog_mutex);

	// Repeat suppression
	if (g_fastlog_has_last && current == g_fastlog_last_line) {
	  g_fastlog_repeat_count++;
	  return;
	}

	// Flush repeats
	if (g_fastlog_repeat_count > 0) {
	  fprintf(g_fastlog, "⏏ %zu   <%llu> ≡ 0x%llx", g_fastlog_repeat_count,
			  hardware_ticks(), fastlog_thread_id());
	  fflush(g_fastlog);
	  g_fastlog_repeat_count = 0;
	}

	// Write new line
	fprintf(g_fastlog, "<%llu> ≡ 0x%llx %.*s", hardware_ticks(),
			fastlog_thread_id(), static_cast<int>(len), msg);
	fflush(g_fastlog);

	g_fastlog_last_line = current;
	g_fastlog_has_last = true;
  }

  // ========================================================
  //  FIFO Pointer tracking
  // ========================================================
  void fifo_ptr_step(uint32_t oldp, uint32_t newp) noexcept {
	if (!g_fastlog_enabled.load(std::memory_order_relaxed))
	  return;

	char buf[128];
	int len = std::snprintf(
		buf, sizeof(buf), "[frame %llu] FIFO.step 0x%08x → 0x%08x",
		static_cast<unsigned long long>(g_frame_index.load()), oldp, newp);

	if (len > 0)
	  fastlog_write(buf, len);
  }

  void fifo_ptr_wrap(uint32_t oldp, uint32_t newp) noexcept {
	if (!g_fastlog_enabled.load(std::memory_order_relaxed))
	  return;

	char buf[128];
	int len = std::snprintf(
		buf, sizeof(buf), "[frame %llu] FIFO.wrap 0x%08x ↺ 0x%08x",
		static_cast<unsigned long long>(g_frame_index.load()), oldp, newp);

	if (len > 0)
	  fastlog_write(buf, len);
  }

  void fifo_ptr_stall(uint32_t ptr) noexcept {
	if (!g_fastlog_enabled.load(std::memory_order_relaxed))
	  return;

	char buf[128];
	int len = std::snprintf(
		buf, sizeof(buf), "[frame %llu] FIFO.stall at 0x%08x",
		static_cast<unsigned long long>(g_frame_index.load()), ptr);

	if (len > 0)
	  fastlog_write(buf, len);
  }

  void fifo_ptr_empty(uint32_t ptr) noexcept {
	if (!g_fastlog_enabled.load(std::memory_order_relaxed))
	  return;

	char buf[128];
	int len = std::snprintf(
		buf, sizeof(buf), "[frame %llu] FIFO.empty at 0x%08x",
		static_cast<unsigned long long>(g_frame_index.load()), ptr);

	if (len > 0)
	  fastlog_write(buf, len);
  }

} // namespace fastlog
