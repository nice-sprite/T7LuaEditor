#pragma once
/* keyboard, mouse, and gamepad (eventually) input system for win32
 *
 * Keyboard:
 *  interested in
 *  text input
 *  key presses
 *  keys being held
 *  key combos and action mappings
 *
 * Mouse:
 *  mouse absolute position (relative to clinet window) for interaction with UI
 * and selections relative mouse coordinates (eg: +20, -50) for camera controls
 *  mouse button states *and* keyboard states that map to actions; ctrl +
 * scroll_forward -> zoom in, shift+lmb_down+mouse_move means pan camera. double
 * click has its own window msg, so I won't consider it combo-able and it will
 * have its own callback-type
 *
 * Gamepad:
 *  Slightly less interested for now
 *  Camera controls are easier for some people (Radiant users) with the gamepad
 * but not essential for this mainly would be useful for matching the games
 * key-bind system in UI menus such as L/R button for shuffling caresel widgets
 *  maybe full interaction testing can be implemented in the future for menus
 *
 * Ideally, input should be "usable" by the time it gets to a callback.
 *
 * Processing:
 *  Some actions are meant to be "one-shot", meaning they are ran once when
 * their condition is true. Example: user clicks on a text box. The text box
 * should not re-select every frame, and should not have to spam static
 * last_state = false; everywhere through teh code. Text box should be able to
 * do this in its update thing; if(input::mouse_query(LEFT_BUTTON, oneshot =
 * true)) { // run once } mouse_query should have all the states from the
 * previous frame, and if oneshot == true && last_mouse_state.buttons &
 * LEFT_BUTTON > 0 then it should return false.
 *
 * Keyboard query should work the same
 *
 */
#include "../defines.h"
#include <GameInput.h>
#include <Windows.h>
#include <array>
#include <functional>
#include <wrl/client.h>

using namespace Microsoft::WRL;
namespace Input {
enum CurrentFocus { ImGui, Scene, Element, None };
extern bool window_has_focus;

void focus_changed(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void decide_focus(b8 imgui_wants);

// Ui input for controls, typing text in boxes, mouse picking
namespace Ui {

struct Cursor {
  f32 x, y;
};

struct MouseState {
  f32 x, y;
  b8 left_double_click;
  b8 right_double_click;
  b8 middle_double_click;
  b8 x1_double_click;
  b8 x2_double_click;
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

struct KeyboardState {
  KeyState keys[256];
  // modifier keys
  b8 shift_down, tab_down, backspace_down, enter_down, space_down, ctrl_down,
      caps_down;
};

extern MouseState mouse_state;
extern KeyboardState kbd_state;

// i think no :)
using InputCallback =
    std::function<bool(MouseState const &, KeyboardState const &)>;

constexpr size_t MaxCallbacks = 512;
static std::array<InputCallback, MaxCallbacks> callbacks;
static u64 num_callbacks = 0;

bool register_callback(InputCallback fn);

void process_input_for_frame();

void parse_mouse(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void parse_keyboard(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// void end_of_frame();// zeros out transient input fields like dbl_clicks and
// scroll delta
void debug_win32_input();

void debug_ui_input();

Cursor cursor();
}; // namespace Ui

namespace GameInput {
// MOUSE
struct GIMouseDelta {
  i64 dx;
  i64 dy;
  i64 wheel_x;
  i64 wheel_y;
};

struct GIMouseGlob {
  IGameInputDevice *device = nullptr;
  GameInputDeviceInfo *device_info = nullptr;
  GameInputMouseState curr_mouse_state{};
  GameInputMouseState prev_mouse_state{};
};

// KEYBOARD
struct GIKeyboardGlob {
  IGameInputDevice *device = nullptr;
  GameInputDeviceInfo *device_info = nullptr;
  GameInputKeyState *curr_keystate = nullptr;
  GameInputKeyState *prev_keystate = nullptr;
  u32 curr_active;
  u32 prev_active;
  u32 max_simultaneous_keys;
};

struct GIGamepadGlob {
  IGameInputDevice *device = nullptr;
  GameInputDeviceInfo *device_info = nullptr;
  GameInputGamepadState state;
};

// GameInput
extern IGameInput *game_input;
extern GIGamepadGlob gamepad;
extern GIMouseGlob mouse;
extern GIKeyboardGlob keyboard;

HRESULT start();

HRESULT shutdown();

void poll_gamepad();

void poll_mouse();

void poll_keyboard();

void update();

bool key_down(u8 vk);

bool key_combo(u8 a, u8 b);

bool key_oneshot(u8 vk);

GIMouseDelta mouse_delta();

b8 mouse_button_down(GameInputMouseButtons button);

b8 mouse_button_oneshot(GameInputMouseButtons button);

void draw_input_debug();
}; // namespace GameInput

} // namespace Input
