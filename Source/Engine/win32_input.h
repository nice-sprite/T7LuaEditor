
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

namespace input
{

    // contains position
    struct MouseState
    {
        float x, y;
        bool left_button_down,
             right_button_down,
             left_dbl_click,
             middle_down;
        int scroll_delta;
        WPARAM stateFlags;
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
        KeyState key[256];
        bool shift_down,
             tab_down,
             ctrl_down,
             caps_down;

    };

    /*static constexpr auto MaxInputCache = 16u;
    static MouseState mouse[MaxInputCache]{};
    static keyboard_t kbd[MaxInputCache]{};
    */
    static MouseState mouse_state;
    static KeyboardState kbd_state; 

    // extra holds the modifer keys like shift, ctrl, etc, what mouse buttons were down
    //    using MouseMoveFn = std::function<bool(float x, float y, WPARAM extra)>;
    //    using MouseClickFn = std::function<bool(mouse_t &mouse, float x, float y, WPARAM extra)>;
    //    using KeyboardFn = std::function<bool(keyboard_t &keyState)>;

    static constexpr auto MaxCallbacks = 64u;
    /*static std::array<MouseMoveFn, MaxCallbacks> mouseMoveListeners{};
    static std::array<MouseClickFn, MaxCallbacks> mouseClickListeners{};
    static std::array<KeyboardFn, MaxCallbacks> keyboardListeners{};
    */
    static size_t countMouseMove = 0;
    static size_t countMouseClick = 0;
    static size_t countKeyboard = 0;

    /*
    void register_mouse_move_callback(MouseMoveFn fn);
    void register_mouse_click_callback(MouseClickFn fn);
    void register_keyboard_callback(KeyboardFn fn);
    */
    void process_input_for_frame();

    void cache_mouse_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    void cache_keyboard_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

#endif
