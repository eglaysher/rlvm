// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "Systems/Base/TextSystem.hpp"

#include <map>

#include "MachineBase/Memory.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextKeyCursor.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/gameexe.h"

#include <boost/bind.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using boost::bind;
using boost::shared_ptr;
using std::back_inserter;
using std::cerr;
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;

const unsigned int MAX_PAGE_HISTORY = 100;

// -----------------------------------------------------------------------
// TextSystemGlobals
// -----------------------------------------------------------------------
TextSystemGlobals::TextSystemGlobals()
    : auto_mode_base_time(100), auto_mode_char_time(100), message_speed(30),
      font_weight(0), font_shadow(1)
{}

// -----------------------------------------------------------------------

TextSystemGlobals::TextSystemGlobals(Gameexe& gexe)
    : auto_mode_base_time(100),
      auto_mode_char_time(100),
      message_speed(gexe("INIT_MESSAGE_SPEED").to_int(30)),
      font_weight(0), font_shadow(1) {
  GameexeInterpretObject in_window_attr(gexe("WINDOW_ATTR"));
  if (in_window_attr.exists())
    window_attr = in_window_attr;
}

// -----------------------------------------------------------------------
// TextSystem
// -----------------------------------------------------------------------
TextSystem::TextSystem(System& system, Gameexe& gexe)
    : auto_mode_(false),
      ctrl_key_skip_(true),
      fast_text_mode_(false),
      message_no_wait_(false),
      active_window_(0),
      is_reading_backlog_(false),
      current_pageset_(new PageSet),
      in_pause_state_(false),
      // #WINDOW_*_USE
      move_use_(false),
      clear_use_(false),
      read_jump_use_(false),
      automode_use_(false),
      msgbk_use_(false),
      msgbkleft_use_(false),
      msgbkright_use_(false),
      exbtn_use_(false),
      globals_(gexe),
      system_visible_(true),
      skip_mode_(false),
      kidoku_read_(false),
      in_selection_mode_(false),
      system_(system) {
  GameexeInterpretObject ctrl_use(gexe("CTRL_USE"));
  if (ctrl_use.exists())
    ctrl_key_skip_ = ctrl_use;

  checkAndSetBool(gexe, "WINDOW_MOVE_USE", move_use_);
  checkAndSetBool(gexe, "WINDOW_CLEAR_USE", clear_use_);
  checkAndSetBool(gexe, "WINDOW_READJUMP_USE", read_jump_use_);
  checkAndSetBool(gexe, "WINDOW_AUTOMODE_USE", automode_use_);
  checkAndSetBool(gexe, "WINDOW_MSGBK_USE", msgbk_use_);
  checkAndSetBool(gexe, "WINDOW_MSGBKLEFT_USE", msgbkleft_use_);
  checkAndSetBool(gexe, "WINDOW_MSGBKRIGHT_USE", msgbkright_use_);
  checkAndSetBool(gexe, "WINDOW_EXBTN_USE", exbtn_use_);

  previous_page_it_ = previous_page_sets_.end();
}

// -----------------------------------------------------------------------

TextSystem::~TextSystem() {
}

// -----------------------------------------------------------------------

void TextSystem::executeTextSystem() {
  // Check to see if the cursor is displayed
  if (showWindow(active_window_)) {
    WindowMap::iterator it = text_window_.find(active_window_);
    if (it != text_window_.end() && it->second->isVisible() &&
        in_pause_state_ && !isReadingBacklog()) {
      if (!text_key_cursor_)
        setKeyCursor(0);

      text_key_cursor_->execute();
    }
  }

  // Let each window update any TextWindowButton s.
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    if (showWindow(it->first)) {
      it->second->execute();
    }
  }
}

// -----------------------------------------------------------------------

