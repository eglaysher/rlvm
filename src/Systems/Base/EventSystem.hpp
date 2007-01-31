#ifndef __EventSystem_hpp__
#define __EventSystem_hpp__

#include <boost/scoped_ptr.hpp>

class RLMachine;

class FrameCounter;

/**
 * Generalization of an event system. Reallive's event model is a bit
 * weird; interpreted code will check the state of certain keyboard
 * modifiers, with functions such as CtrlPressed() or ShiftPressed().
 */
class EventSystem
{
private:
  boost::scoped_ptr<FrameCounter> m_frameCounters[255][2];

  /// Counter for the number of things that require realtime
  /// speed. Whenever this is zero, the system will wait 10ms between
  /// rendering frames to be nice to the user and the OS.
  int m_numberOfRealtimeTasks;

  void checkLayerAndCounter(int layer, int counter);

public:
  EventSystem();
  virtual ~EventSystem();

  virtual void executeEventSystem(RLMachine& machine) = 0;

  /** 
   * Returns whether shift is currently pressed.
   * 
   * @return 
   */
  virtual bool shiftPressed() const = 0;

  /** 
   * Returns whether ctrl has been presed since the last invocation of
   * ctrlPresesd().
   *
   * @todo Do I have to keep track of if the key was pressed between
   * two ctrlPressed() invocations?
   */
  virtual bool ctrlPressed() const = 0;

  /** 
   * Returns the number of milliseconds since the program
   * started. Used for timing things.
   */
  virtual unsigned int getTicks() const = 0;

  void setFrameCounter(int layer, int frameCounter, FrameCounter* counter);
  FrameCounter& getFrameCounter(int layer, int frameCounter);
  bool frameCounterExists(int layer, int frameCounter);

//  virtual FrameCounter& getExFrameCounter(int frameCounter) const;

  /** 
   * Idles the program for a certain amount of time in milliseconds.
   * 
   * @param milliseconds Time
   */
  virtual void wait(unsigned int milliseconds) const = 0;


  /**
   * @name Niceness functions
   * 
   * @{
   */
  void beginRealtimeTask();
  void endRealtimeTask();

  /// Returns whether there are any current tasks that require
  /// realtime acces for smooth animation.
  bool canBeNice();

  /// @}

};

#endif
