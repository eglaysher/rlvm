#ifndef __Effect_hpp__
#define __Effect_hpp__

#include <boost/scoped_ptr.hpp>

#include "MachineBase/LongOperation.hpp"

class Surface;

/**
 * Transition effect on DCs.
 * 
 * Effect is the base class from which all transition effects defined
 * on #SELs derive from. These effects are all implemented as
 * LongOperations on the RLMahcine, as they are all long and blocking
 * operations.
 */
class Effect : public LongOperation
{
private:
  /// Defines the source and destination areas.
  int m_x, m_y, m_width, m_height, m_dx, m_dy;

  /// Defines the duration of in milliseconds
  unsigned int m_duration;

  /// The time since startup when this effect started (in milliseconds)
  unsigned int m_startTime;

  /// The current state of DC1 (we don't own this, so it is a
  /// reference. After blitting to DC1, we treat this as the source
  /// image)
  Surface& m_dc1;

  /// The original state of DC0 before the transition effect started
  /// (we keep this around since it is easier conceptually to redraw
  /// the effect
  boost::scoped_ptr<Surface> m_originalDC0;

  /// Whether the orriginal dc0 should be blitted onto the target
  /// surface before we pass control to the effect
  virtual bool blitOriginalImage() const = 0;

protected:
  int x() const { return m_x; }
  int y() const { return m_y; }
  int width() const { return m_width; }
  int height() const { return m_height; }
  int dx() const { return m_dx; }
  int dy() const { return m_dy; }

  Surface& dc1() { return m_dc1; }

  int duration() const { return m_duration; }

  /** 
   * Implements the effect. Usually, this is all that needs to be
   * overriden, other then the public constructor.
   * 
   * @param currentTime A value [0, m_duration) reprsenting the
   *                    current time of the effect.
   */
  virtual void performEffectForTime(RLMachine& machine, 
                                    int currentTime) = 0;

public:
  /** 
   * Constructor for Effects. The constructor does the following:
   * - Blits imageToBlitToDC1 to DC1 with opacity. (Constructor does
   *   not take ownership)
   * - Sets up all other variables
   */
  Effect(RLMachine& machine, int x, int y, int width, 
         int height, int dx, int dy, int time);

  /** 
   * Implements the LongOperation calling interface. This simply keeps
   * track of the current time and calls performEffectForTime() until
   * time > m_duration, when the default implementation simply sets
   * the current dc0 to the original dc0, then blits dc1 onto it.
   */
  virtual bool operator()(RLMachine& machine);
};

#endif
