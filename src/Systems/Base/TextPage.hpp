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
  friend class NamePageElement;
  friend class HardBreakElement;
  friend class ResetIndentationElement;

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

  bool character_impl(const std::string& c, const std::string& nextChar);

  void name_impl(const std::string& name, const std::string& nextChar);

  void hardBrake_impl();

  void resetIndentation_impl();

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
  bool character(const std::string& current, const std::string& next);

  /**
   * Displays a name. This function will be called by the
   * TextoutLongOperation.
   */
  void name(const std::string& name, const std::string& nextChar);

  /** 
   * Forces a hard line brake.
   */
  void hardBrake();

  /** 
   * Resets the indentation.
   */
  void resetIndentation();

  /// @}


  /** 
   * Queries the corresponding TextWindow to see if it is full. Used
   * to implement implicit pauses when a page is full.
   */
  bool isFull() const;
};

#endif
