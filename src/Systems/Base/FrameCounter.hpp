#ifndef __FrameCounter_hpp__
#define __FrameCounter_hpp__

class EventSystem;

/**
 * Frame counter used by RealLive code to ensure events happen at a
 * constant speed. Constant to all System implementations since it is
 * implemented in terms of operations on System classes.a
 *
 * See section "5.13.3 Frame counters" of the RLdev specification for
 * more details.
 */
class FrameCounter
{
public:
  FrameCounter(EventSystem& es, int frameMin, int frameMax, int milliseconds);

  /** 
   * Returns the current value of this frame counter, a value between
   * 
   * @return 
   */
  virtual int readFrame(EventSystem& eventSystem);

private:
  int m_value;
  int m_minValue;
  int m_maxValue;

  bool m_isActive;

  unsigned int m_changeInterval;

  unsigned int m_timeAtLastCheck;
};

#endif
