
#include "input_system.h"
#include "logging.h"
#include <errhandlingapi.h>
#include <windowsx.h>
#include <winuser.h>
#define QWORD u64
void InputSystem::init(HWND target_hwnd) {
  // register_mouse(target_hwnd);
  //  register_keyboard(target_hwnd);
  register_devices(target_hwnd);
}

InputSystem &InputSystem::instance() {
  static InputSystem is;
  return is;
}

void InputSystem::enumerate_devices() {
  u32 device_count = 0;
  std::vector<RAWINPUTDEVICELIST> devices{};
  GetRawInputDeviceList(nullptr, &device_count, sizeof(RAWINPUTDEVICELIST));
  devices.resize(device_count);
  device_count = GetRawInputDeviceList(devices.data(),
                                       &device_count,
                                       sizeof(RAWINPUTDEVICELIST));
  LOG_INFO("found {} devices\n", device_count);

  for (RAWINPUTDEVICELIST rd : devices) {
    if (rd.dwType == RIM_TYPEKEYBOARD) {
      // get device name
      std::string device_name(256u, '\0');
      u32 name_length = 256;
      u32 bytes_copied = GetRawInputDeviceInfoA(rd.hDevice,
                                                RIDI_DEVICENAME,
                                                (LPVOID)device_name.c_str(),
                                                &name_length);
      Q_ASSERT(bytes_copied != -1);
      LOG_INFO("keyboard: {}\n", device_name);
    }

    if (rd.dwType == RIM_TYPEMOUSE) {
    }
  }
}

bool InputSystem::register_devices(HWND target_hwnd) {

  RAWINPUTDEVICE devices[2]{};
  devices[0].hwndTarget = target_hwnd;
  devices[0].usUsagePage = 1;
  devices[0].usUsage = 2;
  devices[0].dwFlags = 0;

  devices[1].hwndTarget = target_hwnd;
  devices[1].usUsagePage = 1;
  devices[1].usUsage = 6;
  devices[1].dwFlags = 0;
  bool result = RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE));
  Q_ASSERT(result);
  LOG_INFO("registered raw devices input");
  return result;
}

bool InputSystem::register_mouse(HWND target_hwnd) {
  RAWINPUTDEVICE mouse_device{};
  mouse_device.hwndTarget = target_hwnd;
  mouse_device.usUsagePage = 1;
  mouse_device.usUsage = 2;
  mouse_device.dwFlags = 0;

  bool result =
      RegisterRawInputDevices(&mouse_device, 1, sizeof(RAWINPUTDEVICE));
  Q_ASSERT(result);
  LOG_INFO("registered mouse raw input");
  return result;
}

bool InputSystem::register_keyboard(HWND target_hwnd) {
  RAWINPUTDEVICE kbd_device{};
  kbd_device.hwndTarget = target_hwnd;
  kbd_device.usUsagePage = 1;
  kbd_device.usUsage = 6;
  kbd_device.dwFlags = 0;
  bool result = RegisterRawInputDevices(&kbd_device, 1, sizeof(RAWINPUTDEVICE));

  Q_ASSERT(result);
  LOG_INFO("registered kbd raw input");
  return result;
}

u32 InputSystem::raw_input(HWND hwnd,
                           u32 msg,
                           u32 raw_input_code,
                           HRAWINPUT raw_input_handle) {

  RAWINPUT raw_data{};
  u32 written_bytes{};

  GetRawInputData(raw_input_handle,
                  RID_INPUT,
                  nullptr,
                  &written_bytes,
                  sizeof(RAWINPUTHEADER));

  u32 result = GetRawInputData(raw_input_handle,
                               RID_INPUT,
                               &raw_data,
                               &written_bytes,
                               sizeof(RAWINPUTHEADER));

  if (result != written_bytes) {
    LOG_COM(GetLastError());
    LOG_FATAL("");
  }

  parse_raw_input_packet(raw_data);
  return (u32)DefRawInputProc(0, 0, sizeof(RAWINPUTHEADER));
}

u32 InputSystem::proc_buffered_input() {
  mouse_delta.dx = 0;
  mouse_delta.dy = 0;
  u32 buffer_byte_width = 0;
  u32 total_messages = 0;
  u32 num_messages = 100; //

  // does std::vector align allocations on "pointer boundary"?
  std::vector<RAWINPUT> ri_buffer{};

  GetRawInputBuffer(nullptr, &buffer_byte_width, sizeof(RAWINPUTHEADER));

  u32 total_block_size = buffer_byte_width * num_messages;
  ri_buffer.resize(total_block_size);

  for (;;) {

    u32 ri_count = GetRawInputBuffer(ri_buffer.data(),
                                     &total_block_size,
                                     sizeof(RAWINPUTHEADER));
    if (ri_count == 0) {
      return total_messages;
    }
    total_messages += ri_count;

    // LOG_INFO("got {} rawinputs", ri_count);

    // block_ptrs stores the beginning of each rawinput packet
    // std::vector<RAWINPUT *> block_ptrs(num_messages);
    RAWINPUT *curr_block = &ri_buffer[0];
    for (u32 i = 0; i < ri_count; ++i) {
      parse_raw_input_packet(*curr_block);
      curr_block = NEXTRAWINPUTBLOCK(curr_block);
    }
  }
}

