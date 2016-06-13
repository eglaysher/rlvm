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

#include "systems/base/text_system.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "base/notification_details.h"
#include "base/notification_service.h"
#include "base/notification_source.h"
#include "libreallive/gameexe.h"
#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/text_key_cursor.h"
#include "systems/base/text_page.h"
#include "systems/base/text_window.h"
#include "utf8cpp/utf8.h"
#include "utilities/exception.h"
#include "utilities/string_utilities.h"

using std::back_inserter;
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;

const unsigned int MAX_PAGE_HISTORY = 100;

const int FULLWIDTH_NUMBER_SIGN = 0xFF03;
const int FULLWIDTH_A = 0xFF21;
const int FULLWIDTH_B = 0xFF22;
const int FULLWIDTH_ZERO = 0xFF10;
const int FULLWIDTH_NINE = 0xFF19;

// -----------------------------------------------------------------------
// TextSystemGlobals
// -----------------------------------------------------------------------
TextSystemGlobals::TextSystemGlobals()
    : auto_mode_base_time(100),
      auto_mode_char_time(100),
      message_speed(30),
      font_weight(0),
      font_shadow(1) {}

// -----------------------------------------------------------------------

TextSystemGlobals::TextSystemGlobals(Gameexe& gexe)
    : auto_mode_base_time(gexe("MESSAGE_KEY_WAIT_TIME").ToInt(1500)),
      auto_mode_char_time(gexe("INIT_MESSAGE_SPEED").ToInt(30)),
      message_speed(gexe("INIT_MESSAGE_SPEED").ToInt(30)),
      font_weight(0),
      font_shadow(1) {
  GameexeInterpretObject in_window_attr(gexe("WINDOW_ATTR"));
  if (in_window_attr.Exists())
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
      current_pageset_(),
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
  if (ctrl_use.Exists())
    ctrl_key_skip_ = ctrl_use;

  CheckAndSetBool(gexe, "WINDOW_MOVE_USE", move_use_);
  CheckAndSetBool(gexe, "WINDOW_CLEAR_USE", clear_use_);
  CheckAndSetBool(gexe, "WINDOW_READJUMP_USE", read_jump_use_);
  CheckAndSetBool(gexe, "WINDOW_AUTOMODE_USE", automode_use_);
  CheckAndSetBool(gexe, "WINDOW_MSGBK_USE", msgbk_use_);
  CheckAndSetBool(gexe, "WINDOW_MSGBKLEFT_USE", msgbkleft_use_);
  CheckAndSetBool(gexe, "WINDOW_MSGBKRIGHT_USE", msgbkright_use_);
  CheckAndSetBool(gexe, "WINDOW_EXBTN_USE", exbtn_use_);

  // Iterate over all the NAMAE keys, which is a feature that Clannad English
  // Edition uses to translate the Japanese names into English.
  for (GameexeFilteringIterator it = gexe.filtering_begin("NAMAE");
       it != gexe.filtering_end();
       ++it) {
    try {
      // Data in the Gameexe.ini file is implicitly in Shift-JIS and needs to
      // be converted to UTF-8.
      std::string key = cp932toUTF8(it->GetStringAt(0), 0);
      std::string value = cp932toUTF8(it->GetStringAt(1), 0);
      namae_mapping_[key] = value;
    }
    catch (...) {
      // Gameexe.ini file is malformed.
    }
  }
  previous_page_it_ = previous_page_sets_.end();
}

TextSystem::~TextSystem() {}

void TextSystem::ExecuteTextSystem() {
  // Check to see if the cursor is displayed
  if (ShowWindow(active_window_)) {
    WindowMap::iterator it = text_window_.find(active_window_);
    if (it != text_window_.end() && it->second->is_visible() &&
        in_pause_state_ && !IsReadingBacklog()) {
      if (!text_key_cursor_)
        SetKeyCursor(0);

      text_key_cursor_->Execute();
    }
  }

  // Let each window update any TextWindowButton s.
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    if (ShowWindow(it->first)) {
      it->second->Execute();
    }
  }
}

void TextSystem::Render(std::ostream* tree) {
  if (system_visible()) {
    if (tree) {
      *tree << "Text System:" << endl;
    }

    for (WindowMap::iterator it = text_window_.begin();
         it != text_window_.end();
         ++it) {
      if (ShowWindow(it->first)) {
        it->second->Render(tree);
      }
    }

    if (ShowWindow(active_window_)) {
      WindowMap::iterator it = text_window_.find(active_window_);

      if (it != text_window_.end() && it->second->is_visible() &&
          in_pause_state_ && !IsReadingBacklog()) {
        if (!text_key_cursor_)
          SetKeyCursor(0);

        text_key_cursor_->Render(*it->second, tree);
      }
    }
  }
}

