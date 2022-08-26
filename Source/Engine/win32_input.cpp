#include "win32_input.h"
#include <windowsx.h>
#include <Tracy.hpp>
namespace input
{

    void cache_mouse_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        ZoneScoped("cache_mouse_for_frame");
        auto x = float(GET_X_LPARAM(lparam)),
             y = float(GET_Y_LPARAM(lparam));

        if (msg == WM_MOUSEWHEEL)
        { // for some reason mousewheel isn't client area relative
            POINT pt = {x, y};
            ScreenToClient(hwnd, &pt);
            x = pt.x;
            y = pt.y;
        }
       // mouse[0] = {x, y, wparam};
       // ++cacheMouse;
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
            unsigned int character_code = MapVirtualKey(wparam, MAPVK_VK_TO_CHAR); // translate the keycode to a char
            unsigned short repeat_count = (unsigned short)(lparam); // take lower 16 bits
            bool is_extended_key = (lparam & (1 << 24)); // key is either right control or alt key
            kbd_state.key[character_code].character_value = character_code;

            // TODO test if this is correct way to handle windows default 
            // key-held behavior (should have a delay before repeating)
            kbd_state.key[character_code].held = repeat_count > 1; 
            kbd_state.key[character_code].down = true;
        //    return 0;
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
            unsigned int character_code = MapVirtualKey(wparam, MAPVK_VK_TO_CHAR); // translate the keycode to a char
            unsigned short repeat_count = (unsigned short)(lparam); // take lower 16 bits
            bool is_extended_key = (lparam & (1 << 24)); // key is either right control or alt key

            // TODO test if this is correct way to handle windows default 
            // key-held behavior (should have a delay before repeating)
            kbd_state.key[character_code].held = false;
            kbd_state.key[character_code].down = false;
            kbd_state.key[character_code].character_value = character_code;
          //  return 0;
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
            ++countMouseClick;
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


