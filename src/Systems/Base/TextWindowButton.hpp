// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 El Riot
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

#ifndef __TextWindowButton_hpp__
#define __TextWindowButton_hpp__

#include <boost/shared_ptr.hpp>

#include "libReallive/gameexe.h"

class RLMachine;
class TextWindow;
class Surface;

/**
 * Describes the state of a Waku button
 */
enum ButtonState
{
  BUTTONSTATE_BUTTON_NOT_USED = -1,
  BUTTONSTATE_NORMAL = 0,   
  BUTTONSTATE_DISABLED = 1,
  BUTTONSTATE_HIGHLIGHTED = 2,
  BUTTONSTATE_PRSEED = 3,
  BUTTONSTATE_ACTIVATED = 4
};

class TextWindowButton
{
private:
  std::vector<int> m_location;
  ButtonState m_state;

public:
  TextWindowButton();
  TextWindowButton(bool use, GameexeInterpretObject locationBox);
  ~TextWindowButton();

  /// Returns the absolute screen coordinate of this button.
  int xLocation(TextWindow& window);
  int yLocation(TextWindow& window);

  // 
  void render(RLMachine& machine, TextWindow& window,
              const boost::shared_ptr<Surface>& buttons, 
              int basePattern);
};

#endif
