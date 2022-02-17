#include "win32_input.h"
#include <windowsx.h>

namespace input
{

    void cache_mouse_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        auto x = float(GET_X_LPARAM(lparam)),
             y = float(GET_Y_LPARAM(lparam));

        if (msg == WM_MOUSEWHEEL)
        { // for some reason mousewheel isn't client area relative
            POINT pt = {x, y};
            ScreenToClient(hwnd, &pt);
            x = pt.x;
            y = pt.y;
        }
        mouse[0] = {x, y, wparam};
        ++cacheMouse;
    }

    void cache_keyboard_input_for_frame(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
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
            if(lparam & (1 << 24))
                kbd[0].isExtended = true;                       // check if its an extended key

            auto charCode = MapVirtualKey(wparam, MAPVK_VK_TO_CHAR); // translate the keycode to a char
            kbd[0].key[charCode ] = charCode;
        }
        if(msg == WM_KEYUP)
        {
            if(lparam & (1 << 24))
                kbd[0].isExtended = true;                       // check if its an extended key

            auto charCode = MapVirtualKey(wparam, MAPVK_VK_TO_CHAR); // translate the keycode to a char
            kbd[0].key[charCode ] = 0;

        }
        ++cacheKeyboard;
    }

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

    void process_input_for_frame()
    {
        if(true || cacheMouse > 0)
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
