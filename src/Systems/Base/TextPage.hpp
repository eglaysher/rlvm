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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_TEXTPAGE_HPP_
#define SRC_SYSTEMS_BASE_TEXTPAGE_HPP_

#include <boost/ptr_container/ptr_vector.hpp>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

class TextPageElement;
class SetWindowTextPageElement;
class System;
class TextTextPageElement;

// A sequence of replayable commands that write to or modify a window, such as
// displaying characters and changing font information.
//
// The majority of public methods in TextPage simply call the private versions
// of these methods, and add the appropriate TextPageElement to this page's
// back log for replay.
class TextPage {
 public:
  TextPage(System& system, int window_num);
  TextPage(const TextPage& rhs);
  ~TextPage();

  TextPage& operator=(const TextPage& rhs);
  void swap(TextPage& rhs);

  // Replays every recordable action called on this TextPage.
  void replay(bool is_active_page);

  // Returns the number of characters printed with character() and name().
  int numberOfCharsOnPage() const { return number_of_chars_on_page_; }

  // Add this character to the most recent text render operation on
  // this page's backlog, and then render it, minding the kinsoku
  // spacing rules.
  bool character(const std::string& current, const std::string& rest);

  // Displays a name. This function will be called by the
  // TextoutLongOperation.
  void name(const std::string& name, const std::string& next_char);

  // Puts a koe marker in the TextPage. This is only displayed during
  // scrollback.
  void koeMarker(int id);

  // Forces a hard line brake.
  void hardBrake();

  // Sets the indentation to the x part of the current insertion point.
  void setIndentation();

  // Resets the indentation.
  void resetIndentation();

  // Sets the text foreground to the colour passed in, up until the
  // next pause().
  void fontColour(const int colour);

  // Changes the size of text.
  void defaultFontSize();
  void fontSize(const int size);

  // Marks the current character as the beginning of a phrase that has
  // rubytext over it.
  void markRubyBegin();

  // Display the incoming phrase as the rubytext for the text since
  // markRubyBegin() was called.
  void displayRubyText(const std::string& utf8str);

  void setInsertionPointX(int x);
  void setInsertionPointY(int y);
  void offsetInsertionPointX(int offset);
  void offsetInsertionPointY(int offset);

  // Sets the face in slot |index| to filename.
  void faceOpen(const std::string& filename, int index);

  // Removes the face in slot |index|.
  void faceClose(int index);

  // This is a hack to get the backlog colour working. This adds a
  // SetToRightStartingColorElement element to the TextPage, which, on
  // replay, simply checks to see if we're redisplaying a backlog
  // page and sets the colour to the backlog colour if we are.
  void addSetToRightStartingColorElement();

  // Queries the corresponding TextWindow to see if it is full. Used
  // to implement implicit pauses when a page is full.
  bool isFull() const;

  // Queries to see if there has been an invocation of
  // markRubyBegin(), but not the closing displayRubyText().
  bool inRubyGloss() const { return in_ruby_gloss_; }

 private:
  // All subclasses of TextPageElement are friends of TextPage for
  // tight coupling.
  friend class TextPageElement;
  friend class TextTextPageElement;

  // Performs the passed in action and then adds it to |elements_to_replay_|.
  void addAction(const std::function<void(TextPage&, bool)>& action);

  // Private implementations; These methods are what actually does things. They
  // output to the screen, etc.
  bool CharacterImpl(const std::string& c, const std::string& rest);
  void VoidCharacterImpl(const std::string& c, const std::string& rest);
  void NameImpl(const std::string& name,
                const std::string& next_char,
                bool is_active_page);
  void KoeMarkerImpl(int id, bool is_active_page);
  void HardBrakeImpl(bool is_active_page);
  void SetIndentationImpl(bool is_active_page);
  void ResetIndentationImpl(bool is_active_page);
  void FontColourImpl(const int colour, bool is_active_page);
  void DefaultFontSizeImpl(bool is_active_page);
  void FontSizeImpl(int size, bool is_active_page);
  void MarkRubyBeginImpl(bool is_active_page);
  void DisplayRubyTextImpl(const std::string& utf8str, bool is_active_page);
  void SetInsertionPointXImpl(int x, bool is_active_page);
  void SetInsertionPointYImpl(int y, bool is_active_page);
  void OffsetInsertionPointXImpl(int offset, bool is_active_page);
  void OffsetInsertionPointYImpl(int offset, bool is_active_page);
  void FaceOpenImpl(std::string filename, int index, bool is_active_page);
  void FaceCloseImpl(int index, bool is_active_page);
  void SetToRightStartingColourImpl(bool is_active_page);

  boost::ptr_vector<TextPageElement> elements_to_replay_;

  System* system_;

  // Current window that this page is rendering into
  int window_num_;

  // Number of characters on this page (used in automode)
  int number_of_chars_on_page_;

  // Whether markRubyBegin() was called but displayRubyText() hasn't yet been
  // called.
  bool in_ruby_gloss_;
};

#endif  // SRC_SYSTEMS_BASE_TEXTPAGE_HPP_