void TextSystem::HideTextWindow(int win_number) {
  WindowMap::iterator it = text_window_.find(win_number);
  if (it != text_window_.end()) {
    it->second->set_is_visible(0);
  }
}

void TextSystem::HideAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->set_is_visible(0);
  }
}

void TextSystem::HideAllTextWindowsExcept(int i) {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    if (it->first != i) {
      it->second->set_is_visible(0);
    }
  }
}

void TextSystem::ShowTextWindow(int win_number) {
  WindowMap::iterator it = text_window_.find(win_number);
  if (it != text_window_.end()) {
    it->second->set_is_visible(1);
  }
}

void TextSystem::ShowAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->set_is_visible(1);
  }
}

void TextSystem::ClearAllTextWindows() {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    it->second->ClearWin();
  }
}

void TextSystem::SetVisualOverride(int win_number, bool show_window) {
  window_visual_override_[win_number] = show_window;
}

void TextSystem::SetVisualOverrideAll(bool show_window) {
  for (int i = 0; i < 64; ++i) {
    window_visual_override_[i] = show_window;
  }
}

void TextSystem::ClearVisualOverrides() { window_visual_override_.clear(); }

std::shared_ptr<TextWindow> TextSystem::GetCurrentWindow() {
  return GetTextWindow(active_window_);
}

void TextSystem::CheckAndSetBool(Gameexe& gexe,
                                 const std::string& key,
                                 bool& out) {
  GameexeInterpretObject key_obj(gexe(key));
  if (key_obj.Exists())
    out = key_obj.ToInt();
}

void TextSystem::ExpireOldPages() {
  while (previous_page_sets_.size() > MAX_PAGE_HISTORY)
    previous_page_sets_.pop_front();
}

bool TextSystem::MouseButtonStateChanged(MouseButton mouse_button,
                                         bool pressed) {
  if (CurrentlySkipping() && !in_selection_mode_) {
    SetSkipMode(false);
    return true;
  }

  return false;
}

bool TextSystem::KeyStateChanged(KeyCode key_code, bool pressed) {
  if (CurrentlySkipping() && !in_selection_mode_) {
    SetSkipMode(false);
    return true;
  }

  return false;
}

vector<int> TextSystem::GetActiveWindows() {
  vector<int> tmp;
  for (PageSet::iterator it = current_pageset_.begin();
       it != current_pageset_.end();
       ++it) {
    tmp.push_back(it->first);
  }
  return tmp;
}

void TextSystem::Snapshot() {
  bool all_empty = std::all_of(
      current_pageset_.begin(),
      current_pageset_.end(),
      [&](std::pair<const int, TextPage>& rhs) { return rhs.second.empty(); });

  if (!all_empty) {
    previous_page_sets_.push_back(current_pageset_);
    ExpireOldPages();
  }
}

void TextSystem::NewPageOnWindow(int window) {
  // Erase the current instance of this window if it exists
  PageSet::iterator it = current_pageset_.find(window);
  if (it != current_pageset_.end()) {
    current_pageset_.erase(it);
  }

  previous_page_it_ = previous_page_sets_.end();
  current_pageset_.emplace(window, TextPage(system(), window));
  ExpireOldPages();
}

TextPage& TextSystem::GetCurrentPage() {
  // Check to see if the active window has a current page.
  PageSet::iterator it = current_pageset_.find(active_window_);
  if (it == current_pageset_.end())
    it = current_pageset_.emplace(active_window_,
                                  TextPage(system(), active_window_)).first;

  return it->second;
}

void TextSystem::BackPage() {
  is_reading_backlog_ = true;

  if (previous_page_it_ != previous_page_sets_.begin()) {
    previous_page_it_ = std::prev(previous_page_it_);

    // Clear all windows
    ClearAllTextWindows();
    HideAllTextWindows();

    ReplayPageSet(*previous_page_it_, false);
  }
}

