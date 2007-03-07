// This file is part of RLVM, a RealLive virtual machine clone.
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

#ifndef __RLTimer_hpp__
#define __RLTimer_hpp__

class EventSystem;

/**
 * Frame counter used by RealLive code to ensure events happen at a
 * constant speed. Constant to all System implementations since it is
 * implemented in terms of operations on System classes.a
 *
 * See section "5.13.3 Frame counters" of the RLdev specification for
 * more details.
 */
class RLTimer
{
public:
  RLTimer();
  ~RLTimer();

  /** 
   * Returns the current value of this frame counter, a value between
   * 
   * @return 
   */
  unsigned int read(EventSystem& events);

  /** 
   * 
   * @param value New value for the frame counter
   */
  void set(EventSystem& events, unsigned int value = 0);

private:
  unsigned int m_timeAtLastSet;
};

#endif
