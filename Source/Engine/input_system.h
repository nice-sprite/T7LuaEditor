#pragma once
#include "../defines.h"
#include <Windows.h>
#include <vector>

struct MouseDelta {
  i32 dx;
  i32 dy;
};

struct ScreenPos {
  f32 x;
  f32 y;
};

struct KeyState {
  /* The ascii text value of this keycode.
   * differs from the index into the KeyboardState::key[]
   * can be capital or uppercase depending on shift key and caps key state
   */
  u8 character_value;
  b8 held; // whether the windows default "held" behavior is active or not
  b8 down;
};

struct Keyboard {
  KeyState keys[256];
  // modifier keys
  b8 shift_down, tab_down, backspace_down, enter_down, space_down, ctrl_down,
      caps_down;
};

struct MouseButtons {
  b8 left;
  b8 right;
  b8 middle;
  b8 x1;
  b8 x2;
  b8 right_dbl;
  b8 left_dbl;
  b8 middle_dbl;
  b8 x1_dbl;
  b8 x2_dbl;
};

struct InputSystem {
  MouseDelta mouse_delta{};
  MouseButtons mouse_buttons{};
  ScreenPos mouse_pos{};
  Keyboard keyboard{};

  void init(HWND target_hwnd);
  static InputSystem &instance();
  u32 raw_input(HWND hwnd,
                u32 msg,
                u32 raw_input_code,
                HRAWINPUT raw_input_handle);

  // handles the WM_XXX messages from wndproc
  void handle_win32_input(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam);

  // high polling rate mice saturate the WNDPROC
  // with a bunch of WM_INPUT - which blocks rendering and other messages
  // call this once per frame
  // returns number of messages processed
  u32 proc_buffered_input();

private:
  void enumerate_devices();
  bool register_mouse(HWND target_hwnd);
  bool register_keyboard(HWND target_hwnd);
  bool register_devices(HWND target_hwnd);
  void parse_raw_input_packet(RAWINPUT const &packet);
};