void TextSystem::ForwardPage() {
  is_reading_backlog_ = true;

  if (previous_page_it_ != previous_page_sets_.end()) {
    previous_page_it_ = std::next(previous_page_it_);

    // Clear all windows
    ClearAllTextWindows();
    HideAllTextWindows();

    if (previous_page_it_ != previous_page_sets_.end())
      ReplayPageSet(*previous_page_it_, false);
    else
      ReplayPageSet(current_pageset_, false);
  }
}

void TextSystem::ReplayPageSet(PageSet& set, bool is_current_page) {
  for (PageSet::iterator it = set.begin(); it != set.end(); ++it) {
    try {
      it->second.Replay(is_current_page);
    }
    catch (rlvm::Exception& e) {
      // Currently, the text system can throw on a few unimplemented situations,
      // such as ruby across lines.

      // Ignore what would normally be an ignored command when encountered from
      // the main loop.
    }
  }
}

bool TextSystem::IsReadingBacklog() const { return is_reading_backlog_; }

void TextSystem::StopReadingBacklog() {
  is_reading_backlog_ = false;

  // Clear all windows
  ClearAllTextWindows();
  HideAllTextWindows();
  ReplayPageSet(current_pageset_, true);
}

std::string TextSystem::InterpretName(const std::string& utf8name) {
  auto it = namae_mapping_.find(utf8name);
  if (it == namae_mapping_.end())
    return utf8name;
  return it->second;
}

void TextSystem::SetAutoMode(int i) {
  auto_mode_ = (bool)i;

  NotificationService::current()->Notify(
      NotificationType::AUTO_MODE_STATE_CHANGED,
      Source<TextSystem>(this),
      Details<const int>(&i));
}

int TextSystem::GetAutoTime(int num_chars) {
  return globals_.auto_mode_base_time +
         globals_.auto_mode_char_time * num_chars;
}

void TextSystem::SetKeyCursor(int new_cursor) {
  if (new_cursor == -1) {
    text_key_cursor_.reset();
  } else if (!text_key_cursor_ ||
             text_key_cursor_->cursor_number() != new_cursor) {
    text_key_cursor_.reset(new TextKeyCursor(system(), new_cursor));
  }
}

int TextSystem::GetCursorNumber() const {
  if (text_key_cursor_)
    return text_key_cursor_->cursor_number();
  else
    return -1;
}

void TextSystem::UpdateWindowsForChangeToWindowAttr() {
  // Check each text window to see if it needs updating
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    if (!it->second->windowAttrMod())
      it->second->SetRGBAF(window_attr());
  }
}

bool TextSystem::ShowWindow(int win_num) const {
  std::map<int, bool>::const_iterator it =
      window_visual_override_.find(win_num);
  if (it != window_visual_override_.end())
    return it->second;
  else
    return true;
}

void TextSystem::SetDefaultWindowAttr(const std::vector<int>& attr) {
  globals_.window_attr = attr;
  UpdateWindowsForChangeToWindowAttr();
}

void TextSystem::SetWindowAttrR(int i) {
  globals_.window_attr.at(0) = i;
  UpdateWindowsForChangeToWindowAttr();
}

void TextSystem::SetWindowAttrG(int i) {
  globals_.window_attr.at(1) = i;
  UpdateWindowsForChangeToWindowAttr();
}

void TextSystem::SetWindowAttrB(int i) {
  globals_.window_attr.at(2) = i;
  UpdateWindowsForChangeToWindowAttr();
}

void TextSystem::SetWindowAttrA(int i) {
  globals_.window_attr.at(3) = i;
  UpdateWindowsForChangeToWindowAttr();
}

void TextSystem::SetWindowAttrF(int i) {
  globals_.window_attr.at(4) = i;
  UpdateWindowsForChangeToWindowAttr();
}

void TextSystem::SetMousePosition(const Point& pos) {
  for (WindowMap::iterator it = text_window_.begin(); it != text_window_.end();
       ++it) {
    if (ShowWindow(it->first)) {
      it->second->SetMousePosition(pos);
    }
  }
}

bool TextSystem::HandleMouseClick(RLMachine& machine,
                                  const Point& pos,
                                  bool pressed) {
  if (system_visible()) {
    for (WindowMap::iterator it = text_window_.begin();
         it != text_window_.end();
         ++it) {
      if (ShowWindow(it->first)) {
        if (it->second->HandleMouseClick(machine, pos, pressed))
          return true;
      }
    }
  }

  return false;
}