void InputSystem::parse_raw_input_packet(RAWINPUT const &packet) {
  if (packet.header.dwType == RIM_TYPEMOUSE) {
    // LOG_INFO("mouse packet: dx {}, dy {}",
    //          curr_block->data.mouse.lLastX,
    //          curr_block->data.mouse.lLastY);
    mouse_delta.dx += packet.data.mouse.lLastX;
    mouse_delta.dy += packet.data.mouse.lLastY;
  }
  if (packet.header.dwType == RIM_TYPEKEYBOARD) {
    RAWKEYBOARD kbd = packet.data.keyboard;
    // LOG_INFO("{}", (char)kbd.VKey);
  }
}

// handles the WM_XXX messages from wndproc
void InputSystem::handle_win32_input(HWND hwnd,
                                     u32 msg,
                                     WPARAM wparam,
                                     LPARAM lparam) {

  switch (msg) {

  case WM_MOUSEMOVE:
    mouse_pos.x = (f32)GET_X_LPARAM(lparam);
    mouse_pos.y = (f32)GET_Y_LPARAM(lparam);
    break;

  case WM_LBUTTONDOWN:
    mouse_buttons.left = true;
    break;

  case WM_LBUTTONUP:
    mouse_buttons.left = false;
    break;

  case WM_RBUTTONDOWN:
    mouse_buttons.right = true;
    break;

  case WM_RBUTTONUP:
    mouse_buttons.right = false;
    break;

  case WM_MBUTTONDOWN:
    mouse_buttons.middle = true;
    break;

  case WM_MBUTTONUP:
    mouse_buttons.middle = false;
    break;

  case WM_XBUTTONDOWN:
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
      mouse_buttons.x1 = true;
    }
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
      mouse_buttons.x2 = true;
    }
    break;

  case WM_XBUTTONUP:
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
      mouse_buttons.x1 = false;
    }
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
      mouse_buttons.x2 = false;
    }
    break;

    // handle double clicks
  case WM_RBUTTONDBLCLK:
    mouse_buttons.right_dbl = true;
    break;

  case WM_LBUTTONDBLCLK:
    mouse_buttons.left_dbl = true;
    break;

  case WM_MBUTTONDBLCLK:
    mouse_buttons.middle_dbl = true;
    break;

  case WM_XBUTTONDBLCLK:
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
      mouse_buttons.x1_dbl = true;
    }
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
      mouse_buttons.x2_dbl = true;
    }
    break;

  case WM_KEYDOWN:

    /* LPARAM
     * !bits 0-15:   repeat count
     * 16-23:        scan code (depends on OEM)
     * !24:          indicates extended key (right ctrl/alt)
     * 25-28:        reserved
     * 29:           context code, always 0 for WM_KEYDOWN
     * 30:           previous key state. 1 if down before msg was sent, 0 if the
     * key is up 31:           always 0 for wm_keydown
     */

    /* WPARAM
     * the VK code of non-system key
     */
    switch (wparam) {
    case VK_SHIFT:
      keyboard.shift_down = true;
      break;
    case VK_BACK:
      keyboard.backspace_down = true;
      break;
    case VK_TAB:
      keyboard.tab_down = true;
      break;
    case VK_RETURN:
      keyboard.enter_down = true;
      break;
    case VK_CONTROL:
      keyboard.ctrl_down = true;
      break;
    case VK_CAPITAL:
      keyboard.caps_down = true;
      break;
    default:
      u32 ccode =
          MapVirtualKey(wparam,
                        MAPVK_VK_TO_CHAR); // translate the keycode to a char
      u16 repeat_count = (unsigned short)(lparam); // take lower 16 bits
      b8 is_extended_key =
          (lparam & (1 << 24)); // key is either right control or alt key
      keyboard.keys[ccode].held = lparam & (1 << 30);
      keyboard.keys[ccode].down = true;
      keyboard.keys[ccode].character_value = ccode;
    }
  }
  if (msg == WM_KEYUP) {
    /* LPARAM
     * !bits 0-15:   repeat count (always 1 for WM_KEYUP)
     * 16-23:        scan code (depends on OEM)
     * !24:          indicates extended key (right ctrl/alt)
     * 25-28:        reserved
     * 29:           context code, always 0 for WM_KEYUP
     * 30:           previous key state. Always 1 for WM_KEYUP
     * 31:           always 1 for wm_keyup
     */
    switch (wparam) {
    case VK_SHIFT:
      keyboard.shift_down = false;
      break;
    case VK_BACK:
      keyboard.backspace_down = false;
      break;
    case VK_TAB:
      keyboard.tab_down = false;
      break;
    case VK_RETURN:
      keyboard.enter_down = false;
      break;
    case VK_CONTROL:
      keyboard.ctrl_down = false;
      break;
    case VK_CAPITAL:
      keyboard.caps_down = false;
      break;
    default: {
      u32 ccode =
          MapVirtualKey(wparam,
                        MAPVK_VK_TO_CHAR); // translate the keycode to a char
      u16 repeat_count = (unsigned short)(lparam); // take lower 16 bits
      b8 is_extended_key =
          (lparam & (1 << 24)); // key is either right control or alt key
      keyboard.keys[ccode].held = false; //  should be false
      keyboard.keys[ccode].down = false;
      keyboard.keys[ccode].character_value = ccode;
    }
    }
  }
}
