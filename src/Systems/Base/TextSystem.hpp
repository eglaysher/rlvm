#ifndef __TextSystem_hpp__
#define __TextSystem_hpp__

class RLMachine;

class TextSystem
{
private:
  /// Fast text mode
  bool m_fastTextMode;

  /// Internal 'no wait' flag
  bool m_messageNoWait;

  /// Message speed; range from 0 to 255
  char m_messageSpeed;

public:
  virtual void setActiveTextWindow(RLMachine& machine, int window) = 0;
  virtual void render(RLMachine& machine) = 0;

  void setFastTextMode(int i) { m_fastTextMode = i; }
  int fastTextMode() const { return m_fastTextMode; }

  void setMessageNoWait(int i) { m_messageNoWait = i; }
  int messageNoWait() const { return m_messageNoWait; }

  void setMessageSpeed(int i) { m_messageSpeed = i; }
  int messageSpeed() const { return m_messageSpeed; }
};

#endif