void TextSystem::render(std::ostream* tree) {
  if (systemVisible()) {
    if (tree) {
      *tree << "Text System:" << endl;
    }

    for (WindowMap::iterator it = text_window_.begin();
         it != text_window_.end(); ++it) {
      if (showWindow(it->first)) {
        it->second->render(tree);
      }
    }

    if (showWindow(active_window_)) {
      WindowMap::iterator it = text_window_.find(active_window_);

      if (it != text_window_.end() && it->second->isVisible() &&
          in_pause_state_ && !isReadingBacklog()) {
        if (!text_key_cursor_)
          setKeyCursor(0);

        text_key_cursor_->render(*it->second, tree);
      }
    }
  }
}

// -----------------------------------------------------------------------

void TextSystem::hideTextWindow(int win_number) {
  WindowMap::iterator it = text_window_.find(win_number);
  if (it != text_window_.end()) {
    it->second->setVisible(0);
  }
}

// -----------------------------------------------------------------------

void TextSystem::hideAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->setVisible(0);
  }
}

// -----------------------------------------------------------------------

void TextSystem::hideAllTextWindowsExcept(int i) {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    if (it->first != i) {
      it->second->setVisible(0);
    }
  }
}

// -----------------------------------------------------------------------

void TextSystem::showTextWindow(int win_number) {
  WindowMap::iterator it = text_window_.find(win_number);
  if (it != text_window_.end()) {
    it->second->setVisible(1);
  }
}

// -----------------------------------------------------------------------

void TextSystem::showAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->setVisible(1);
  }
}

// -----------------------------------------------------------------------

void TextSystem::clearAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->clearWin();
  }
}

// -----------------------------------------------------------------------

void TextSystem::setVisualOverride(int win_number, bool show_window) {
  window_visual_override_[win_number] = show_window;
}

// -----------------------------------------------------------------------

void TextSystem::setVisualOverrideAll(bool show_window) {
  for (int i = 0; i < 64; ++i) {
    window_visual_override_[i] = show_window;
  }
}

// -----------------------------------------------------------------------

void TextSystem::clearVisualOverrides() {
  window_visual_override_.clear();
}

// -----------------------------------------------------------------------

boost::shared_ptr<TextWindow> TextSystem::currentWindow() {
  return textWindow(active_window_);
}

// -----------------------------------------------------------------------

void TextSystem::checkAndSetBool(Gameexe& gexe, const std::string& key,
                                 bool& out) {
  GameexeInterpretObject key_obj(gexe(key));
  if (key_obj.exists())
    out = key_obj.to_int();
}

// -----------------------------------------------------------------------

void TextSystem::expireOldPages() {
  while (previous_page_sets_.size() > MAX_PAGE_HISTORY)
    previous_page_sets_.pop_front();
}

// -----------------------------------------------------------------------

