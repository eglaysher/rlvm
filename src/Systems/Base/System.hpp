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

#ifndef __System_hpp__
#define __System_hpp__

//#include "ScriptModifyable.hpp"

/** 
 * Abstract base class that provides a generalized interface to the
 * text windows.
 *
 */
/* class TextSystem : public ScriptModifyable
{
private:
  int xpos;
  int ypos;

  // The current size of the font
  int fontSizeInPixels;

  /// The text color
  int r, g, b, alpha, filter;

  /// Fast text mode
  bool fastTextMode;

  /// Internal 'no wait' flag
  bool messageNoWait;

  /// Message speed; range from 0 to 255
  char messageSpeed;

public:
  TextSystem() 
    : xpos(0), ypos(0), fontSizeInPixels(0), r(0), g(0), b(0), 
      alpha(0), filter(0), fastTextMode(0)
  {}

  virtual int& xpos() { return xpos; }
  virtual int& ypos() { return ypos; }

  virtual int& fontSizeInPixels() { return fontSizeInPixels; }
  
  virtual int& r() { return r; }
  virtual int& g() { return g; }
  virtual int& b() { return b; }
  virtual int& alpha() { return alpha; }
  virtual int& filter() { return filter; }

  virtual int& fastTextMode() { return fastTextMode; }
  virtual int& messageNoWait() { return messageNoWait; }
  virtual int& messageSpeed() { return messageSpeed; }
*/
  /** 
   * Give a default implementation for alertModified(). This should be
   * overridden by subclasses that need to be alerted when one of the
   * values were modified.
   */
//  virtual void alertModified() { }
//};

class GraphicsSystem;
class EventSystem;
class RLMachine;
  class Gameexe;

/**
 * The system class provides a generalized interface to all the
 * components that make up a local system that may need to be
 * implemented differently on different systems, i.e., sound,
 * graphics, filesystem et cetera.
 *
 * The base System class is an abstract base class that 
 */
class System
{
public:
  virtual ~System() {}

  virtual void run(RLMachine& machine) = 0;

  virtual GraphicsSystem& graphics() = 0;
  virtual EventSystem& event() = 0;
  virtual Gameexe& gameexe() = 0;
//  virtual TextSystem& textSystem() = 0;
//  virtual SoundSystem& soundSystem() = 0;

  
  /** 
   * Locates a file. 
   * 
   * @param filebasename 
   * 
   * @return 
   */
//  virtual std::string& findFile(const std::string& filebasename) = 0;
};

#endif
