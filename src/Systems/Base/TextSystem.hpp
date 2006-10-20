
class TextSystem
{
private:

  
  /// Fast text mode
  bool fastTextMode;

  /// Internal 'no wait' flag
  bool messageNoWait;

  /// Message speed; range from 0 to 255
  char messageSpeed;
public:
  virtual void setFastTextMode(int i) const { fastTextMode = i; }
  virtual int& fastTextMode() { return fastTextMode; }

  virtual void setMessageNoWait(int i) const { messageNoWait = i; }
  virtual int& messageNoWait() { return messageNoWait; }

  virtual void setMessageSpeed(int i) const { messageSpeed = i; }
  virtual int& messageSpeed() { return messageSpeed; }
};
