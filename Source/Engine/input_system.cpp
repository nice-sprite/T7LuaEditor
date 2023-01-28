#include "input_system.h"
#include "logging.h"
#include <errhandlingapi.h>
#include <memoryapi.h>
#include <time.h>
#include <windowsx.h>
#include <winuser.h>
#define QWORD u64
void InputSystem::init(HWND target_hwnd) {
  msg_buff_count = 100;
  ri_chunk = nullptr;
  ri_chunk_bytewidth = sizeof(RAWINPUT) * msg_buff_count;
  // alloc memory for raw input buffer
  ri_chunk = VirtualAlloc(NULL,
                          ri_chunk_bytewidth,
                          MEM_COMMIT | MEM_RESERVE,
                          PAGE_READWRITE);
  Q_ASSERT(ri_chunk != nullptr);
  register_devices(target_hwnd);
  timer.start();
}

InputSystem &InputSystem::instance() {
  static InputSystem is;
  return is;
}

void InputSystem::shutdown() {
  VirtualFree(ri_chunk, ri_chunk_bytewidth, MEM_DECOMMIT | MEM_RELEASE);
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

bool InputSystem::unregister_devices(HWND target_hwnd) {
  RAWINPUTDEVICE devices[2]{};
  devices[0].hwndTarget = NULL;
  devices[0].usUsagePage = 1;
  devices[0].usUsage = 2;
  devices[0].dwFlags = RIDEV_REMOVE;

  devices[1].hwndTarget = NULL;
  devices[1].usUsagePage = 1;
  devices[1].usUsage = 6;
  devices[1].dwFlags = RIDEV_REMOVE;
  bool result = RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE));
  Q_ASSERT(result);
  LOG_INFO("unregistered raw devices input");
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
  GetRawInputBuffer(nullptr, &buffer_byte_width, sizeof(RAWINPUTHEADER));

  u32 total_block_size = buffer_byte_width * num_messages;

  for (; total_block_size > 0;) {
    u32 ri_count = GetRawInputBuffer((RAWINPUT *)ri_chunk,
                                     &total_block_size,
                                     sizeof(RAWINPUTHEADER));
    if (ri_count == 0 || ri_count == -1) {
      return total_messages;
    }
    total_messages += ri_count;

    RAWINPUT *curr_block = (RAWINPUT *)ri_chunk;
    for (u32 i = 0; i < ri_count; ++i) {
      parse_raw_input_packet(*curr_block);
      curr_block = NEXTRAWINPUTBLOCK(curr_block);
    }
  }
  return 0;
}

void InputSystem::parse_raw_input_packet(RAWINPUT const &packet) {
  if (packet.header.dwType == RIM_TYPEMOUSE) {
    mouse_delta.dx += packet.data.mouse.lLastX;
    mouse_delta.dy += packet.data.mouse.lLastY;
    // ignore the input if we recieved the message but teh
    // window is not focused
    if (!window_active) {
      mouse_delta.dx = 0;
      mouse_delta.dy = 0;
    }
  }
  if (packet.header.dwType == RIM_TYPEKEYBOARD) {
    RAWKEYBOARD kbd = packet.data.keyboard;
  }
}

