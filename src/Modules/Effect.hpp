// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

/**
 * @file   Effect.hpp
 * @author Elliot Glaysher
 * @date   Thu Nov  2 20:35:25 2006
 * 
 * @brief  Base LongOperation for all transition effects on DCs.
 */


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
  /// Defines the size of the screen; since effects update the entire screen.
  int m_width, m_height;

  /// Defines the duration of in milliseconds
  unsigned int m_duration;

  /// The time since startup when this effect started (in milliseconds)
  unsigned int m_startTime;

//   /// The current state of DC1 (we don't own this, so it is a
//   /// reference. After blitting to DC1, we treat this as the source
//   /// image)
//   Surface& m_dc1;

//   /// The original state of DC0 before the transition effect started
//   /// (we keep this around since it is easier conceptually to redraw
//   /// the effect
//   boost::scoped_ptr<Surface> m_originalDC0;

  /// Whether the orriginal dc0 should be blitted onto the target
  /// surface before we pass control to the effect
  virtual bool blitOriginalImage() const = 0;

protected:
  int width() const { return m_width; }
  int height() const { return m_height; }

//  Surface& dc1() { return m_dc1; }

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
   *
   * Note that we add 1 to both width and height; RL is the only
   * system I know of where ranges are inclusive...
   */
  Effect(RLMachine& machine, int width, int height, int time);

  /** 
   * Implements the LongOperation calling interface. This simply keeps
   * track of the current time and calls performEffectForTime() until
   * time > m_duration, when the default implementation simply sets
   * the current dc0 to the original dc0, then blits dc1 onto it.
   */
  virtual bool operator()(RLMachine& machine);
};

#endif
