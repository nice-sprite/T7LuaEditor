#include "win32_input.h"
#include <windowsx.h>
#include <Tracy.hpp>
namespace input
{

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
                }

                if(GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
                    mouse_state.x2_down = false;
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

    /*
    void register_mouse_move_callback(MouseMoveFn fn)
    {
        if (countMouseMove < MaxCallbacks)
        {
            mouseMoveListeners.at(countMouseMove) = fn;
            ++countMouseMove;
        }
    }

    void register_mouse_click_callback(MouseClickFn fn)
    {
        if (countMouseClick < MaxCallbacks)
        {
            mouseClickListeners.at(countMouseClick) = fn;
            ++countMouseClicks;
        }
    }

    void register_keyboard_callback(KeyboardFn fn)
    {
        if (countKeyboard < MaxCallbacks)
        {
            keyboardListeners.at(countKeyboard) = fn;
            ++countKeyboard;
        }
    }

    */

    void process_input_for_frame()
    {
        ZoneScoped("process_input");
        for(int i = 0; i < num_callbacks; ++i) {
            callbacks[i](mouse_state, kbd_state);
        }
        mouse_state.scroll_delta = 0;
        /*if(true || cacheMouse > 0)
        {
            auto cursor = mouse[0];
            for (auto i = 0u; i < countMouseMove; ++i)
                mouseMoveListeners[i](cursor.x, cursor.y, cursor.stateFlags);

            for (auto i = 0u; i < countMouseClick; ++i)
                mouseClickListeners[i](cursor, 0.f, 1.f, cursor.stateFlags);


            cacheMouse = 0;
        }

        if(true || cacheKeyboard > 0)
        {
            auto keys = kbd[0];
            for (auto i = 0u; i < countKeyboard; ++i)
                keyboardListeners[i](keys);
            cacheKeyboard = 0;
        }
        */
    }

    bool Ctrl(WPARAM wparam)
    {
        return (wparam & MK_CONTROL) != 0;
    }

    bool Btn_Left(WPARAM wparam)
    {
        return (wparam & MK_LBUTTON) != 0;
    }

    bool Btn_Right(WPARAM wparam)
    {
        return (wparam & MK_RBUTTON) != 0;
    }

    bool Btn_Mid(WPARAM wparam)
    {
        return wparam & MK_MBUTTON;
    }

    bool Shift(WPARAM wparam)
    {
        return wparam & MK_SHIFT;
    }

    bool Btn_XBtn1(WPARAM wparam)
    {
        return wparam & MK_XBUTTON1;
    }

    bool Btn_XBtn2(WPARAM wparam)
    {
        return wparam & MK_XBUTTON2;
    }

}