// handles the WM_XXX messages from wndproc
void InputSystem::handle_win32_input(HWND hwnd,
                                     u32 msg,
                                     WPARAM wparam,
                                     LPARAM lparam) {

  timer.tick();
  u64 timestamp = timer.elapsed_ms();

  // if (msg == WM_LBUTTONDOWN) {
  //   down_pos = mouse_pos;
  //   mouse_buttons.left = true;

  //   queue_mouse_event(EventType::DragStart,
  //                     mouse_pos,
  //                     mouse_delta,
  //                     mouse_buttons);

  //   queue_mouse_event(EventType::MouseLeftClick,
  //                     mouse_pos,
  //                     mouse_delta,
  //                     mouse_buttons);
  // }

  switch (msg) {
  case WM_ACTIVATE:
    if (LOWORD(wparam) == WA_ACTIVE || LOWORD(wparam) == WA_CLICKACTIVE) {
      window_active = true;
    } else if (LOWORD(wparam) == WA_INACTIVE) {
      window_active = false;
    }
    break;
  case WM_MOUSEWHEEL:
    queue_mouse_event(EventType::MouseScroll,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons,
                      GET_WHEEL_DELTA_WPARAM(wparam));
    break;

  case WM_MOUSEMOVE:
    mouse_pos.x = (f32)GET_X_LPARAM(lparam);
    mouse_pos.y = (f32)GET_Y_LPARAM(lparam);
    if (mouse_buttons.left) {
      queue_mouse_event(EventType::Dragging,
                        mouse_pos,
                        mouse_delta,
                        mouse_buttons);
    }
    break;

  case WM_LBUTTONDOWN:
    mouse_buttons.left = true;

    queue_mouse_event(EventType::DragStart,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);

    queue_mouse_event(EventType::MouseLeftClick,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_LBUTTONUP:
    mouse_buttons.left = false;
    mouse_buttons.left_dbl = false;
    queue_mouse_event(EventType::DragEnd,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);

    queue_mouse_event(EventType::MouseLeftRelease,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_RBUTTONDOWN:
    mouse_buttons.right = true;
    queue_mouse_event(EventType::MouseRightClick,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_RBUTTONUP:
    mouse_buttons.right = false;
    mouse_buttons.right_dbl = false;
    queue_mouse_event(EventType::MouseRightRelease,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_MBUTTONDOWN:
    mouse_buttons.middle = true;
    queue_mouse_event(EventType::MouseMiddleClick,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_MBUTTONUP:
    mouse_buttons.middle = false;
    mouse_buttons.middle_dbl = false;
    queue_mouse_event(EventType::MouseMiddleRelease,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_XBUTTONDOWN:
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
      mouse_buttons.x1 = true;
      queue_mouse_event(EventType::MouseExtraClick1,
                        mouse_pos,
                        mouse_delta,
                        mouse_buttons);
    }
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
      mouse_buttons.x2 = true;
      queue_mouse_event(EventType::MouseExtraClick2,
                        mouse_pos,
                        mouse_delta,
                        mouse_buttons);
    }

    break;

  case WM_XBUTTONUP:
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
      mouse_buttons.x1 = false;
      mouse_buttons.x1_dbl = false;
      queue_mouse_event(EventType::MouseExtraRelease1,
                        mouse_pos,
                        mouse_delta,
                        mouse_buttons);
    }
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
      mouse_buttons.x2 = false;
      mouse_buttons.x2_dbl = false;
      queue_mouse_event(EventType::MouseExtraRelease2,
                        mouse_pos,
                        mouse_delta,
                        mouse_buttons);
    }
    break;

    // handle double clicks
  case WM_RBUTTONDBLCLK:
    mouse_buttons.right_dbl = true;
    queue_mouse_event(EventType::MouseRightDblClick,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_LBUTTONDBLCLK:
    mouse_buttons.left_dbl = true;
    queue_mouse_event(EventType::MouseLeftDblClick,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_MBUTTONDBLCLK:
    mouse_buttons.middle_dbl = true;
    queue_mouse_event(EventType::MouseMiddleDblClick,
                      mouse_pos,
                      mouse_delta,
                      mouse_buttons);
    break;

  case WM_XBUTTONDBLCLK:
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) {
      mouse_buttons.x1_dbl = true;
      queue_mouse_event(EventType::MouseExtraDblClick1,
                        mouse_pos,
                        mouse_delta,
                        mouse_buttons);
    }
    if (GET_XBUTTON_WPARAM(wparam) == XBUTTON2) {
      mouse_buttons.x2_dbl = true;
      queue_mouse_event(EventType::MouseExtraDblClick2,
                        mouse_pos,
                        mouse_delta,
                        mouse_buttons);
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
    break;

  case WM_KEYUP:
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
    default:
      u32 ccode =
          MapVirtualKey(wparam,
                        MAPVK_VK_TO_CHAR); // translate the keycode to a char
      u16 repeat_count = (u16)(lparam);
      b8 is_extended_key =
          (lparam & (1 << 24)); // key is either right control or alt key
      keyboard.keys[ccode].held = false; //  should be false
      keyboard.keys[ccode].down = false;
      keyboard.keys[ccode].character_value = ccode;
    }
    break;
  }
  /*  if (is_msg_mouse(msg)) {
      mouse_event.delta_mouse = mouse_delta;
      mouse_event.mouse_pos = mouse_pos;
      mouse_event.wheel_delta = mouse_wheel;
      mouse_event.buttons = mouse_buttons;
      mouse_event.timestamp = timestamp;
      dispatch_listeners(mouse_event);
    }
  */
}

void InputSystem::update() {
#ifdef INPUT_DEBUG
 // ImGui::Text("mouse delta: %d, %d", mouse_delta.dx, mouse_delta.dy);
 // ImGui::Text("cursor pos: %f %f", mouse_pos.x, mouse_pos.y);
 // ImGui::Text("wheel_delta : %d ", mouse_wheel);

 // ImGui::Text("left %d ", mouse_buttons.left);
 // ImGui::Text("right %d ", mouse_buttons.right);
 // ImGui::Text("middle %d ", mouse_buttons.middle);
 // ImGui::Text("x1 %d ", mouse_buttons.x1);
 // ImGui::Text("x2 %d ", mouse_buttons.x2);
 // ImGui::Text("right_dbl %d ", mouse_buttons.right_dbl);
 // ImGui::Text("left_dbl %d ", mouse_buttons.left_dbl);
 // ImGui::Text("middle_dbl %d ", mouse_buttons.middle_dbl);
 // ImGui::Text("x1_dbl %d ", mouse_buttons.x1_dbl);
 // ImGui::Text("x2_dbl %d ", mouse_buttons.x2_dbl);
#endif
  u32 i = 0;
  // TODO some events might still want to fire even if the window is not active
  for (; i < event_count && window_active; ++i) {
    dispatch_listeners(frame_events[i]);
  }
  event_count = 0;
}

void InputSystem::end_frame() {
  mouse_buttons.left_dbl = false;
  mouse_buttons.right_dbl = false;
  mouse_buttons.middle_dbl = false;
  mouse_buttons.x1_dbl = false;
  mouse_buttons.x2_dbl = false;
  this->mouse_wheel = 0;
}

bool InputSystem::key_down(u8 key) { return keyboard.keys[key].down; }

bool InputSystem::add_mouse_listener(const MouseEventListener listener) {
  if (mouse_listener_count < MaxMouseListeners) {
    mouse_listeners[mouse_listener_count] = listener;
    mouse_listener_count++;
    return true;
  } else {
    return false;
  }
}

u32 InputSystem::dispatch_listeners(const MouseEvent mouse_event) {
  u32 i;
  for (i = 0; i < mouse_listener_count; ++i) {
    MouseEventListener listener = mouse_listeners[i];
    ((void (*)(void *self, MouseEvent))(listener.function))(listener.self,
                                                            mouse_event);
  }
  return i;
}

bool InputSystem::is_msg_mouse(u32 msg) {
  switch (msg) {

  case WM_MOUSEWHEEL:
  case WM_MOUSEMOVE:
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
  case WM_RBUTTONDBLCLK:
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_XBUTTONDBLCLK:
    return true;

  default:
    return false;
  }
}

// queues a mouse event to be dispatched at the end of the frame
// all parameters are passed seperately so that every mouse event has uniform
// information contained in it even if not all events would use all the info.
void InputSystem::queue_mouse_event(EventType etype,
                                    ScreenPos mouse_pos,
                                    MouseDelta mouse_delta,
                                    MouseButtons buttons,
                                    WheelDelta wheel_delta) {

  if (event_count < MaxEventsPerFrame) {
    frame_events[event_count].type = etype;
    frame_events[event_count].mouse_pos = mouse_pos;
    frame_events[event_count].mouse_delta = mouse_delta;
    frame_events[event_count].buttons = buttons;
    frame_events[event_count].wheel_delta = wheel_delta;
    event_count++;
  } else {
    LOG_WARNING("Something is filling up the frame_events buffer!");
  }
}
