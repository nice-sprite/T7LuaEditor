#include "WindowUtil.h"
#include "Renderer.h"


LRESULT Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    if (msg == WM_NCCREATE) {
        // extract ptr to window class from creation data
        const CREATESTRUCTW *const pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
        auto *const pWnd = static_cast<Window *>(pCreate->lpCreateParams);
        // set WinAPI-managed user data to store ptr to window instance
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // set message proc to normal (non-setup) handler now that setup is finished
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
        // forward message to window instance handler
        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    // retrieve ptr to window instance
    Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr( hWnd,GWLP_USERDATA ));
    // forward message to window instance handler
    return pWnd->HandleMsg( hWnd,msg,wParam,lParam );
}

LRESULT Window::HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: // TODO debounce this to avoid GB of memory being used when resizing over and over
        {
            this->GetGfx().Resize(lParam, wParam);
            break;
        }

        default:
            break;

    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

Window::Window(HINSTANCE hInst, std::wstring _title, std::wstring _classname, size_t _width, size_t _height)
        : hInst{hInst}, width{_width}, height{_height}, title{_title}, classname{_classname} {
    WNDCLASS wc = {};

    wc.lpfnWndProc = Window::HandleMsgSetup;
    wc.hInstance = hInst;
    wc.lpszClassName = _classname.c_str();

    RegisterClass(&wc);

    hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            _classname.c_str(),             // Window class
            _title.c_str(),                 // Window text
            WS_OVERLAPPEDWINDOW,            // Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

            nullptr,        // Parent window
            nullptr,        // Menu
            hInst,          // Instance handle
            this            // Additional application data
    );
    assert(hwnd != nullptr);
    graphics = std::make_unique<Renderer>(hwnd, _width, _height);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);


}

bool Window::ProcessMessages() {
    // main loop
    MSG msg;
    bool shouldClose = false;
    while (!shouldClose)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) {
                shouldClose = true;
                break;
            }
        }
        graphics->Render();
    }
    return false;
}
