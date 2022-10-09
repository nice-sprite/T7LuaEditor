#include "win32_input.h"
#include <windowsx.h>
#include <Tracy.hpp>

#if defined DEBUG_GAME_INPUT || defined DEBUG_WIN32_INPUT
    #include <imgui.h>
#endif

namespace Input
{
    bool window_has_focus = false;
    void focus_changed(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        if(msg == WM_ACTIVATE) {
            window_has_focus = (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE);
        }
    }
    namespace Ui {

        MouseState    mouse_state{};
        KeyboardState kbd_state{}; 
        void debug_ui_input() {
                if(ImGui::Begin("double click")) {
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
            Input::Ui::register_callback([](Input::MouseState const& mouse, Input::KeyboardState const& kbd) -> bool {
                if(ImGui::Begin("Keyboard Debug")) {
                    ImGui::Text("keyboard state debug");
                    ImGui::Text("shift_down, %d", kbd.shift_down);
                    ImGui::Text("tab_down, %d", kbd.tab_down);
                    ImGui::Text("backspace_down, %d", kbd.backspace_down);
                    ImGui::Text("enter_down, %d", kbd.enter_down);
                    ImGui::Text("space_down, %d", kbd.space_down);
                    ImGui::Text("ctrl_down, %d", kbd.ctrl_down);
                    ImGui::Text("caps_down, %d", kbd.caps_down);
                    size_t index = 0;
                    for(Input::KeyState const& key : kbd.keys) {
                        char value = key.character_value ? key.character_value : (char)index;
                        ImGui::Text("%c | h: %d | d: %d", value, (int)key.held, (int)key.down);
                        index++;
                    }
                }
                ImGui::End();
                if(false && ImGui::Begin("Mouse Debug")) {
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

                static float cursor_down_pos_x = 0.f; 
                static float cursor_down_pos_y = 0.f; 
                static bool held = false;
                if(mouse.left_down && !held) {
                    cursor_down_pos_x = mouse.x; 
                    cursor_down_pos_y = mouse.y; 
                    held = true;
                }else if(mouse.left_down && held) {
                    rhi->imgui_draw_screen_rect(cursor_down_pos_x, mouse.x, cursor_down_pos_y, mouse.y);
                } else {
                    held = false;
                }

                //rhi->imgui_draw_screen_rect(-50.f, 50.f, -50.f, 50.f);
                return true;
            });
#endif
        }

        // we get everything except double clicks from GameInput, and mouse doesnt really need anything besides the 
        // client coordinates of the mouse, literally why not just call GetCursorPos!
        void parse_mouse(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            ZoneScoped("");

            mouse_state.left_double_click = 0;
            mouse_state.right_double_click = 0;
            mouse_state.x1_double_click = 0;
            mouse_state.x2_double_click = 0;
            mouse_state.middle_double_click = 0;
            switch(msg) {

                case WM_MOUSEMOVE:
                    mouse_state.x = (float)GET_X_LPARAM(lparam);
                    mouse_state.y = (float)GET_Y_LPARAM(lparam);
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
                    if(GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
                        mouse_state.x1_double_click = true;
                    }

                    if(GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
                        mouse_state.x2_double_click = true;
                    }
                    break;
            }
        }

        void parse_keyboard(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            ZoneScoped("cache_keyboard_for_frame");
            if(msg == WM_CHAR)
            {
                switch (wparam)
                {
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

            if(msg == WM_KEYDOWN)
            {
                /* LPARAM
                 * !bits 0-15:   repeat count
                 * 16-23:        scan code (depends on OEM)
                 * !24:          indicates extended key (right ctrl/alt) 
                 * 25-28:        reserved
                 * 29:           context code, always 0 for WM_KEYDOWN
                 * 30:           previous key state. 1 if down before msg was sent, 0 if the key is up
                 * 31:           always 0 for wm_keydown
                 */

                /* WPARAM
                 * the VK code of non-system key
                 */
                switch(wparam) {
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
                        unsigned int ccode = MapVirtualKey(wparam, MAPVK_VK_TO_CHAR); // translate the keycode to a char
                        unsigned short repeat_count = (unsigned short)(lparam); // take lower 16 bits
                        bool is_extended_key = (lparam & (1 << 24)); // key is either right control or alt key
                        kbd_state.keys[ccode].held = lparam & (1<<30); 
                        kbd_state.keys[ccode].down = true;
                        kbd_state.keys[ccode].character_value = ccode;
                    }
                }
            }

            if(msg == WM_KEYUP)
            {
                /* LPARAM
                 * !bits 0-15:   repeat count (always 1 for WM_KEYUP)
                 * 16-23:        scan code (depends on OEM)
                 * !24:          indicates extended key (right ctrl/alt) 
                 * 25-28:        reserved
                 * 29:           context code, always 0 for WM_KEYUP
                 * 30:           previous key state. Always 1 for WM_KEYUP
                 * 31:           always 1 for wm_keyup
                 */
                switch(wparam) {
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
                         unsigned int ccode = MapVirtualKey(wparam, MAPVK_VK_TO_CHAR); // translate the keycode to a char
                         unsigned short repeat_count = (unsigned short)(lparam); // take lower 16 bits
                         bool is_extended_key = (lparam & (1 << 24)); // key is either right control or alt key
                         kbd_state.keys[ccode].held = false; //  should be false
                         kbd_state.keys[ccode].down = false;
                         kbd_state.keys[ccode].character_value = ccode;
                    }
                }
            }
        }


        bool register_callback(InputCallback fn) {
            if(num_callbacks < MaxCallbacks) {
                callbacks[num_callbacks] = fn;
                num_callbacks += 1;
                return true;
            } else {
                // make some kind of error or log message
                return false;
            }
        }

        void process_input_for_frame()
        {
            ZoneScoped("process_input");
//            for(int i = 0; i < num_callbacks; ++i) {
//                callbacks[i](mouse_state, kbd_state);
//            }
        }

        Cursor cursor() { return Cursor{mouse_state.x, mouse_state.y}; } 

    };

}

namespace Input {
    namespace GameInput {
        IGameInput* game_input = nullptr;
        GIMouseGlob mouse{};
        GIKeyboardGlob keyboard{};
        GIGamepadGlob gamepad{};

