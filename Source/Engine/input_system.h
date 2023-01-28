#pragma once
#include "../defines.h"
#include "math.h"
#include "timer.h"
#include "win32_lib.h"
#include <Windows.h>

#define INPUT_DEBUG
#ifdef INPUT_DEBUG
#include <imgui.h>
#endif

typedef i32 WheelDelta;

struct MouseDelta {
  i32 dx;
  i32 dy;
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

enum EventType {
  MouseMove,

  MouseLeftClick,
  MouseLeftDblClick,
  MouseLeftRelease,

  MouseRightClick,
  MouseRightDblClick,
  MouseRightRelease,

  MouseMiddleClick,
  MouseMiddleDblClick,
  MouseMiddleRelease,

  // for the x1 and x2 buttons
  MouseExtraClick1,
  MouseExtraDblClick1,
  MouseExtraRelease1,

  MouseExtraClick2,
  MouseExtraDblClick2,
  MouseExtraRelease2,

  DragStart,
  Dragging,
  DragEnd,

  MouseScroll,

  KeyDown,
  KeyUp,
  KeyHeld,
  KeyCombo,
  KeyChord

};

struct MouseEvent {
  ScreenPos mouse_pos;
  MouseDelta mouse_delta;
  WheelDelta wheel_delta;
  MouseButtons buttons;
  EventType type;
  // u64 timestamp;
};

enum InputFocusScope {
  IMGUI,
  WORLD_EDIT,
  CAMERA_CTRL,
};

// TODO input layers might be a natural way to let listeners declare when they
// are interested in certain messages. Maybe an array of layers like
// register_event_listener( {Layer_Global, Layer_EditorImgui, Layer_Scene}, ..r)
struct MouseEventListener {
  void *self;
  void *function;
  u8 priority;
};

struct InputSystem {
  InputSystem() = default;
  InputSystem(const InputSystem &) = delete;
  InputSystem &operator=(const InputSystem &) = delete; // disable copy

  MouseDelta mouse_delta{};
  WheelDelta mouse_wheel{};
  MouseButtons mouse_buttons{};
  ScreenPos mouse_pos{};
  Keyboard keyboard{};
  bool window_active = true;
  bool imgui_active = true;
  InputFocusScope focus_scope;

  Timer timer;
  static constexpr u32 MaxMouseListeners = 1024;
  MouseEventListener mouse_listeners[MaxMouseListeners];
  u32 mouse_listener_count = 0;

  static constexpr u32 MaxEventsPerFrame = 256;
  MouseEvent frame_events[MaxEventsPerFrame];
  u32 event_count;

  inline void queue_mouse_event(
      EventType etype,
      ScreenPos mouse_pos,
      MouseDelta mouse_delta,
      MouseButtons buttons,
      WheelDelta wheel_delta = 0); // only WM_MOUSEWHEEL updates this so its
                                   // easier to just default it to 0

  void init(HWND target_hwnd);
  void shutdown();
  static InputSystem &instance();
  u32 raw_input(HWND hwnd,
                u32 msg,
                u32 raw_input_code,
                HRAWINPUT raw_input_handle);

  // handles the WM_XXX messages from wndproc
  void handle_win32_input(HWND hwnd, u32 msg, WPARAM wparam, LPARAM lparam);

  void update();

  // at the end of a frame we need to clear transient values such as double
  // clicks
  void end_frame();

  // high polling rate mice saturate the WNDPROC
  // with a bunch of WM_INPUT - which blocks rendering and other messages
  // call this once per frame
  // returns number of messages processed
  u32 proc_buffered_input();

  // get state
  bool key_down(u8 key);

  bool add_mouse_listener(const MouseEventListener listener);
  u32 dispatch_listeners(const MouseEvent mouse_event);

  // bool add_keyboard_listener(const KeyboardEventListener listener);

private:
  void *ri_chunk;
  u32 msg_buff_count;
  u32 ri_chunk_bytewidth;

private:
  // drag state
  ScreenPos down_pos{};
  bool register_mouse(HWND target_hwnd);
  bool register_keyboard(HWND target_hwnd);
  bool register_devices(HWND target_hwnd);
  bool unregister_devices(HWND target_hwnd);
  void parse_raw_input_packet(RAWINPUT const &packet);
  bool is_msg_mouse(u32 msg);
};
