#pragma once
#include "../defines.h"
#include <comdef.h> // for _com_error
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <windows.h>
#define LOG_ENABLE 1
#define ASSERTS_ENABLE 1

enum class LogLevel : u32 { Info, Warning, Com, Fatal };

template <> struct fmt::formatter<LogLevel> {
  constexpr auto parse(fmt::format_parse_context &ctx)
      -> decltype(ctx.begin()) {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const LogLevel &loglevel, FormatContext &ctx) const
      -> decltype(ctx.out()) {
    const char *level_str[] = {"INFO   ", "WARNING", "COM    ", "FATAL  "};
    return fmt::format_to(ctx.out(), "{}", level_str[(u32)loglevel]);
  }
};

static inline void impl_log_msg(LogLevel level, fmt::string_view fmt,
                                fmt::format_args args) {
  const char *level_str[] = {"INFO   ", "WARNING", "COM    ", "FATAL  "};
  const fmt::text_style level_style[] = {
      fmt::fg(fmt::color::sky_blue), fmt::fg(fmt::color::yellow), fmt::fg(fmt::color::red),
      fmt::emphasis::bold | fmt::fg(fmt::color::red)};

  fmt::print("[{}] {}\r\n",
             fmt::format(level_style[(u32)level], level_str[(u32)level]),
             fmt::vformat(level_style[(u32)level], fmt, args));
}

template <typename S, typename... Args>
void log_msg(LogLevel level, const S &format, Args &&...args) {
  impl_log_msg(level, format, fmt::make_format_args(args...));
}

static inline void log_assert(const char *expression, const char *msg,
                              const char *file, i32 line) {
  fmt::print("{}:{}: {}\r\n", fmt::format(fmt::fg(fmt::color::orange_red), file),
             fmt::format(fmt::fg(fmt::color::light_sky_blue), "{}", line),
             fmt::format(fmt::fg(fmt::color::orange), expression));
}

static inline void log_hresult(LogLevel level, HRESULT e) {
  _com_error ce(e);
  std::string error = ce.ErrorMessage();
  fmt::print("[{}] {}\r\n", level, error);
}

#if LOG_ENABLE == 1
#define LOG_INFO(fmt, ...) log_msg(LogLevel::Info, FMT_STRING(fmt), __VA_ARGS__)
#define LOG_WARNING(fmt, ...)                                                  \
  log_msg(LogLevel::Warning, FMT_STRING(fmt), __VA_ARGS__)
#define LOG_FATAL(fmt, ...)                                                    \
  log_msg(LogLevel::Fatal, FMT_STRING(fmt), __VA_ARGS__)
#define LOG_COM(error_code) log_hresult(LogLevel::Com, error_code)
#endif

#if LOG_ENABLE == 0
#define LOG_INFO(fmt, ...)
#define LOG_WARNING(fmt, ...)
#define LOG_FATAL(fmt, ...)
#define LOG_COM(error_code)
#endif

#if ASSERTS_ENABLE == 1
#define Q_ASSERT(expr)                                                         \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      log_assert(#expr, "", __FILE__, __LINE__);                               \
      __debugbreak();                                                          \
    }                                                                          \
  }

#define Q_ASSERTMSG(expr, msg)                                                 \
  {                                                                            \
    if (expr) {                                                                \
    } else {                                                                   \
      log_assert(#expr, msg, __FILE__, __LINE__);                              \
      __debugbreak();                                                          \
    }                                                                          \
  }
#endif

#if ASSERTS_ENABLE == 0
#define Q_ASSERT(expr)
#define Q_ASSERTMSG(expr, msg)
#endif

void logging_start();
