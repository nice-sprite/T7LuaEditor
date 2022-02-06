#ifndef INPUT_H
#define INPUT_H
#include <Windows.h>
#include <array>
#include <functional>

namespace Input
{
    // using MouseClickFn = bool (__stdcall*)(void* userData, Mouse::ButtonStates& buttons, float x, float y, int extra);
    // using KeyboardFn = bool(__stdcall *)(void *userData, Keyboard::KeyboardState keyState);
    // using MouseMoveFn = bool (__stdcall*)(void* userData, float x, float y, int extra);

    struct mouse_t
    {
        float x, y;
        WPARAM stateFlags;
    };

    struct keyboard_t
    {
        char key[256];
        int isExtended;
    };

    static constexpr auto MaxInputCache = 16u;
    static mouse_t mouse[MaxInputCache]{};
    static keyboard_t kbd[MaxInputCache]{};
    static int cacheMouse = 0;
    static int cacheKeyboard = 0;

    // extra holds the modifer keys like shift, ctrl, etc, what mouse buttons were down
    using MouseMoveFn = std::function<bool(float x, float y, WPARAM extra)>;
    using MouseClickFn = std::function<bool(mouse_t &mouse, float x, float y, WPARAM extra)>;
    using KeyboardFn = std::function<bool(keyboard_t &keyState)>;

    static constexpr auto MaxCallbacks = 50u;
    static std::array<MouseMoveFn, MaxCallbacks> mouseMoveListeners{};
    static std::array<MouseClickFn, MaxCallbacks> mouseClickListeners{};
    static std::array<KeyboardFn, MaxCallbacks> keyboardListeners{};
    static size_t countMouseMove = 0;
    static size_t countMouseClick = 0;
    static size_t countKeyboard = 0;

    void RegisterMouseMove(MouseMoveFn fn);
    void RegisterMouseClick(MouseClickFn fn);
    void RegisterKeyboardFn(KeyboardFn fn);
    void ProcessInput();

    void CacheMouseEvents(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    void CacheKeyboardEvents(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    bool Ctrl(WPARAM wparam);
    bool Btn_Left(WPARAM);
    bool Btn_Right(WPARAM);
    bool Btn_Mid(WPARAM);
    bool Shift(WPARAM);
    bool Btn_XBtn1(WPARAM);
    bool Btn_XBtn2(WPARAM);
}
#endif