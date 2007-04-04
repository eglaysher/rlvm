#ifndef __EventHandler_hpp__
#define __EventHandler_hpp__

enum MouseButton
{
  MOUSE_LEFT = 0,
  MOUSE_RIGHT,
  MOUSE_MIDDLE,
  MOUSE_WHEELUP,
  MOUSE_WHEELDOWN
};

class EventHandler
{
public:
  virtual void mouseButtonStateChanged(MouseButton mouseButton, bool pressed) {}
};

#endif
