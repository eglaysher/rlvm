// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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
 * @ingroup TransitionEffects
 * @brief  Base LongOperation for all transition effects on DCs.
 */


#ifndef __Effect_hpp__
#define __Effect_hpp__

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "MachineBase/LongOperation.hpp"

class Surface;
class RLMachine;

/**
 * @defgroup TransitionEffects SEL/SELR transition effects
 * 
 * recOpen/grpOpen use a large number of transition effects when
 * compositing DC1 to DC0. These effects are created either based of
 * the \#SEL/\#SELR entries in the Gameexe.ini file, or based off of
 * arguments to a recOpen/grpOpen command. These transition effects
 * are all LongOperations which take over the screen during the
 * transition, surpressing the normal auto-redrawing behaviour.
 *
 * There are a large number of Effect s. Instead of trying to manually
 * create them, use the EffectFactory instead, which will instantiate
 * the correct Effect subclass based off the parameters
 *
 * @see EffectFactory
 *
 * @{
 */

/**
 * Transition effect on DCs.
 * 
 * Effect is the base class from which all transition effects defined
 * on \#SELs derive from. These effects are all implemented as
 * LongOperations on the RLMachine, as they are all long and blocking
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

  /// Whether the orriginal dc0 should be blitted onto the target
  /// surface before we pass control to the effect
  virtual bool blitOriginalImage() const = 0;

  /// Keep track of what machine we're running on so we can send an
  /// appropriate endRealtimeTask() to the eventsystem on destruction
  RLMachine& m_machine;

  /// The source surface (previously known as DC1, before I realized
  /// that temporary surfaces could in fact be part of effects)
  boost::shared_ptr<Surface> m_srcSurface;

  /// The destination surface (previously known as DC0)
  boost::shared_ptr<Surface> m_dstSurface;

protected:
  int width() const { return m_width; }
  int height() const { return m_height; }
  int duration() const { return m_duration; }

  /** 
   * Implements the effect. Usually, this is all that needs to be
   * overriden, other then the public constructor.
   * 
   * @param machine Machine to run on
   * @param currentTime A value [0, m_duration) reprsenting the
   *                    current time of the effect.
   */
  virtual void performEffectForTime(RLMachine& machine, 
                                    int currentTime) = 0;

public:
  /** 
   * Sets up all other variables
   *
   * Note that we add 1 to both width and height; RL is the only
   * system I know of where ranges are inclusive...
   */
  Effect(RLMachine& machine, boost::shared_ptr<Surface> src,
         boost::shared_ptr<Surface> dst,
         int width, int height, int time);

  virtual ~Effect();

  /** 
   * Implements the LongOperation calling interface. This simply keeps
   * track of the current time and calls performEffectForTime() until
   * time > m_duration, when the default implementation simply sets
   * the current dc0 to the original dc0, then blits dc1 onto it.
   */
  virtual bool operator()(RLMachine& machine);

  /**
   * Accessors for which surfaces we're composing. These are public as
   * an ugly hack for ScrollOnScrollOff.cpp.
   */
  Surface& srcSurface() { return *m_srcSurface; }
  Surface& dstSurface() { return *m_dstSurface; }
};

// -----------------------------------------------------------------------

/**
 * LongOperationDecorator used in cases where we need to blit an image
 * to the screen after an Effect finishes. This is most of the cases.
 * This is optional and isn't part of Effect because {rec,grp}OpenBg
 * can take '?' as the name of the image file to load, in which case
 * the Blit doesn't happen.
 *
 * @see Grp_openBg_1
 */
class BlitAfterEffectFinishes : public PerformAfterLongOperationDecorator
{
private:
  /// The source surface (previously known as DC1, before I realized
  /// that temporary surfaces could in fact be part of effects)
  boost::shared_ptr<Surface> m_srcSurface;

  /// The destination surface (previously known as DC0)
  boost::shared_ptr<Surface> m_dstSurface;

  int m_srcX;
  int m_srcY;
  int m_srcWidth;
  int m_srcHeight;
  int m_dstX;
  int m_dstY;
  int m_dstWidth;
  int m_dstHeight;

  virtual void performAfterLongOperation(RLMachine& machine);

public:
  BlitAfterEffectFinishes(LongOperation* in,
                          boost::shared_ptr<Surface> src, 
                          boost::shared_ptr<Surface> dst,
                          int srcX, int srcY, int srcWidth, int srcHeight,
                          int dstX, int dstY, int dstWidth, int dstHeight);

  ~BlitAfterEffectFinishes();
};

/** 
 * Takes a normal Effect and decorates it with the
 * BlitAfterEffectFinishes, so that after the Effect has run, it will
 * do the final blit.
 *
 * @relates BlitAfterEffectFinishes
 * @param[in,out] lop LongOperation to decorate
 * @param src 
 * @param dst 
 */
void decorateEffectWithBlit(LongOperation*& lop, 
                            boost::shared_ptr<Surface> src,
                            boost::shared_ptr<Surface> dst);
                        

// @}

#endif