        HRESULT start() {
            HRESULT result = GameInputCreate(&game_input);
            game_input->SetFocusPolicy(GameInputDisableBackgroundInput);
            return result;
        }

        HRESULT shutdown() {
            return HRESULT{};
        }


        void poll_gamepad() {
            IGameInputReading* reading;
            if(SUCCEEDED(game_input->GetCurrentReading(GameInputKindGamepad, gamepad.device, &reading))) {
                if(gamepad.device == nullptr) {
                    reading->GetDevice(&gamepad.device);
                }
                reading->GetGamepadState(&gamepad.state);
                reading->Release();
            } else if(mouse.device != nullptr) {
                gamepad.device->Release();
                gamepad.device = nullptr;
            }
		}

        void poll_mouse() {
            IGameInputReading* reading;
            GameInputMouseState state;
            if(SUCCEEDED(game_input->GetCurrentReading(GameInputKindMouse, mouse.device, &reading))) {
                if(mouse.device == nullptr) {
                    reading->GetMouseState(&state);
                    if(state.buttons != 0 || state.positionX != 0 || state.positionY != 0 || state.wheelX != 0 || state.wheelY != 0) {
                        // use this device only if there is a non-zero field, prevents ghost device from 'stealing' input
                        reading->GetDevice(&mouse.device);
                        mouse.device_info = (GameInputDeviceInfo*)mouse.device->GetDeviceInfo();
                    }
                }
                mouse.prev_mouse_state = mouse.curr_mouse_state;
                reading->GetMouseState(&state);
                reading->Release();
                mouse.curr_mouse_state = state;
            } else if(mouse.device != nullptr) {
                mouse.device->Release();
                mouse.device = nullptr;
            }
        }

        void poll_keyboard() {
            IGameInputReading* reading;
            if(SUCCEEDED(game_input->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading))) {
                if(keyboard.device == nullptr) {
                    reading->GetDevice(&keyboard.device);
                    keyboard.device_info = (GameInputDeviceInfo*)keyboard.device->GetDeviceInfo();
                    if(keyboard.device_info->keyboardInfo->maxSimultaneousKeys > 0) {
                        keyboard.max_simultaneous_keys = keyboard.device_info->keyboardInfo->maxSimultaneousKeys;
                        keyboard.curr_keystate = new GameInputKeyState[keyboard.max_simultaneous_keys]{}; // this is currently leaking lol 
                        keyboard.prev_keystate = new GameInputKeyState[keyboard.max_simultaneous_keys]{}; // this is currently leaking lol 
                    } else {
                        keyboard.device = nullptr;
                    }
                }

                if(keyboard.device != nullptr) {
                    memcpy(keyboard.prev_keystate, keyboard.curr_keystate, keyboard.max_simultaneous_keys * sizeof(GameInputKeyState));
                    keyboard.prev_active = keyboard.curr_active;
                    keyboard.curr_active = reading->GetKeyCount();
                    reading->GetKeyState(keyboard.curr_active, keyboard.curr_keystate);
                }
                reading->Release();
            } else if(keyboard.device != nullptr) {
                keyboard.device->Release();
                keyboard.device = nullptr;
            }
        }

        void update() {
            poll_keyboard();
            poll_mouse();
            poll_gamepad();
		}

        bool key_down(uint8_t vk) {
            for(int i = 0; i < keyboard.curr_active; ++i)
                if(keyboard.curr_keystate[i].virtualKey == vk) return true;
            return false;
		}


        bool key_combo(uint8_t a, uint8_t b) {
            return key_down(a) && key_down(b);
		}

