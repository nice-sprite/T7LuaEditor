#include "win32_input.h"
#include <Tracy.hpp>
#include <imgui.h>
#include <windowsx.h>

#if defined DEBUG_GAME_INPUT || defined DEBUG_WIN32_INPUT
#include <imgui.h>
#endif

namespace Input {
b8 window_has_focus = false;

void focus_changed(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == WM_ACTIVATE) {
    window_has_focus =
        (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE);
  }
}

namespace Ui {
MouseState mouse_state{};
KeyboardState kbd_state{};

void debug_ui_input() {
  if (ImGui::Begin("double click")) {
    ImGui::Text("%f %f", mouse_state.x, mouse_state.y);
    ImGui::Text("left_double: %d", mouse_state.left_double_click);
    ImGui::Text("right_double: %d", mouse_state.right_double_click);
    ImGui::Text("middle_double: %d", mouse_state.middle_double_click);
    ImGui::Text("x1_double: %d", mouse_state.x1_double_click);
    ImGui::Text("x2_double: %d", mouse_state.x2_double_click);
  }
  ImGui::End();
}

void debug_win32_input() {
  /* INPUT DEBUGGING and TESTING */
#ifdef DEBUG_WIN32_INPUT
  Input::Ui::register_callback([](Input::MouseState const &mouse,
                                  Input::KeyboardState const &kbd) -> b8 {
    if (ImGui::Begin("Keyboard Debug")) {
      ImGui::Text("keyboard state debug");
      ImGui::Text("shift_down, %d", kbd.shift_down);
      ImGui::Text("tab_down, %d", kbd.tab_down);
      ImGui::Text("backspace_down, %d", kbd.backspace_down);
      ImGui::Text("enter_down, %d", kbd.enter_down);
      ImGui::Text("space_down, %d", kbd.space_down);
      ImGui::Text("ctrl_down, %d", kbd.ctrl_down);
      ImGui::Text("caps_down, %d", kbd.caps_down);
      size_t index = 0;
      for (Input::KeyState const &key : kbd.keys) {
        u8 value = key.character_value ? key.character_value : (u8)index;
        ImGui::Text("%c | h: %d | d: %d", value, (i32)key.held, (i32)key.down);
        index++;
      }
    }
    ImGui::End();
    if (false && ImGui::Begin("Mouse Debug")) {
      ImGui::Text("%f %f", mouse.x, mouse.y);
      ImGui::Separator();
      ImGui::Text("scroll: %d", mouse.scroll_delta);
      ImGui::Separator();

      ImGui::Text("left_button: %d", mouse.left_down);
      ImGui::Text("left_double: %d", mouse.left_double_click);
      ImGui::Separator();

      ImGui::Text("right_button: %d", mouse.right_down);
      ImGui::Text("right_double: %d", mouse.right_double_click);
      ImGui::Separator();

      ImGui::Text("middle_button: %d", mouse.middle_down);
      ImGui::Text("middle_double: %d", mouse.middle_double_click);
      ImGui::Separator();

      ImGui::Text("extra buttons");
      ImGui::Text("x1_down: %d", mouse.x1_down);
      ImGui::Text("x1_double: %d", mouse.x1_double_click);

      ImGui::Text("\nx2_down: %d", mouse.x2_down);
      ImGui::Text("x2_double: %d", mouse.x2_double_click);
    }
    ImGui::End();

    static f32 cursor_down_pos_x = 0.f;
    static f32 cursor_down_pos_y = 0.f;
    static b8 held = false;
    if (mouse.left_down && !held) {
      cursor_down_pos_x = mouse.x;
      cursor_down_pos_y = mouse.y;
      held = true;
    } else if (mouse.left_down && held) {
      rhi->imgui_draw_screen_rect(cursor_down_pos_x,
                                  mouse.x,
                                  cursor_down_pos_y,
                                  mouse.y);
    } else {
      held = false;
    }

    // rhi->imgui_draw_screen_rect(-50.f, 50.f, -50.f, 50.f);
    return true;
  });
#endif
}

void parse_mouse(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  ZoneScoped("");

  mouse_state.left_double_click = 0;
  mouse_state.right_double_click = 0;
  mouse_state.x1_double_click = 0;
  mouse_state.x2_double_click = 0;
  mouse_state.middle_double_click = 0;

  
  switch (msg) {

  case WM_MOUSEMOVE:
    mouse_state.x = (f32)GET_X_LPARAM(lparam);
    mouse_state.y = (f32)GET_Y_LPARAM(lparam);
    break;

    // handle double clicks
  case WM_RBUTTONDBLCLK:
    mouse_state.right_double_click = true;
    break;

  case WM_LBUTTONDBLCLK:
    mouse_state.left_double_click = true;
    break;

  case WM_MBUTTONDBLCLK:
    mouse_state.middle_double_click = true;
    break;

  case WM_XBUTTONDBLCLK:
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
      mouse_state.x1_double_click = true;
    }

    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
      mouse_state.x2_double_click = true;
    }
    break;
  }
}

