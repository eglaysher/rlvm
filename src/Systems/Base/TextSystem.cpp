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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "Systems/Base/TextSystem.hpp"

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "base/notification_service.h"
#include "base/notification_source.h"
#include "base/notification_details.h"
#include "MachineBase/Memory.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextKeyCursor.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/gameexe.h"
#include "utf8cpp/utf8.h"

using boost::bind;
using boost::shared_ptr;
using std::back_inserter;
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
    : auto_mode_base_time(gexe("MESSAGE_KEY_WAIT_TIME").to_int(1500)),
      auto_mode_char_time(gexe("INIT_MESSAGE_SPEED").to_int(30)),
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
      script_message_no_wait_(false),
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

TextSystem::~TextSystem() {
}

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

void TextSystem::hideTextWindow(int win_number) {
  WindowMap::iterator it = text_window_.find(win_number);
  if (it != text_window_.end()) {
    it->second->setVisible(0);
  }
}

void TextSystem::hideAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->setVisible(0);
  }
}

void TextSystem::hideAllTextWindowsExcept(int i) {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    if (it->first != i) {
      it->second->setVisible(0);
    }
  }
}

void TextSystem::showTextWindow(int win_number) {
  WindowMap::iterator it = text_window_.find(win_number);
  if (it != text_window_.end()) {
    it->second->setVisible(1);
  }
}

void TextSystem::showAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->setVisible(1);
  }
}

void TextSystem::clearAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->clearWin();
  }
}

void TextSystem::setVisualOverride(int win_number, bool show_window) {
  window_visual_override_[win_number] = show_window;
}

void TextSystem::setVisualOverrideAll(bool show_window) {
  for (int i = 0; i < 64; ++i) {
    window_visual_override_[i] = show_window;
  }
}

void TextSystem::clearVisualOverrides() {
  window_visual_override_.clear();
}

boost::shared_ptr<TextWindow> TextSystem::currentWindow() {
  return textWindow(active_window_);
}

void TextSystem::checkAndSetBool(Gameexe& gexe, const std::string& key,
                                 bool& out) {
  GameexeInterpretObject key_obj(gexe(key));
  if (key_obj.exists())
    out = key_obj.to_int();
}

void TextSystem::expireOldPages() {
  while (previous_page_sets_.size() > MAX_PAGE_HISTORY)
    previous_page_sets_.pop_front();
}

bool TextSystem::mouseButtonStateChanged(MouseButton mouse_button,
                                         bool pressed) {
  if (currentlySkipping() && !in_selection_mode_) {
    setSkipMode(false);
    return true;
  }

  return false;
}

bool TextSystem::keyStateChanged(KeyCode key_code, bool pressed) {
  if (currentlySkipping() && !in_selection_mode_) {
    setSkipMode(false);
    return true;
  }

  return false;
}

vector<int> TextSystem::activeWindows() {
  vector<int> tmp;
  for (PageSet::iterator it = current_pageset_->begin();
       it != current_pageset_->end(); ++it) {
    tmp.push_back(it->first);
  }
  return tmp;
}

void TextSystem::snapshot() {
  previous_page_sets_.push_back(current_pageset_->clone().release());
  expireOldPages();
}

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

TextPage& TextSystem::currentPage() {
  // Check to see if the active window has a current page.
  PageSet::iterator it = current_pageset_->find(active_window_);
  if (it == current_pageset_->end())
    it = current_pageset_->insert(
        active_window_, new TextPage(system(), active_window_)).first;

  return *it->second;
}

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

bool TextSystem::isReadingBacklog() const {
  return is_reading_backlog_;
}

void TextSystem::stopReadingBacklog() {
  is_reading_backlog_ = false;

  // Clear all windows
  clearAllTextWindows();
  hideAllTextWindows();
  replayPageSet(*current_pageset_, true);
}

void TextSystem::setAutoMode(int i) {
  auto_mode_ = (bool)i;

  NotificationService::current()->Notify(
      NotificationType::AUTO_MODE_STATE_CHANGED,
      Source<TextSystem>(this),
      Details<const int>(&i));
}

int TextSystem::getAutoTime(int num_chars) {
  return globals_.auto_mode_base_time + globals_.auto_mode_char_time *
      num_chars;
}

