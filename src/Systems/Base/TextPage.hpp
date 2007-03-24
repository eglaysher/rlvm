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

#ifndef __TextPage_hpp__
#define __TextPage_hpp__

#include <boost/ptr_container/ptr_vector.hpp>

class RLMachine;
class TextPageElement;
class SetWindowTextPageElement;
class TextTextPageElement;


// -----------------------------------------------------------------------

class TextPage : public boost::noncopyable
{
private:
  /// All subclasses of TextPageElement are friends of TextPage for
  /// tight coupling. 
  friend class TextPageElement;
  friend class SetWindowTextPageElement;
  friend class TextTextPageElement;

  boost::ptr_vector<TextPageElement> m_elementsToReplay;

  RLMachine& m_machine;

  /// Current window that this page is rendering into
  int m_currentWindow;

  /**
   * @name Private implementations
   * 
   * These methods are what actually does things. They output to the
   * screen, etc. 
   */

  void setWindow_impl(int window);

  void text_impl(const std::string& text);

  /// @}

public:
  TextPage(RLMachine& inSys);

  ~TextPage();

  /**
   * @name Public operations
   * 
   * These methods simply call the private versions of these methods,
   * and add the appropriate TextPageElement to this page's back log
   * for replay.
   *
   * @{
   */

  /** 
   * Sets the window we render to.
   */
  void setWindow(int windowNum);

  /**
   * Add this character to the most recent text render operation on
   * this page's backlog, and then render it, minding the kinsoku
   * spacing rules. 
   */
  void character(const std::string& current, const std::string& next);

  /**
   * Add one or more UTF-8 character dropped in a raw string. We leave
   * things in UTF-8 from textPage on for both space, and for the
   * serious problem that our TTF rendering library thinks uses a
   * character of unsigned 16 bits, thus not being able to handle the
   * entire unicode standard. :(
   */
  void text(const std::string& text);

  /// @}
};

#endif
