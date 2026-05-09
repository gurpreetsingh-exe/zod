#include "application/window.hh"
#include "window_api.hh"

namespace x11 {

using _XInternAtom = Atom (*)(Display*, const char*, Bool);
using _XSendEvent = int (*)(Display*, Window, int, long, XEvent*);
using _XQueryPointer = int (*)(Display*, Window, Window*, Window*, int*, int*,
                               int*, int*, unsigned int*);
using _XUngrabPointer = int (*)(Display*, Time);
using _XFlush = int (*)(Display*);

_XInternAtom InternAtom = nullptr;
_XSendEvent SendEvent = nullptr;
_XQueryPointer QueryPointer = nullptr;
_XUngrabPointer UngrabPointer = nullptr;
_XFlush Flush = nullptr;

auto init_x11() -> void {
  auto f = [](const char* path) {
    return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
  };

#if defined(PLATFORM_LINUX)
#  if defined(__CYGWIN__)
  void* module = f("libX11-6.so");
#  elif defined(__OpenBSD__) || defined(__NetBSD__)
  void* module = f("libX11.so");
#  else
  void* module = f("libX11.so.6");
#  endif
#endif

  InternAtom = (_XInternAtom)dlsym(module, "XInternAtom");
  SendEvent = (_XSendEvent)dlsym(module, "XSendEvent");
  QueryPointer = (_XQueryPointer)dlsym(module, "XQueryPointer");
  UngrabPointer = (_XUngrabPointer)dlsym(module, "XUngrabPointer");
  Flush = (_XFlush)dlsym(module, "XFlush");
}

auto send_window_event(Display* display, Window x11win, int root_x, int root_y,
                       int direction) -> void {
  Atom moveresize = InternAtom(display, "_NET_WM_MOVERESIZE", False);

  XEvent e {};
  e.xclient.type = ClientMessage;
  e.xclient.serial = 0;
  e.xclient.send_event = True;
  e.xclient.message_type = moveresize;
  e.xclient.window = x11win;
  e.xclient.format = 32;

  e.xclient.data.l[0] = root_x;
  e.xclient.data.l[1] = root_y;
  e.xclient.data.l[2] = direction;
  e.xclient.data.l[3] = Button1;
  e.xclient.data.l[4] = 0;

  UngrabPointer(display, CurrentTime);
  SendEvent(display, DefaultRootWindow(display), False,
            SubstructureRedirectMask | SubstructureNotifyMask, &e);
  Flush(display);
}

auto get_resize_direction(zod::f64 x, zod::f64 y, int window_width,
                          int window_height) -> int {
  const int border = 8;

  auto left = x < border;
  auto right = x > window_width - border;
  auto top = y < border;
  auto bottom = y > window_height - border;

  if (top and left)
    return 0;
  if (top and right)
    return 2;
  if (bottom and right)
    return 4;
  if (bottom and left)
    return 6;
  if (top)
    return 1;
  if (bottom)
    return 5;
  if (left)
    return 7;
  if (right)
    return 3;

  return -1;
}

auto mouse_down_event(GLFWwindow* win, zod::f64 x, zod::f64 y, int win_width,
                      int win_height) -> void {
  auto* display = glfwGetX11Display();
  auto x11w = glfwGetX11Window(win);
  auto root = DefaultRootWindow(display);

  int root_x, root_y;
  int win_x, win_y;
  auto mask = (unsigned int)0;
  auto child = Window();
  QueryPointer(display, root, &root, &child, &root_x, &root_y, &win_x, &win_y,
               &mask);
  auto dir = x11::get_resize_direction(x, y, win_width, win_height);
  if (dir != -1) {
    x11::send_window_event(display, x11w, root_x, root_y, dir);
  } else if (y < zod::Window::TitleBarHeight) {
    x11::send_window_event(display, x11w, root_x, root_y, 8);
  }
}

} // namespace x11
