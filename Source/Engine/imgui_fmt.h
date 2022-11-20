#include <DirectXMath.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <imgui.h>

using namespace DirectX;
template <> struct fmt::formatter<XMFLOAT4> {
  constexpr auto parse(fmt::format_parse_context &ctx)
      -> decltype(ctx.begin()) {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const XMFLOAT4 &vec, FormatContext &ctx) const
      -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(),
                          "({:2f}, {:2f}, {:2f}, {:2f})",
                          vec.x,
                          vec.y,
                          vec.z,
                          vec.w);
  }
};

template <> struct fmt::formatter<XMVECTOR> {
  constexpr auto parse(fmt::format_parse_context &ctx)
      -> decltype(ctx.begin()) {
    return ctx.end();
  }

  template <typename FormatContext>
  auto format(const XMVECTOR &vec, FormatContext &ctx) const
      -> decltype(ctx.out()) {
    XMFLOAT4 v;
    XMStoreFloat4(&v, vec);
    return fmt::format_to(ctx.out(),
                          "({:2f}, {:2f}, {:2f}, {:2f})",
                          v.x,
                          v.y,
                          v.z,
                          v.w);
  }
};

namespace ImGui {
void inline impl_imgui_fmt_print(fmt::string_view fmt, fmt::format_args args) {
  ImGui::Text("%s", fmt::vformat(fmt, args).c_str());
}

template <typename S, typename... Args>
void TextFmt(const S &format, Args &&...args) {
  impl_imgui_fmt_print(format, fmt::make_format_args(args...));
}
} // namespace ImGui