
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
 *  interested in mouse absolute position (relative to clinet window) for interaction with UI and selections
 *  relative mouse coordinates (eg: +20, -50) for camera controls
 *  mouse button states *and* keyboard states that map to actions; ctrl + scroll_forward -> zoom in, shift+lmb_down+mouse_move means pan camera.
 *  double click has its own window msg, so I won't consider it combo-able and it will have its own callback-type 
 *  
 * Gamepad: 
 *  Slightly less interested for now
 *  Camera controls are easier for some people (Radiant users) with the gamepad but not essential for this 
 *  mainly would be useful for matching the games key-bind system in UI menus such as L/R button for shuffling caresel widgets
 *  maybe full interaction testing can be implemented in the future for menus
 *
 * Ideally, input should be "usable" by the time it gets to a callback. 
 *
 *
 */
#ifndef INPUT_H
#define INPUT_H
#include <Windows.h>
#include <array>
#include <functional>
#include <GameInput.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;
namespace input
{
    // contains position
    struct MouseState
    {
        float x, y;
        bool ctrl_down;
        bool left_down;
        bool right_down;
        bool middle_down;
        bool shift_down;
        bool x1_down;
        bool x2_down;
        bool left_double_click;
        bool right_double_click;
        bool middle_double_click;
        bool x1_double_click;
        bool x2_double_click;
        int scroll_delta;
    };


    struct KeyState {
        /* The ascii text value of this keycode. 
         * differs from the index into the KeyboardState::key[] 
         * can be capital or uppercase depending on shift key and caps key state
         */
        char character_value;  
        bool held; // whether the windows default "held" behavior is active or not
        bool down; 
    };

    struct KeyboardState
    {
        KeyState keys[256];
        // modifier keys
        bool shift_down,
             tab_down,
             backspace_down,
             enter_down,
             space_down,
             ctrl_down,
             caps_down;

    };

    static MouseState mouse_state;
    static KeyboardState kbd_state; 

    using InputCallback = std::function<bool(MouseState const& , KeyboardState const& )>;
    
    constexpr size_t MaxCallbacks = 512; 
    static std::array<InputCallback, MaxCallbacks> callbacks;
    static size_t num_callbacks = 0;

    bool register_callback(InputCallback fn);
    void process_input_for_frame();
    void cache_mouse_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    void cache_keyboard_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    // GameInput stuff
    static ComPtr<IGameInput> game_input;
    static ComPtr<IGameInputDevice> kbd = nullptr;
    static GameInputDeviceInfo* mouse_device_info = nullptr;
    HRESULT start_game_input();
    HRESULT shutdown_game_input();

    static ComPtr<IGameInputDevice> gamepad = nullptr;
    GameInputGamepadState poll_gamepad();

    // MOUSE
    static ComPtr<IGameInputDevice> mouse = nullptr;
    GameInputMouseState poll_mouse();

    // KEYBOARD
    static GameInputDeviceInfo* keyboard_device_info = nullptr;
    extern GameInputKeyState* keystate;
    extern unsigned int max_keys;
    extern unsigned int active_keys;
    void poll_keys();
    bool key_down(uint8_t vk);
    bool key_combo(uint8_t a, uint8_t b);


}


#endif