void parse_keyboard(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  ZoneScoped("cache_keyboard_for_frame");
  if (msg == WM_CHAR) {
    switch (wparam) {
    case 0x08:

      // Process a backspace.

      break;

    case 0x0A:

      // Process a linefeed.

      break;

    case 0x1B:

      // Process an escape.

      break;

    case 0x09:

      // Process a tab.

      break;

    case 0x0D:

      // Process a carriage return.

      break;

    default:

      // Process displayable characters.

      break;
    }
  }

  if (msg == WM_KEYDOWN) {
    /* LPARAM
     * !bits 0-15:   repeat count
     * 16-23:        scan code (depends on OEM)
     * !24:          indicates extended key (right ctrl/alt)
     * 25-28:        reserved
     * 29:           context code, always 0 for WM_KEYDOWN
     * 30:           previous key state. 1 if down before msg was sent, 0 if the
     * key is up 31:           always 0 for wm_keydown
     */

    /* WPARAM
     * the VK code of non-system key
     */
    switch (wparam) {
    case VK_SHIFT:
      kbd_state.shift_down = true;
      break;
    case VK_BACK:
      kbd_state.backspace_down = true;
      break;
    case VK_TAB:
      kbd_state.tab_down = true;
      break;
    case VK_RETURN:
      kbd_state.enter_down = true;
      break;
    case VK_CONTROL:
      kbd_state.ctrl_down = true;
      break;
    case VK_CAPITAL:
      kbd_state.caps_down = true;
      break;
    default: {
      u32 ccode =
          MapVirtualKey(wparam,
                        MAPVK_VK_TO_CHAR); // translate the keycode to a char
      u16 repeat_count = (unsigned short)(lparam); // take lower 16 bits
      b8 is_extended_key =
          (lparam & (1 << 24)); // key is either right control or alt key
      kbd_state.keys[ccode].held = lparam & (1 << 30);
      kbd_state.keys[ccode].down = true;
      kbd_state.keys[ccode].character_value = ccode;
    }
    }
  }

  if (msg == WM_KEYUP) {
    /* LPARAM
     * !bits 0-15:   repeat count (always 1 for WM_KEYUP)
     * 16-23:        scan code (depends on OEM)
     * !24:          indicates extended key (right ctrl/alt)
     * 25-28:        reserved
     * 29:           context code, always 0 for WM_KEYUP
     * 30:           previous key state. Always 1 for WM_KEYUP
     * 31:           always 1 for wm_keyup
     */
    switch (wparam) {
    case VK_SHIFT:
      kbd_state.shift_down = false;
      break;
    case VK_BACK:
      kbd_state.backspace_down = false;
      break;
    case VK_TAB:
      kbd_state.tab_down = false;
      break;
    case VK_RETURN:
      kbd_state.enter_down = false;
      break;
    case VK_CONTROL:
      kbd_state.ctrl_down = false;
      break;
    case VK_CAPITAL:
      kbd_state.caps_down = false;
      break;
    default: {
      u32 ccode =
          MapVirtualKey(wparam,
                        MAPVK_VK_TO_CHAR); // translate the keycode to a char
      u16 repeat_count = (unsigned short)(lparam); // take lower 16 bits
      b8 is_extended_key =
          (lparam & (1 << 24)); // key is either right control or alt key
      kbd_state.keys[ccode].held = false; //  should be false
      kbd_state.keys[ccode].down = false;
      kbd_state.keys[ccode].character_value = ccode;
    }
    }
  }
}

b8 register_callback(InputCallback fn) {
  if (num_callbacks < MaxCallbacks) {
    callbacks[num_callbacks] = fn;
    num_callbacks += 1;
    return true;
  } else {
    // make some kind of error or log message
    return false;
  }
}

void process_input_for_frame() {
  ZoneScoped("process_input");
  //            for(int i = 0; i < num_callbacks; ++i) {
  //                callbacks[i](mouse_state, kbd_state);
  //            }
}

Cursor cursor() { return Cursor{mouse_state.x, mouse_state.y}; }

}; // namespace Ui

} // namespace Input
