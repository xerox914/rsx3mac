#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <atomic>
#include <util/types.hpp>

// ============================================================
//  PUBLIC API — matches fastlog.cpp
// ============================================================

// GLOBAL Master fastlog toggle and expiration time for burst logging
extern std::atomic<bool> g_fastlog_enabled;
extern u64 g_fastlog_end_time;

namespace fastlog {

  uint64_t fastlog_timestamp() noexcept;

  // --------------------------------------------------------
  //  Startup / bootstrap
  // --------------------------------------------------------
  void fastlog_bootstrap(); // Clear log + header + install signal handler
  void fastlog_init();      // Lazy file open
  void fastlog_shutdown();  // Close file

  // --------------------------------------------------------
  //  Signal-driven burst mode
  // --------------------------------------------------------
  void init_signal_handler(); // Install SIGUSR1 handler
  void heartbeat();           // Called once per frame/tick
  void next_frame() noexcept; // Advance the global RSX frame counter (called
							  // once per frame)

  // --------------------------------------------------------
  //  RSX FIFO logging entry point
  // --------------------------------------------------------
  void fifo(uint32_t addr, uint32_t cmd, uint32_t value) noexcept;

  // --------------------------------------------------------
  //  Low-level write used by FIFO logger
  // --------------------------------------------------------
  void fastlog_write(const char *msg, size_t len);

  // --------------------------------------------------------
  //  Log FIFO pointer movement (step, wrap, stall, empty)
  // --------------------------------------------------------
  void fifo_ptr_step(uint32_t oldp, uint32_t newp) noexcept;
  void fifo_ptr_wrap(uint32_t oldp, uint32_t newp) noexcept;
  void fifo_ptr_stall(uint32_t ptr) noexcept;
  void fifo_ptr_empty(uint32_t ptr) noexcept;

} // namespace fastlog

// ============================================================
//  Convenience printf-style wrapper
// ============================================================

inline void fastlog_printf(const char *fmt, ...) {
  char buffer[512];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  fastlog::fastlog_write(buffer, std::strlen(buffer));
}

inline void fastlog_msg(const char *msg) {
  fastlog::fastlog_write(msg, std::strlen(msg));
}

// ============================================================
//  Pointer hashing (still used by your RSX macros)
// ============================================================

static inline uint32_t fastlog_hash_ptr(const void *p) {
  uint64_t v = reinterpret_cast<uintptr_t>(p);
  uint32_t h = 2166136261u;

  for (int i = 0; i < 8; ++i) {
	uint8_t b = static_cast<uint8_t>(v & 0xFF);
	v >>= 8;
	h ^= b;
	h *= 16777619u;
  }

  return h;
}
