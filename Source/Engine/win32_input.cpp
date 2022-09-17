#include "win32_input.h"
#include <windowsx.h>
#include <Tracy.hpp>

namespace Input
{
    namespace Ui {

        void cache_mouse_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            ZoneScoped("cache_mouse_for_frame");
            float x = (float)GET_X_LPARAM(lparam);
            float y = (float)GET_Y_LPARAM(lparam);
            int keys = GET_KEYSTATE_WPARAM(wparam);
            POINT screen_point{};

            mouse_state.ctrl_down = LOWORD(wparam) & MK_CONTROL;
            mouse_state.left_down = LOWORD(wparam) & MK_LBUTTON;
            mouse_state.right_down = LOWORD(wparam) & MK_RBUTTON;
            mouse_state.middle_down = LOWORD(wparam) & MK_MBUTTON;
            mouse_state.shift_down = LOWORD(wparam) & MK_SHIFT;
            mouse_state.x1_down = LOWORD(wparam) & MK_XBUTTON1;
            mouse_state.x2_down = LOWORD(wparam) & MK_XBUTTON2;
            switch(msg) {
                case WM_MOUSEWHEEL:
                    // WM_MOUSEWHEEL doesnt send client relative coords for some reason...
                    screen_point.x = int(x); 
                    screen_point.y = int(y);
                    ScreenToClient(hwnd, &screen_point);
                    mouse_state.x = (float)screen_point.x;
                    mouse_state.y = (float)screen_point.y;
                    mouse_state.scroll_delta = GET_WHEEL_DELTA_WPARAM(wparam);
                    break;

                case WM_MOUSEMOVE:
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_RBUTTONUP:
                    mouse_state.right_down = false;
                    mouse_state.right_double_click = false;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_RBUTTONDOWN:
                    mouse_state.right_down = true;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_RBUTTONDBLCLK:
                    mouse_state.right_down = true;
                    mouse_state.right_double_click = true;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_LBUTTONUP:
                    mouse_state.left_down = false;
                    mouse_state.left_double_click = false;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_LBUTTONDOWN:
                    mouse_state.left_down = true;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_LBUTTONDBLCLK:
                    mouse_state.left_down = true;
                    mouse_state.left_double_click = true;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_MBUTTONUP:
                    mouse_state.middle_down = false;
                    mouse_state.middle_double_click = false;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_MBUTTONDOWN:
                    mouse_state.middle_down = true;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_MBUTTONDBLCLK:
                    mouse_state.middle_down = true;
                    mouse_state.middle_double_click = true;
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_XBUTTONUP:
                    if(GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
                        mouse_state.x1_down = false;
                        mouse_state.x1_double_click = false;
                    }

                    if(GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
                        mouse_state.x2_down = false;
                        mouse_state.x2_double_click = false;
                    }
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_XBUTTONDOWN:
                    if(GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
                        mouse_state.x1_down = true;
                    }

                    if(GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
                        mouse_state.x2_down = true;
                    }
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;

                case WM_XBUTTONDBLCLK:
                    if(GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
                        mouse_state.x1_down = true;
                        mouse_state.x1_double_click = true;
                    }

                    if(GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
                        mouse_state.x2_down = true;
                        mouse_state.x2_double_click = true;
                    }
                    mouse_state.x = x;
                    mouse_state.y = y;
                    break;
            }
        }

        void cache_keyboard_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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
                                 unsigned int character_code = MapVirtualKey(wparam, MAPVK_VK_TO_CHAR); // translate the keycode to a char
                                 unsigned short repeat_count = (unsigned short)(lparam); // take lower 16 bits
                                 bool is_extended_key = (lparam & (1 << 24)); // key is either right control or alt key
                                 kbd_state.keys[character_code].held = lparam & (1<<30); 
                                 kbd_state.keys[character_code].down = true;
                                 kbd_state.keys[character_code].character_value = character_code;
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
                                 unsigned int character_code = MapVirtualKey(wparam, MAPVK_VK_TO_CHAR); // translate the keycode to a char
                                 unsigned short repeat_count = (unsigned short)(lparam); // take lower 16 bits
                                 bool is_extended_key = (lparam & (1 << 24)); // key is either right control or alt key
                                 kbd_state.keys[character_code].held = false; //  should be false
                                 kbd_state.keys[character_code].down = false;
                                 kbd_state.keys[character_code].character_value = character_code;
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
            for(int i = 0; i < num_callbacks; ++i) {
                callbacks[i](mouse_state, kbd_state);
            }
            mouse_state.scroll_delta = 0;
        }

        void handle_activate(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
            if(msg == WM_ACTIVATE) {
                window_has_focus = (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE);
            }
        }

    };
}

namespace Input {

    namespace GameInput {
        IGameInput* game_input = nullptr;
        IGameInputDevice* gamepad = nullptr;
        GIMouseGlob mouse{};
        GIKeyboardGlob keyboard{};

        HRESULT start() {
            HRESULT result = GameInputCreate(&game_input);
            game_input->SetFocusPolicy(GameInputDisableBackgroundInput);
            return result;
        }

        HRESULT shutdown() {
            return HRESULT{};
        }


        void poll_gamepad() {
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


    };
};