        bool key_oneshot(uint8_t vk) {
            bool in_last = false;
            for(int i = 0; i < keyboard.prev_active; ++i) {
                if(keyboard.prev_keystate[i].virtualKey == vk) { 
                    in_last = true; 
                    break;
                }
            }
            return key_down(vk) && !in_last;
		}

        GIMouseDelta mouse_delta() {
            GIMouseDelta d;
            d.dx = mouse.curr_mouse_state.positionX - mouse.prev_mouse_state.positionX;
            d.dy = mouse.curr_mouse_state.positionY - mouse.prev_mouse_state.positionY;
            d.wheel_x = mouse.curr_mouse_state.wheelX - mouse.prev_mouse_state.wheelX; 
            d.wheel_y = mouse.curr_mouse_state.wheelY - mouse.prev_mouse_state.wheelY; 
            return d;
		}

        bool mouse_button_down(GameInputMouseButtons button) {
            return (mouse.curr_mouse_state.buttons & button) > 0;
		}

        bool mouse_button_oneshot(GameInputMouseButtons button) {
            return false;
		}

        void draw_input_debug() {
            #ifdef DEBUG_GAME_INPUT 
                if(ImGui::Begin("GameInput Debug View")) {

                    float left_stick[2] = {gamepad.state.leftThumbstickX, gamepad.state.leftThumbstickY};
                    float right_stick[2] = {gamepad.state.rightThumbstickX, gamepad.state.rightThumbstickY};
                    ImGui::Text("MOUSE");
                    ImGui::Text("deltas: %d %d", mouse.curr_mouse_state.positionX, mouse.curr_mouse_state.positionY);
                    ImGui::Text("wheel: %d %d", mouse.curr_mouse_state.wheelX, mouse.curr_mouse_state.wheelY);
                    ImGui::Text(
                        "buttons: %d %d %d %d %d %d %d",
                        mouse.curr_mouse_state.buttons & GameInputMouseLeftButton, 
                        mouse.curr_mouse_state.buttons & GameInputMouseRightButton,
                        mouse.curr_mouse_state.buttons & GameInputMouseMiddleButton,
                        mouse.curr_mouse_state.buttons & GameInputMouseButton4,
                        mouse.curr_mouse_state.buttons & GameInputMouseButton5,
                        mouse.curr_mouse_state.buttons & GameInputMouseWheelTiltLeft,
                        mouse.curr_mouse_state.buttons & GameInputMouseWheelTiltRight
                    );
                    ImGui::Separator();

                    ImGui::Text("Gamepad");
                    ImGui::SliderFloat("LT", &gamepad.state.leftTrigger, 0.0f, 1.0f, "%.2f", 1.0f);
                    ImGui::SliderFloat("RT", &gamepad.state.rightTrigger, 0.0f, 1.0f, "%.2f", 1.0f);
                    ImGui::DragFloat2("Left Stick", left_stick);
                    ImGui::DragFloat2("Right Stick", right_stick);

                    ImGui::Text(
                       "None %d\n,  Menu %d\n,  View %d\n,  A %d\n,  B %d\n,  X %d\n,  Y %d\n,  DPadUp %d\n,  DPadDown %d\n,  DPadLeft %d\n,  DPadRight %d\n,  LeftShoulder %d\n,  RightShoulder %d\n,  LeftThumbstick %d\n,  RightThumbstick %d", 
                       gamepad.state.buttons & GameInputGamepadNone ,  
                       gamepad.state.buttons & GameInputGamepadMenu ,  
                       gamepad.state.buttons & GameInputGamepadView ,  
                       gamepad.state.buttons & GameInputGamepadA ,  
                       gamepad.state.buttons & GameInputGamepadB ,  
                       gamepad.state.buttons & GameInputGamepadX ,  
                       gamepad.state.buttons & GameInputGamepadY ,  
                       gamepad.state.buttons & GameInputGamepadDPadUp ,  
                       gamepad.state.buttons & GameInputGamepadDPadDown ,  
                       gamepad.state.buttons & GameInputGamepadDPadLeft ,  
                       gamepad.state.buttons & GameInputGamepadDPadRight ,  
                       gamepad.state.buttons & GameInputGamepadLeftShoulder ,  
                       gamepad.state.buttons & GameInputGamepadRightShoulder ,  
                       gamepad.state.buttons & GameInputGamepadLeftThumbstick ,  
                       gamepad.state.buttons & GameInputGamepadRightThumbstick
                    );

                    ImGui::Separator();
                    ImGui::Text("KEYBOARD");
                    ImGui::Text("max_keys: %llu", keyboard.max_simultaneous_keys);
                    ImGui::Text("active_keys: %llu", keyboard.curr_active);
                    if(key_down('W')) ImGui::Text("Forward");
                    if(key_down('A')) ImGui::Text("Left");
                    if(key_down('S')) ImGui::Text("Back");
                    if(key_down('D')) ImGui::Text("Right");
                    if(key_down(VK_LCONTROL)) ImGui::Text("CTRL");
                } 
                ImGui::End();
            #endif
        }

    };
};

