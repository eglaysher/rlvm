// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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
#include <boost/function.hpp>

class RLMachine;
class TextPageElement;
class SetWindowTextPageElement;
class TextTextPageElement;

// -----------------------------------------------------------------------

class TextPage : public boost::noncopyable
{
public:
  TextPage(RLMachine& in_sys, int window_num);
  TextPage(const TextPage& rhs);
  ~TextPage();

  TextPage& operator=(const TextPage& rhs);
  void swap(TextPage& rhs);

  void replay(bool is_active_page);

  int numberOfCharsOnPage() const { return number_of_chars_on_page_; }

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
   * Add this character to the most recent text render operation on
   * this page's backlog, and then render it, minding the kinsoku
   * spacing rules.
   */
  bool character(const std::string& current, const std::string& next);

  /**
   * Displays a name. This function will be called by the
   * TextoutLongOperation.
   */
  void name(const std::string& name, const std::string& next_char);

  /**
   * Forces a hard line brake.
   */
  void hardBrake();

  /**
   * Sets the indentation to the x part of the current insertion point.
   */
  void setIndentation();

  /**
   * Resets the indentation.
   */
  void resetIndentation();

  /**
   * Sets the text foreground to the color passed in, up until the
   * next pause().
   */
  void fontColour(const int color);

  /**
   * Marks the current character as the beginning of a phrase that has
   * rubytext over it.
   */
  void markRubyBegin();

  /**
   * Display the incoming phrase as the rubytext for the text since
   * markRubyBegin() was called.
   */
  void displayRubyText(const std::string& utf8str);

  void setInsertionPointX(int x);
  void setInsertionPointY(int y);
  void offsetInsertionPointX(int offset);
  void offsetInsertionPointY(int offset);

  /**
   * This is a hack to get the backlog color working. This adds a
   * SetToRightStartingColorElement element to the TextPage, which, on
   * replay, simply checks to see if we're redisplaying a backlog
   * page and sets the color to the backlog color if we are.
   */
  void addSetToRightStartingColorElement();
  /// @}

  /**
   * Queries the corresponding TextWindow to see if it is full. Used
   * to implement implicit pauses when a page is full.
   */
  bool isFull() const;

  /**
   * Queries to see if there has been an invocation of
   * markRubyBegin(), but not the closing displayRubyText().
   */
  bool inRubyGloss() const { return in_ruby_gloss_; }

private:
  /// Performs the passed in action and then adds it to |elements_to_replay_|.
  void addAction(const boost::function<void(TextPage&, bool)>& action);

  /// All subclasses of TextPageElement are friends of TextPage for
  /// tight coupling.
  ///
  /// TODO: These classes suck. Any way to get rid of them with a function<>?
  friend class TextPageElement;
  friend class SetWindowTextPageElement;
  friend class TextTextPageElement;
  friend class NamePageElement;
  friend class HardBreakElement;
  friend class ResetIndentationElement;
  friend class FontColourElement;
  friend class SetToRightStartingColorElement;
  friend class MarkRubyBeginElement;
  friend class DisplayRubyTextElement;

  boost::ptr_vector<TextPageElement> elements_to_replay_;

  RLMachine* machine_;

  /// Current window that this page is rendering into
  int window_num_;

  /// Number of characters on this page (used in automode)
  int number_of_chars_on_page_;

  bool in_ruby_gloss_;

  /**
   * @name Private implementations
   *
   * These methods are what actually does things. They output to the
   * screen, etc.
   *
   * @{
   */

  bool character_impl(const std::string& c, const std::string& next_char);

  void name_impl(const std::string& name, const std::string& next_char,
                 bool is_active_page);

  void hard_brake_impl(bool is_active_page);

  void set_indentation_impl(bool is_active_page);

  void reset_indentation_impl(bool is_active_page);

  void font_colour_impl(const int color, bool is_active_page);

  void mark_ruby_begin_impl(bool is_active_page);

  void display_ruby_text_impl(const std::string& utf8str, bool is_active_page);

  void set_insertion_point_x_impl(int x, bool is_active_page);
  void set_insertion_point_y_impl(int y, bool is_active_page);
  void offset_insertion_point_x_impl(int offset, bool is_active_page);
  void offset_insertion_point_y_impl(int offset, bool is_active_page);

  void set_to_right_starting_color_impl(bool is_active_page);
  /// @}
};

#endif
