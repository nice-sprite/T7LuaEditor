#include "WindowUtil.h"

HWND WindowUtil::g_mainHwnd = NULL;
HINSTANCE WindowUtil::g_hInst = NULL;
std::wstring WindowUtil::g_windowName = L"";
std::wstring WindowUtil::g_windowClassName = L"";


HWND WindowUtil::NewWindow( std::wstring windowName, HINSTANCE hInst, bool nCmdShow )
{
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowUtil::WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = windowName.c_str();
    g_hInst = hInst;

    RegisterClass( &wc );

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        windowName.c_str(),                     // Window class
        windowName.c_str(),    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInst,  // Instance handle
        NULL        // Additional application data
    );
    assert( hwnd != NULL );
    g_mainHwnd = hwnd;

    ShowWindow( hwnd, nCmdShow );
    UpdateWindow( hwnd );

    return hwnd;
}

LRESULT WINAPI WindowUtil::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // TODO: handle resize event
    if (ImGui_ImplWin32_WndProcHandler( hwnd, uMsg, wParam, lParam ))
        return true;

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint( hwnd, &ps );

            FillRect( hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1) );

            EndPaint( hwnd, &ps );
        }
        return 0;

    }
    return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

void WindowUtil::Cleanup()
{
    DestroyWindow( g_mainHwnd );
    UnregisterClass( g_windowClassName.c_str(), g_hInst );
}