void TextSystem::setKeyCursor(int new_cursor) {
  if (new_cursor == -1) {
    text_key_cursor_.reset();
  } else if (!text_key_cursor_ ||
             text_key_cursor_->cursorNumber() != new_cursor) {
    text_key_cursor_.reset(new TextKeyCursor(system(), new_cursor));
  }
}

int TextSystem::cursorNumber() const {
  if (text_key_cursor_)
    return text_key_cursor_->cursorNumber();
  else
    return -1;
}

void TextSystem::updateWindowsForChangeToWindowAttr() {
  // Check each text window to see if it needs updating
  for (WindowMap::iterator it = text_window_.begin();
       it != text_window_.end(); ++it) {
    if (!it->second->windowAttrMod())
      it->second->setRGBAF(windowAttr());
  }
}

bool TextSystem::showWindow(int win_num) const {
  std::map<int, bool>::const_iterator it =
      window_visual_override_.find(win_num);
  if (it != window_visual_override_.end())
    return it->second;
  else
    return true;
}

void TextSystem::setDefaultWindowAttr(const std::vector<int>& attr) {
  globals_.window_attr = attr;
  updateWindowsForChangeToWindowAttr();
}

void TextSystem::setWindowAttrR(int i) {
  globals_.window_attr.at(0) = i;
  updateWindowsForChangeToWindowAttr();
}

void TextSystem::setWindowAttrG(int i) {
  globals_.window_attr.at(1) = i;
  updateWindowsForChangeToWindowAttr();
}

void TextSystem::setWindowAttrB(int i) {
  globals_.window_attr.at(2) = i;
  updateWindowsForChangeToWindowAttr();
}

void TextSystem::setWindowAttrA(int i) {
  globals_.window_attr.at(3) = i;
  updateWindowsForChangeToWindowAttr();
}

void TextSystem::setWindowAttrF(int i) {
  globals_.window_attr.at(4) = i;
  updateWindowsForChangeToWindowAttr();
}

void TextSystem::setMousePosition(const Point& pos) {
  for (WindowMap::iterator it = text_window_.begin();
       it != text_window_.end(); ++it) {
    if (showWindow(it->first)) {
      it->second->setMousePosition(pos);
    }
  }
}

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

void TextSystem::takeSavepointSnapshot() {
  savepoint_active_window_ = activeWindow();
  savepoint_cursor_number_ = cursorNumber();
}

bool parseInteger(std::string::const_iterator& begin,
                  std::string::const_iterator end,
                  int& out_value) {
  std::string::const_iterator str_begin = begin;

  if (begin != end && *begin == '-')
    begin++;
  while (begin != end && *begin >= '0' && *begin <= '9')
    begin++;

  if (str_begin == begin)
    return false;

  out_value = boost::lexical_cast<int>(std::string(str_begin, begin));
  return true;
}