bool TextSystem::mouseButtonStateChanged(MouseButton mouse_button,
                                         bool pressed) {
  if (currentlySkipping() && !in_selection_mode_) {
    setSkipMode(false);
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------

bool TextSystem::keyStateChanged(KeyCode key_code, bool pressed) {
  if (currentlySkipping() && !in_selection_mode_) {
    setSkipMode(false);
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------

vector<int> TextSystem::activeWindows() {
  vector<int> tmp;
  for (PageSet::iterator it = current_pageset_->begin();
       it != current_pageset_->end(); ++it) {
    tmp.push_back(it->first);
  }
  return tmp;
}

// -----------------------------------------------------------------------

void TextSystem::snapshot() {
  previous_page_sets_.push_back(current_pageset_->clone().release());
  expireOldPages();
}

// -----------------------------------------------------------------------

void TextSystem::newPageOnWindow(int window) {
  // Erase the current instance of this window if it exists
  PageSet::iterator it = current_pageset_->find(window);
  if (it != current_pageset_->end()) {
    current_pageset_->erase(it);
  }

  previous_page_it_ = previous_page_sets_.end();
  current_pageset_->insert(window, new TextPage(system(), window));
  expireOldPages();
}

// -----------------------------------------------------------------------

TextPage& TextSystem::currentPage() {
  // Check to see if the active window has a current page.
  PageSet::iterator it = current_pageset_->find(active_window_);
  if (it == current_pageset_->end())
    it = current_pageset_->insert(
        active_window_, new TextPage(system(), active_window_)).first;

  return *it->second;
}

// -----------------------------------------------------------------------

void TextSystem::backPage() {
  is_reading_backlog_ = true;

  if (previous_page_it_ != previous_page_sets_.begin()) {
    previous_page_it_ = boost::prior(previous_page_it_);

    // Clear all windows
    clearAllTextWindows();
    hideAllTextWindows();

    replayPageSet(*previous_page_it_, false);
  }
}

// -----------------------------------------------------------------------

void TextSystem::forwardPage() {
  is_reading_backlog_ = true;

  if (previous_page_it_ != previous_page_sets_.end()) {
    previous_page_it_ = boost::next(previous_page_it_);

    // Clear all windows
    clearAllTextWindows();
    hideAllTextWindows();

    if (previous_page_it_ != previous_page_sets_.end())
      replayPageSet(*previous_page_it_, false);
    else
      replayPageSet(*current_pageset_, false);
  }
}

// -----------------------------------------------------------------------

void TextSystem::replayPageSet(PageSet& set, bool is_current_page) {
  for (PageSet::iterator it = set.begin(); it != set.end(); ++it) {
    try {
      it->second->replay(is_current_page);
    }
    catch(rlvm::Exception& e) {
      // Currently, the text system can throw on a few unimplemented situations,
      // such as ruby across lines.

      // Ignore what would normally be an ignored command when encoutered from
      // the main loop.
    }
  }
}

// -----------------------------------------------------------------------

bool TextSystem::isReadingBacklog() const {
  return is_reading_backlog_;
}

// -----------------------------------------------------------------------

void TextSystem::stopReadingBacklog() {
  is_reading_backlog_ = false;

  // Clear all windows
  clearAllTextWindows();
  hideAllTextWindows();
  replayPageSet(*current_pageset_, true);
}

// -----------------------------------------------------------------------

void TextSystem::setAutoMode(int i) {
  auto_mode_ = (bool)i;
  auto_mode_signal_(auto_mode_);
}

// -----------------------------------------------------------------------

int TextSystem::getAutoTime(int num_chars) {
  return globals_.auto_mode_base_time + globals_.auto_mode_char_time *
      num_chars;
}

// -----------------------------------------------------------------------

void TextSystem::setKeyCursor(int new_cursor) {
  if (new_cursor == -1) {
    text_key_cursor_.reset();
  } else if (!text_key_cursor_ ||
             text_key_cursor_->cursorNumber() != new_cursor) {
    text_key_cursor_.reset(new TextKeyCursor(system(), new_cursor));
  }
}

// -----------------------------------------------------------------------

int TextSystem::cursorNumber() const {
  if (text_key_cursor_)
    return text_key_cursor_->cursorNumber();
  else
    return -1;
}

// -----------------------------------------------------------------------

void TextSystem::updateWindowsForChangeToWindowAttr() {
  // Check each text window to see if it needs updating
  for (WindowMap::iterator it = text_window_.begin();
       it != text_window_.end(); ++it) {
    if (!it->second->windowAttrMod())
      it->second->setRGBAF(windowAttr());
  }
}

// -----------------------------------------------------------------------

bool TextSystem::showWindow(int win_num) const {
  std::map<int, bool>::const_iterator it =
      window_visual_override_.find(win_num);
  if (it != window_visual_override_.end())
    return it->second;
  else
    return true;
}

// -----------------------------------------------------------------------

void TextSystem::setDefaultWindowAttr(const std::vector<int>& attr) {
  globals_.window_attr = attr;
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void TextSystem::setWindowAttrR(int i) {
  globals_.window_attr.at(0) = i;
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void TextSystem::setWindowAttrG(int i) {
  globals_.window_attr.at(1) = i;
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void TextSystem::setWindowAttrB(int i) {
  globals_.window_attr.at(2) = i;
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void TextSystem::setWindowAttrA(int i) {
  globals_.window_attr.at(3) = i;
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void TextSystem::setWindowAttrF(int i) {
  globals_.window_attr.at(4) = i;
  updateWindowsForChangeToWindowAttr();
}

// -----------------------------------------------------------------------

void TextSystem::setMousePosition(const Point& pos) {
  for (WindowMap::iterator it = text_window_.begin();
       it != text_window_.end(); ++it) {
    if (showWindow(it->first)) {
      it->second->setMousePosition(pos);
    }
  }
}

// -----------------------------------------------------------------------

bool TextSystem::handleMouseClick(RLMachine& machine, const Point& pos,
                                  bool pressed) {
  if (systemVisible()) {
    for (WindowMap::iterator it = text_window_.begin();
         it != text_window_.end(); ++it) {
      if (showWindow(it->first)) {
        if (it->second->handleMouseClick(machine, pos, pressed))
          return true;
      }
    }
  }

  return false;
}

// -----------------------------------------------------------------------

void TextSystem::reset() {
  is_reading_backlog_ = false;

  current_pageset_ = std::auto_ptr<PageSet>(new PageSet);
  previous_page_sets_.clear();
  previous_page_it_ = previous_page_sets_.end();

  window_visual_override_.clear();
  text_window_.clear();

  system_visible_ = true;
  in_pause_state_ = false;
  in_selection_mode_ = false;
  kidoku_read_ = false;
  skip_mode_ = false;
}

// -----------------------------------------------------------------------

void TextSystem::setKidokuRead(const int in) {
  bool value_changed = kidoku_read_ != in;

  kidoku_read_ = in;
  skip_mode_enabled_signal_(in);

  if (value_changed && !kidoku_read_ && skip_mode_) {
    // Auto leave skip mode when we stop reading previously read text.
    setSkipMode(false);
  }
}

// -----------------------------------------------------------------------

void TextSystem::setSkipMode(const int in) {
  bool value_changed = skip_mode_ != in;
  skip_mode_ = in;

  if (value_changed)
    skip_mode_signal_(in);
}

// -----------------------------------------------------------------------

template<class Archive>
void TextSystem::load(Archive& ar, unsigned int version) {
  int win, cursor_num;
  ar & win & cursor_num;

  setActiveWindow(win);
  setKeyCursor(cursor_num);
}

// -----------------------------------------------------------------------

template<class Archive>
void TextSystem::save(Archive& ar, unsigned int version) const {
  int win = activeWindow();
  int cursor_num = cursorNumber();
  ar & win & cursor_num;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void TextSystem::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void TextSystem::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);

// -----------------------------------------------------------------------

void parseNames(const Memory& memory, const std::string& input,
                std::string& output) {
  const char* cur = input.c_str();

  const char LOWER_BYTE_FULLWIDTH_ASTERISK = 0x96;
  const char LOWER_BYTE_FULLWIDTH_PERCENT = 0x93;

  while (*cur) {
    if (cur[0] == 0x81 && (cur[1] == LOWER_BYTE_FULLWIDTH_ASTERISK ||
                           cur[1] == LOWER_BYTE_FULLWIDTH_PERCENT)) {
      char type = cur[1];
      cur += 2;

      string strindex;
      if (readFullwidthLatinLetter(cur, strindex)) {
        // Try to read a second character. We don't care if it fails.
        readFullwidthLatinLetter(cur, strindex);
      } else {
        throw rlvm::Exception("Malformed name construct in bytecode!");
      }

      int index = Memory::ConvertLetterIndexToInt(strindex);
      if (type == LOWER_BYTE_FULLWIDTH_ASTERISK)
        output += memory.getName(index);
      else
        output += memory.getLocalName(index);
    } else {
      copyOneShiftJisCharacter(cur, output);
    }
  }
}

// -----------------------------------------------------------------------

bool TextSystem::currentlySkipping() const {
  return kidoku_read_ && skipMode();
}

// -----------------------------------------------------------------------

bool RestoreTextSystemVisibility::operator()(RLMachine& machine) {
  machine.system().text().setSystemVisible(true);
  return true;
}