void TextSystem::TakeSavepointSnapshot() {
  savepoint_active_window_ = active_window();
  savepoint_cursor_number_ = GetCursorNumber();
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

  out_value = std::stoi(std::string(str_begin, begin));
  return true;
}

std::shared_ptr<Surface> TextSystem::RenderText(const std::string& utf8str,
                                                  int size,
                                                  int xspace,
                                                  int yspace,
                                                  const RGBColour& colour,
                                                  RGBColour* shadow_colour,
                                                  int max_chars_in_line) {
  const int line_max_width =
      (max_chars_in_line > 0) ? (size + xspace) * max_chars_in_line : INT_MAX;

  // On the first pass, we figure out how large of a surface we need for
  // rendering the text.
  int current_size = size;
  int total_height = 0;
  int max_width = 0;
  int current_line_width = 0;
  int current_line_height = 0;
  bool should_break = false;
  std::string::const_iterator it = utf8str.begin();
  std::string::const_iterator strend = utf8str.end();
  while (it != strend) {
    int codepoint = utf8::next(it, strend);
    bool add_char = true;
    bool is_emoji = false;

    if (codepoint == '#') {
      add_char = false;
      codepoint = utf8::next(it, strend);
      switch (codepoint) {
        case 'D':
        case 'd': {
          should_break = true;
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
    } else if (codepoint == FULLWIDTH_NUMBER_SIGN) {
      // The codepoint is a fullwidth '#'. If the codepoint after this is a
      // fullwidth 'A' or 'B', followed by two fullwidth digits, we have an
      // emoji code, and should treat it like a fullwidth space during the
      // allocation phase.
      std::string::const_iterator n = it;
      int next_codepoint = utf8::next(n, strend);
      if (next_codepoint == FULLWIDTH_A || next_codepoint == FULLWIDTH_B) {
        int num_one = utf8::next(n, strend);
        int num_two = utf8::next(n, strend);
        if (num_one >= FULLWIDTH_ZERO && num_one <= FULLWIDTH_NINE &&
            num_two >= FULLWIDTH_ZERO && num_two <= FULLWIDTH_NINE) {
          // This is an emoji mark. We should consume all input so far.
          it = n;
          is_emoji = true;
        }
      }
    }

    int added_width = 0;
    if (add_char)
      added_width = GetCharWidth(current_size, codepoint) + xspace;
    else if (is_emoji)
      added_width = size + xspace;

    if (added_width) {
      if (should_break || current_line_width + added_width > line_max_width) {
        total_height += current_line_height + yspace;
        current_line_height = current_size;
        current_line_width = added_width;
        should_break = false;
      } else {
        current_line_width += added_width;
        current_line_height = std::max(current_line_height, current_size);
      }

      max_width = std::max(max_width, current_line_width);
    }
  }
  total_height += current_line_height;
  should_break = false;

  // If this text has a shadow, our surface needs to have a final two pixels
  // added to the bottom and right to accommodate it.
  if (shadow_colour) {
    total_height += 2;
    max_width += 2;
  }

  // If we have an empty width or height, make sure we create a non-empty
  // surface to render onto.
  if (max_width == 0)
    max_width = 1;
  if (total_height == 0)
    total_height = 1;

  // TODO(erg): Surely there's a way to allocate with something other than
  // black, right?
  std::shared_ptr<Surface> surface(
      system().graphics().BuildSurface(Size(max_width, total_height)));
  surface->Fill(RGBAColour::Clear());

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
    bool is_emoji = false;
    int emoji_id = 0;
    std::shared_ptr<const Surface> emoji_surface;

    if (codepoint == '#') {
      add_char = false;
      codepoint = utf8::next(cur_end, strend);
      switch (codepoint) {
        case 'D':
        case 'd': {
          should_break = true;
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
    } else if (codepoint == FULLWIDTH_NUMBER_SIGN) {
      // The codepoint is a fullwidth '#'. If the codepoint after this is a
      // fullwidth 'A' or 'B', followed by two fullwidth digits, we have an
      // emoji code, and should treat it like a fullwidth space during the
      // allocation phase.
      std::string::const_iterator n = cur_end;
      int next_codepoint = utf8::next(n, strend);
      if (next_codepoint == FULLWIDTH_A || next_codepoint == FULLWIDTH_B) {
        int num_one = utf8::next(n, strend);
        int num_two = utf8::next(n, strend);
        if (num_one >= FULLWIDTH_ZERO && num_one <= FULLWIDTH_NINE &&
            num_two >= FULLWIDTH_ZERO && num_two <= FULLWIDTH_NINE) {
          // This is an emoji mark. We should consume all input so far.
          cur_end = n;

          // Get the emoji number from the text.
          num_one -= FULLWIDTH_ZERO;
          num_two -= FULLWIDTH_ZERO;
          emoji_id = num_one * 10 + num_two;

          // Look up what g00 surface we should use for emoji.
          emoji_surface = system().graphics().GetEmojiSurface();
          is_emoji = true;
          add_char = false;
        }
      }
    }

    int item_width = 0;
    if (add_char) {
      // If we add this character, will we horizontally overflow?
      item_width = GetCharWidth(current_size, codepoint);
    } else if (is_emoji) {
      // Whatever the real size is, we only allocate the incoming size. This
      // means that if the emoji is larger than |size|, we'll draw text over
      // it. This is to be bug for bug compatible with RealLive.
      item_width = size;
    }

    if (item_width) {
      if (should_break || currentX + item_width > max_width) {
        currentX = 0;
        currentY += current_line_height + yspace;
        current_line_height = 0;
        should_break = false;
      }
    }

    if (add_char) {
      Size s = RenderGlyphOnto(character,
                               current_size,
                               false,
                               current_colour,
                               shadow_colour,
                               currentX,
                               currentY,
                               surface);
      currentX += s.width() + xspace;
      current_line_height = std::max(current_line_height, current_size);
    } else if (is_emoji) {
      if (emoji_surface) {
        // Emoji surfaces don't have internal pattnos. Instead, assume that
        // icons are square and laid out to the right, sort of like mouse
        // cursors.
        int height = emoji_surface->GetSize().height();
        Rect src = Rect(height * emoji_id, 0, Size(height, height));
        Rect dst = Rect(currentX, currentY, Size(height, height));
        emoji_surface->BlitToSurface(*surface, src, dst, 255, false);
      }

      currentX += size + xspace;
      current_line_height = std::max(current_line_height, current_size);
    }

    it = cur_end;
  }

  return surface;
}

void TextSystem::Reset() {
  is_reading_backlog_ = false;
  script_message_no_wait_ = false;

  current_pageset_.clear();
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

void TextSystem::SetKidokuRead(const int in) {
  bool value_changed = kidoku_read_ != in;

  kidoku_read_ = in;
  NotificationService::current()->Notify(
      NotificationType::SKIP_MODE_ENABLED_CHANGED,
      Source<TextSystem>(this),
      Details<const int>(&in));

  if (value_changed && !kidoku_read_ && skip_mode_) {
    // Auto leave skip mode when we stop reading previously read text.
    SetSkipMode(false);
  }
}

void TextSystem::SetSkipMode(int in) {
  bool value_changed = skip_mode_ != in;
  skip_mode_ = in;

  if (value_changed) {
    NotificationService::current()->Notify(
        NotificationType::SKIP_MODE_STATE_CHANGED,
        Source<TextSystem>(this),
        Details<int>(&in));
  }
}

template <class Archive>
void TextSystem::load(Archive& ar, unsigned int version) {
  int win, cursor_num;
  ar& win& cursor_num;

  set_active_window(win);
  SetKeyCursor(cursor_num);
}

template <class Archive>
void TextSystem::save(Archive& ar, unsigned int version) const {
  ar& savepoint_active_window_& savepoint_cursor_number_;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void TextSystem::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    unsigned int version) const;

template void TextSystem::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    unsigned int version);

// -----------------------------------------------------------------------

void parseNames(const Memory& memory,
                const std::string& input,
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
      if (ReadFullwidthLatinLetter(cur, strindex)) {
        // Try to read a second character. We don't care if it fails.
        ReadFullwidthLatinLetter(cur, strindex);
      } else {
        throw rlvm::Exception("Malformed name construct in bytecode!");
      }

      int index = Memory::ConvertLetterIndexToInt(strindex);
      if (type == LOWER_BYTE_FULLWIDTH_ASTERISK)
        output += memory.GetName(index);
      else
        output += memory.GetLocalName(index);
    } else {
      CopyOneShiftJisCharacter(cur, output);
    }
  }
}

bool TextSystem::CurrentlySkipping() const {
  return kidoku_read_ && skip_mode();
}

bool RestoreTextSystemVisibility::operator()(RLMachine& machine) {
  machine.system().text().set_system_visible(true);
  return true;
}