boost::shared_ptr<Surface> TextSystem::renderText(
    const std::string& utf8str, int size, int xspace, int yspace,
    const RGBColour& colour, RGBColour* shadow_colour) {
  // On the first pass, we figure out how large of a surface we need for
  // rendering the text.
  int current_size = size;
  int total_height = 0;
  int max_width = 0;
  int current_line_width = 0;
  int current_line_height = 0;
  std::string::const_iterator it = utf8str.begin();
  std::string::const_iterator strend = utf8str.end();
  while (it != strend) {
    int codepoint = utf8::next(it, strend);
    bool add_char = true;

    if (codepoint == '#') {
      add_char = false;
      codepoint = utf8::next(it, strend);
      switch (codepoint) {
        // TODO(erg): While playtesting with this new parser, I noticed a '#a'
        // control code that wasn't mentioned in xclannad's source or in rldev,
        // but is used in Little Busters.
        case 'D':
        case 'd': {
          total_height += current_line_height + yspace;
          current_line_height = 0;
          break;
        }
        case 'S':
        case 's': {
          int val;
          if (parseInteger(it, strend, val)) {
            current_size = val;
          } else {
            current_size = size;
          }
          break;
        }
        case 'C':
        case 'c': {
          // Consume an integer. Or don't.
          int val;
          parseInteger(it, strend, val);
        }
        case 'X':
        case 'x':
        case 'Y':
        case 'y':
        case '#':
          break;
        default: {
          // Ooops. This isn't a control code.
          add_char = true;
        }
      }
    }

    if (add_char) {
      int w = charWidth(current_size, codepoint);
      // TODO(erg): xspace should be added on the next character add.
      current_line_width += (w + xspace);

      max_width = std::max(max_width, current_line_width);
      current_line_height = std::max(current_line_height, current_size);
    }
  }
  total_height += current_line_height;

  // If this text has a shadow, our surface needs to have a final two pixels
  // added to the bottom and right to accommodate it.
  if (shadow_colour) {
    total_height += 2;
    max_width += 2;
  }

  // TODO(erg): Surely there's a way to allocate with something other than
  // black, right?
  boost::shared_ptr<Surface> surface(
      system().graphics().buildSurface(Size(max_width, total_height)));
  surface->fill(RGBAColour::Clear());

  RGBColour current_colour = colour;
  int currentX = 0;
  int currentY = 0;
  current_size = size;
  current_line_height = 0;
  it = utf8str.begin();
  std::string::const_iterator cur_end = it;
  while (it != strend) {
    int codepoint = utf8::next(cur_end, strend);
    std::string character(it, cur_end);
    bool add_char = true;

    if (codepoint == '#') {
      add_char = false;
      codepoint = utf8::next(cur_end, strend);
      switch (codepoint) {
        case 'D':
        case 'd': {
          currentX = 0;
          currentY += current_line_height + yspace;
          current_line_height = 0;
          break;
        }
        case 'S':
        case 's': {
          int val;
          if (parseInteger(cur_end, strend, val)) {
            current_size = val;
          } else {
            current_size = size;
          }
          break;
        }
        case 'C':
        case 'c': {
          // Consume an integer. Or don't.
          int val;
          if (parseInteger(cur_end, strend, val)) {
            Gameexe& gexe = system().gameexe();
            current_colour = RGBColour(gexe("COLOR_TABLE", val));
          } else {
            current_colour = colour;
          }
        }
        case 'X':
        case 'x':
        case 'Y':
        case 'y':
        case '#':
          break;
        default: {
          // Ooops. This isn't a control code.
          add_char = true;
        }
      }
    }

    if (add_char) {
      Size s = renderGlyphOnto(character,
                               current_size,
                               current_colour,
                               shadow_colour,
                               currentX,
                               currentY,
                               surface);
      currentX += s.width() + xspace;
      current_line_height = std::max(current_line_height, current_size);
    }

    it = cur_end;
  }

  return surface;
}

void TextSystem::reset() {
  is_reading_backlog_ = false;
  script_message_no_wait_ = false;

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

void TextSystem::setKidokuRead(const int in) {
  bool value_changed = kidoku_read_ != in;

  kidoku_read_ = in;
  NotificationService::current()->Notify(
      NotificationType::SKIP_MODE_ENABLED_CHANGED,
      Source<TextSystem>(this),
      Details<const int>(&in));

  if (value_changed && !kidoku_read_ && skip_mode_) {
    // Auto leave skip mode when we stop reading previously read text.
    setSkipMode(false);
  }
}

void TextSystem::setSkipMode(int in) {
  bool value_changed = skip_mode_ != in;
  skip_mode_ = in;

  if (value_changed) {
    NotificationService::current()->Notify(
        NotificationType::SKIP_MODE_STATE_CHANGED,
        Source<TextSystem>(this),
        Details<int>(&in));
  }
}

template<class Archive>
void TextSystem::load(Archive& ar, unsigned int version) {
  int win, cursor_num;
  ar & win & cursor_num;

  setActiveWindow(win);
  setKeyCursor(cursor_num);
}

template<class Archive>
void TextSystem::save(Archive& ar, unsigned int version) const {
  ar & savepoint_active_window_ & savepoint_cursor_number_;
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

bool TextSystem::currentlySkipping() const {
  return kidoku_read_ && skipMode();
}

bool RestoreTextSystemVisibility::operator()(RLMachine& machine) {
  machine.system().text().setSystemVisible(true);
  return true;
}
