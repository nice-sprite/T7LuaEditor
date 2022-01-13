#include "WindowUtil.h"

namespace WindowUtil
{
    Window Create(HINSTANCE hinst, const wchar_t *windowTitle,
                  const wchar_t *classname,
                  int width, int height, WNDPROC proc)
    {
        Window window;
        WNDCLASS wc = {};
        wc.lpfnWndProc = proc;
        wc.hInstance = hinst;
        wc.lpszClassName = classname;
        RegisterClass(&wc);
        window.hwnd = CreateWindowEx( 0,
            classname,
            windowTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            nullptr,
            nullptr,
            hinst,
            nullptr);
        ShowWindow(window.hwnd, SW_SHOWDEFAULT);
        UpdateWindow(window.hwnd);
        GetWindowRect(window.hwnd, &window.clientRect);
        return window;
    }

    void SetIcon(HWND hwnd, const wchar_t *iconPath)
    {
        HINSTANCE hinst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        SetClassLongPtr(
            hwnd, 
            GCLP_HICON, 
            (LONG_PTR)LoadImage(hinst, iconPath, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE));
    }

};
