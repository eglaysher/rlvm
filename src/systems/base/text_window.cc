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

#include "systems/base/text_window.h"

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "libreallive/defs.h"
#include "libreallive/gameexe.h"
#include "machine/rlmachine.h"
#include "systems/base/graphics_system.h"
#include "systems/base/selection_element.h"
#include "systems/base/sound_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/system_error.h"
#include "systems/base/text_system.h"
#include "systems/base/text_waku.h"
#include "utf8cpp/utf8.h"
#include "utilities/exception.h"
#include "utilities/graphics.h"
#include "utilities/string_utilities.h"

using std::bind;
using std::endl;
using std::ostringstream;
using std::ref;
using std::setfill;
using std::setw;
using std::unique_ptr;
using std::vector;
using std::placeholders::_1;
using std::placeholders::_2;

struct TextWindow::FaceSlot {
  explicit FaceSlot(const std::vector<int>& vec)
      : x(vec.at(0)),
        y(vec.at(1)),
        behind(vec.at(2)),
        hide_other_windows(vec.at(3)),
        unknown(vec.at(4)) {}

  int x, y;

  // 0 if layered in front or window background. 1 if behind.
  int behind;

  // Speculation: This makes ALMA work correctly and doesn't appear to harm
  // P_BRIDE.
  int hide_other_windows;

  // Unknown.
  int unknown;

  // The current face loaded. NULL whenever no face is loaded.
  std::shared_ptr<const Surface> face_surface;
};

// -----------------------------------------------------------------------
// TextWindow
// -----------------------------------------------------------------------

TextWindow::TextWindow(System& system, int window_num)
    : window_num_(window_num),
      text_insertion_point_x_(0),
      text_insertion_point_y_(0),
      text_wrapping_point_x_(0),
      ruby_begin_point_(-1),
      current_line_number_(0),
      current_indentation_in_pixels_(0),
      current_indentation_in_chars_(0),
      last_token_was_name_(false),
      use_indentation_(0),
      colour_(),
      filter_(0),
      is_visible_(0),
      in_selection_mode_(0),
      next_char_italic_(false),
      system_(system),
      text_system_(system.text()) {
  Gameexe& gexe = system.gameexe();

  // POINT
  Size size = GetScreenSize(gexe);
  screen_width_ = size.width();
  screen_height_ = size.height();

  // Base form for everything to follow.
  GameexeInterpretObject window(gexe("WINDOW", window_num));

  // Handle: #WINDOW.index.ATTR_MOD, #WINDOW_ATTR, #WINDOW.index.ATTR
  window_attr_mod_ = window("ATTR_MOD");
  if (window_attr_mod_ == 0)
    SetRGBAF(system.text().window_attr());
  else
    SetRGBAF(window("ATTR"));

  default_font_size_in_pixels_ = window("MOJI_SIZE").ToInt(25);
  set_font_size_in_pixels(default_font_size_in_pixels_);
  SetWindowSizeInCharacters(window("MOJI_CNT"));
  SetSpacingBetweenCharacters(window("MOJI_REP"));
  set_ruby_text_size(window("LUBY_SIZE").ToInt(0));
  SetTextboxPadding(window("MOJI_POS"));

  SetWindowPosition(window("POS"));

  SetDefaultTextColor(gexe("COLOR_TABLE", 0));

  // INDENT_USE appears to default to on. See the first scene in the
  // game with Nagisa, paying attention to indentation; then check the
  // Gameexe.ini.
  set_use_indentation(window("INDENT_USE").ToInt(1));

  SetKeycursorMod(window("KEYCUR_MOD"));
  set_action_on_pause(window("R_COMMAND_MOD").ToInt(0));

  // Main textbox waku
  waku_set_ = window("WAKU_SETNO").ToInt(0);
  textbox_waku_.reset(TextWaku::Create(system_, *this, waku_set_, 0));

  // Name textbox if that setting has been enabled.
  set_name_mod(window("NAME_MOD").ToInt(0));
  if (name_mod_ == 1 && window("NAME_WAKU_SETNO").Exists()) {
    name_waku_set_ = window("NAME_WAKU_SETNO");
    namebox_waku_.reset(TextWaku::Create(system_, *this, name_waku_set_, 0));
    SetNameSpacingBetweenCharacters(window("NAME_MOJI_REP"));
    SetNameboxPadding(window("NAME_MOJI_POS"));
    // Ignoring NAME_WAKU_MIN for now
    SetNameboxPosition(window("NAME_POS"));
    name_waku_dir_set_ = window("NAME_WAKU_DIR").ToInt(0);
    namebox_centering_ = window("NAME_CENTERING").ToInt(0);
    minimum_namebox_size_ = window("NAME_MOJI_MIN").ToInt(4);
    name_size_ = window("NAME_MOJI_SIZE");
  }

  // Load #FACE information.
  GameexeFilteringIterator it = gexe.filtering_begin(window.key() + ".FACE");
  GameexeFilteringIterator end = gexe.filtering_end();
  for (; it != end; ++it) {
    // Retrieve the face slot number
    std::vector<std::string> GetKeyParts = it->GetKeyParts();

    try {
      int slot = std::stoi(GetKeyParts.at(3));
      if (slot < kNumFaceSlots) {
        face_slot_[slot].reset(new FaceSlot(it->ToIntVector()));
      }
    }
    catch (...) {
      // Parsing failure. Ignore this key.
    }
  }
}

TextWindow::~TextWindow() {}

void TextWindow::Execute() {
  if (is_visible() && !system_.graphics().is_interface_hidden()) {
    textbox_waku_->Execute();
  }
}

void TextWindow::SetTextboxPadding(const vector<int>& pos_data) {
  upper_box_padding_ = pos_data.at(0);
  lower_box_padding_ = pos_data.at(1);
  left_box_padding_ = pos_data.at(2);
  right_box_padding_ = pos_data.at(3);
}

void TextWindow::SetName(const std::string& utf8name,
                         const std::string& next_char) {
  if (name_mod_ == 0) {
    std::string interpreted_name = text_system_.InterpretName(utf8name);

    // Display the name in one pass
    PrintTextToFunction(
        bind(&TextWindow::DisplayCharacter, ref(*this), _1, _2),
        interpreted_name, next_char);
    SetIndentation();
  }

  SetNameWithoutDisplay(utf8name);
}

void TextWindow::SetNameWithoutDisplay(const std::string& utf8name) {
  if (name_mod_ == 1) {
    std::string interpreted_name = text_system_.InterpretName(utf8name);

    namebox_characters_ = 0;
    try {
      namebox_characters_ = utf8::distance(interpreted_name.begin(),
                                           interpreted_name.end());
    }
    catch (...) {
      // If utf8name isn't a real UTF-8 string, possibly overestimate:
      namebox_characters_ = interpreted_name.size();
    }

    namebox_characters_ = std::max(namebox_characters_, minimum_namebox_size_);

    RenderNameInBox(interpreted_name);
  }

  last_token_was_name_ = true;
}

void TextWindow::SetDefaultTextColor(const vector<int>& colour) {
  default_colour_ = RGBColour(colour.at(0), colour.at(1), colour.at(2));
}

void TextWindow::SetFontColor(const vector<int>& colour) {
  font_colour_ = RGBColour(colour.at(0), colour.at(1), colour.at(2));
}

void TextWindow::SetWindowSizeInCharacters(const vector<int>& pos_data) {
  x_window_size_in_chars_ = pos_data.at(0);
  y_window_size_in_chars_ = pos_data.at(1);
}

void TextWindow::SetSpacingBetweenCharacters(const vector<int>& pos_data) {
  x_spacing_ = pos_data.at(0);
  y_spacing_ = pos_data.at(1);
}

void TextWindow::SetWindowPosition(const vector<int>& pos_data) {
  origin_ = pos_data.at(0);
  x_distance_from_origin_ = pos_data.at(1);
  y_distance_from_origin_ = pos_data.at(2);
}

Size TextWindow::GetTextWindowSize() const {
  return Size(
      (x_window_size_in_chars_ * (default_font_size_in_pixels_ + x_spacing_)),
      (y_window_size_in_chars_ *
       (default_font_size_in_pixels_ + y_spacing_ + ruby_size_)));
}

Size TextWindow::GetTextSurfaceSize() const {
  // There is one extra character in each line to accommodate squeezed
  // punctuation.
  return GetTextWindowSize() + Size(default_font_size_in_pixels_, 0);
}

Rect TextWindow::GetWindowRect() const {
  // This absolutely needs to know the size of the on main backing waku if we
  // want to draw things correctly! If we are going to offset this text box
  // from the top or the bottom, we MUST know what the size of the image
  // graphic is if we want accurate calculations, because some image graphics
  // are significantly larger than GetTextWindowSize() + the paddings.
  //
  // RealLive is definitely correcting programmer errors which places textboxes
  // offscreen. For example, take P_BRAVE (please!): #WINDOW.002.POS=2:78,6,
  // and a waku that refers to PM_WIN.g00 for it's image. That image is 800x300
  // pixels. The image is still centered perfectly, even though it's supposed
  // to be shifted 78 pixels right since the origin is the bottom
  // left. Expanding this number didn't change the position offscreen.
  Size boxSize = textbox_waku_->GetSize(GetTextSurfaceSize());

  int x, y;
  switch (origin_) {
    case 0:
    case 2:
      x = x_distance_from_origin_;
      break;
    case 1:
    case 3:
      x = screen_width_ - boxSize.width() - x_distance_from_origin_;
      break;
    default:
      throw SystemError("Invalid origin");
  }

  switch (origin_) {
    case 0:  // Top and left
    case 1:  // Top and right
      y = y_distance_from_origin_;
      break;
    case 2:  // Bottom and left
    case 3:  // Bottom and right
      y = screen_height_ - boxSize.height() - y_distance_from_origin_;
      break;
    default:
      throw SystemError("Invalid origin");
  }

  // Now that we have the coordinate that the programmer wanted to position the
  // box at, possibly move the box so it fits on screen.
  if ((x + boxSize.width()) > screen_width_)
    x -= (x + boxSize.width()) - screen_width_;
  if (x < 0)
    x = 0;

  if ((y + boxSize.height()) > screen_height_)
    y -= (y + boxSize.height()) - screen_height_;
  if (y < 0)
    y = 0;

  return Rect(x, y, boxSize);
}

Rect TextWindow::GetTextSurfaceRect() const {
  Rect window = GetWindowRect();

  Point textOrigin =
      window.origin() + Size(left_box_padding_, upper_box_padding_);

  Size rectSize = GetTextSurfaceSize();
  rectSize += Size(right_box_padding_, lower_box_padding_);

  return Rect(textOrigin, rectSize);
}

Rect TextWindow::GetNameboxWakuRect() const {
  // Like the main GetWindowRect(), we need to ask the waku what size it wants to
  // be.
  Size boxSize = namebox_waku_->GetSize(GetNameboxTextArea());

  // The waku is offset from the top left corner of the text window.
  Rect r = GetWindowRect();
  return Rect(Point(r.x() + namebox_x_offset_,
                    r.y() + namebox_y_offset_ - boxSize.height()),
              boxSize);
}

Size TextWindow::GetNameboxTextArea() const {
  // TODO(erg): This seems excessively wide.
  return Size(
      2 * horizontal_namebox_padding_ + namebox_characters_ * name_size_,
      vertical_namebox_padding_ + name_size_);
}

void TextWindow::SetNameSpacingBetweenCharacters(
    const std::vector<int>& pos_data) {
  name_x_spacing_ = pos_data.at(0);
}

void TextWindow::SetNameboxPadding(const std::vector<int>& pos_data) {
  if (pos_data.size() >= 1)
    horizontal_namebox_padding_ = pos_data.at(0);
  if (pos_data.size() >= 2)
    vertical_namebox_padding_ = pos_data.at(1);
}

void TextWindow::SetNameboxPosition(const vector<int>& pos_data) {
  namebox_x_offset_ = pos_data.at(0);
  namebox_y_offset_ = pos_data.at(1);
}

void TextWindow::SetKeycursorMod(const vector<int>& keycur) {
  keycursor_type_ = keycur.at(0);
  keycursor_pos_ = Point(keycur.at(1), keycur.at(2));
}

Point TextWindow::KeycursorPosition(const Size& cursor_size) const {
  switch (keycursor_type_) {
    case 0:
      return GetTextSurfaceRect().lower_right() - cursor_size;
    case 1:
      return GetTextSurfaceRect().origin() +
             Point(text_insertion_point_x_, text_insertion_point_y_);
    case 2:
      return GetTextSurfaceRect().origin() + keycursor_pos_;
    default:
      throw SystemError("Invalid keycursor type");
  }
}

void TextWindow::FaceOpen(const std::string& filename, int index) {
  if (face_slot_[index]) {
    face_slot_[index]->face_surface =
        system_.graphics().GetSurfaceNamed(filename);

    if (face_slot_[index]->hide_other_windows) {
      system_.text().HideAllTextWindowsExcept(window_number());
    }
  }
}

void TextWindow::FaceClose(int index) {
  if (face_slot_[index]) {
    face_slot_[index]->face_surface.reset();

    if (face_slot_[index]->hide_other_windows) {
      system_.text().HideAllTextWindowsExcept(window_number());
    }
  }
}

void TextWindow::NextCharIsItalic() {
  next_char_italic_ = true;
}

// TODO(erg): Make this pass the #WINDOW_ATTR colour off wile rendering the
// waku_backing.
void TextWindow::Render(std::ostream* tree) {
  std::shared_ptr<Surface> text_surface = GetTextSurface();

  if (text_surface && is_visible()) {
    Size surface_size = text_surface->GetSize();

    // POINT
    Point box = GetWindowRect().origin();

    if (tree) {
      *tree << "  Text Window #" << window_num_ << endl;
    }

    Point textOrigin = GetTextSurfaceRect().origin();

    textbox_waku_->Render(tree, box, surface_size);
    RenderFaces(tree, 1);

    if (in_selection_mode()) {
      for_each(selections_.begin(), selections_.end(),
               [](unique_ptr<SelectionElement>& e) { e->Render(); });
    } else {
      std::shared_ptr<Surface> name_surface = GetNameSurface();
      if (name_surface) {
        Rect r = GetNameboxWakuRect();

        if (namebox_waku_) {
          // TODO(erg): The waku needs to be adjusted to be the minimum size of
          // the window in characters
          namebox_waku_->Render(tree, r.origin(), GetNameboxTextArea());
        }

        Point insertion_point = namebox_waku_->InsertionPoint(
            r,
            Size(horizontal_namebox_padding_, vertical_namebox_padding_),
            name_surface->GetSize(),
            namebox_centering_);
        name_surface->RenderToScreen(
            name_surface->GetRect(),
            Rect(insertion_point, name_surface->GetSize()),
            255);

        if (tree) {
          *tree << "    Name Area: " << r << endl;
        }
      }

      RenderFaces(tree, 0);
      RenderKoeReplayButtons(tree);

      text_surface->RenderToScreen(
          Rect(Point(0, 0), surface_size), Rect(textOrigin, surface_size), 255);

      if (tree) {
        *tree << "    Text Area: " << Rect(textOrigin, surface_size) << endl;
      }
    }
  }
}

void TextWindow::RenderFaces(std::ostream* tree, int behind) {
  for (int i = 0; i < kNumFaceSlots; ++i) {
    if (face_slot_[i] && face_slot_[i]->face_surface &&
        face_slot_[i]->behind == behind) {
      const std::shared_ptr<const Surface>& surface =
          face_slot_[i]->face_surface;

      Rect dest(GetWindowRect().x() + face_slot_[i]->x,
                GetWindowRect().y() + face_slot_[i]->y,
                surface->GetSize());
      surface->RenderToScreen(surface->GetRect(), dest, 255);

      if (tree) {
        *tree << "    Face Slot #" << i << ": " << dest << endl;
      }
    }
  }
}

void TextWindow::RenderKoeReplayButtons(std::ostream* tree) {
  for (std::vector<std::pair<Point, int>>::const_iterator it =
           koe_replay_button_.begin();
       it != koe_replay_button_.end();
       ++it) {
    koe_replay_info_->icon->RenderToScreen(
        Rect(Point(0, 0), koe_replay_info_->icon->GetSize()),
        Rect(GetTextSurfaceRect().origin() + it->first,
             koe_replay_info_->icon->GetSize()),
        255);
  }
}

void TextWindow::ClearWin() {
  text_insertion_point_x_ = 0;
  text_insertion_point_y_ = ruby_text_size();
  text_wrapping_point_x_ = 0;
  current_indentation_in_pixels_ = 0;
  current_indentation_in_chars_ = 0;
  current_line_number_ = 0;
  ruby_begin_point_ = -1;
  font_colour_ = default_colour_;
  koe_replay_button_.clear();
}

bool TextWindow::DisplayCharacter(const std::string& current,
                                  const std::string& rest) {
  // If this text page is already full, save some time and reject
  // early.
  if (IsFull())
    return false;

  set_is_visible(true);

  if (current != "") {
    int cur_codepoint = Codepoint(current);
    bool indent_after_spacing = false;

    // But if the last character was a lenticular bracket, we need to indent
    // now. See doc/notes/NamesAndIndentation.txt for more details.
    if (last_token_was_name_) {
      if (name_mod_ == 0) {
        if (IsOpeningQuoteMark(cur_codepoint))
          indent_after_spacing = true;
      } else if (name_mod_ == 2) {
        if (IsOpeningQuoteMark(cur_codepoint)) {
          indent_after_spacing = true;
        }
      }
    }

    // If the width of this glyph plus the spacing will put us over the
    // edge of the window, then line increment.
    if (MustLineBreak(cur_codepoint, rest)) {
      HardBrake();

      if (IsFull())
        return false;
    }

    RGBColour shadow = RGBAColour::Black().rgb();
    text_system_.RenderGlyphOnto(current,
                                 font_size_in_pixels(),
                                 next_char_italic_,
                                 font_colour_,
                                 &shadow,
                                 text_insertion_point_x_,
                                 text_insertion_point_y_,
                                 GetTextSurface());
    next_char_italic_ = false;
    text_wrapping_point_x_ += GetWrappingWidthFor(cur_codepoint);

    if (cur_codepoint < 127) {
      // This is a basic ASCII character. In normal RealLive, western text
      // appears to be treated as half width monospace. If we're here, we are
      // either in a manually laid out western game (therefore we should try to
      // fit onto the monospace grid) or we're in rlbabel (in which case, our
      // insertion point will be manually set by the bytecode immediately after
      // this character).
      if (text_system_.FontIsMonospaced()) {
        // If our font is monospaced (ie msgothic.ttc), we want to follow the
        // game's layout instructions perfectly.
        text_insertion_point_x_ += GetWrappingWidthFor(cur_codepoint);
      } else {
        // If out font has different widths for 'i' and 'm', we aren't using
        // the recommended font so we'll try laying out the text so that
        // kerning looks better. This is the common case.
        text_insertion_point_x_ +=
            text_system_.GetCharWidth(font_size_in_pixels(), cur_codepoint);
      }
    } else {
      // Move the insertion point forward one character
      text_insertion_point_x_ += font_size_in_pixels_ + x_spacing_;
    }

    if (indent_after_spacing)
      SetIndentation();
  }

  // When we aren't rendering a piece of text with a ruby gloss, mark
  // the screen as dirty so that this character renders.
  if (ruby_begin_point_ == -1) {
    system_.graphics().MarkScreenAsDirty(GUT_TEXTSYS);
  }

  last_token_was_name_ = false;

  return true;
}

// Lines we still get wrong in CLANNAD Prologue:
//
// <rlmax> = Official RealLive's breaking
// <rlvm> = Where rlvm places the line break
//
// - "Whose ides was it to put a school at the top of a giant <rlmax> slope,<rlvm> anyway?"
//

bool TextWindow::MustLineBreak(int cur_codepoint, const std::string& rest) {
  int char_width = GetWrappingWidthFor(cur_codepoint);
  bool cur_codepoint_is_kinsoku = IsKinsoku(cur_codepoint) ||
                                  cur_codepoint == 0x20;
  int normal_width =
      x_window_size_in_chars_ * (default_font_size_in_pixels_ + x_spacing_);
  int extended_width = normal_width + default_font_size_in_pixels_;

  // If this character is a kinsoku, and will squeeze onto this line, don't
  // break and don't follow any of the further rules.
  if (cur_codepoint_is_kinsoku &&
      (text_wrapping_point_x_ + char_width <= extended_width)) {
    return false;
  }

  // If this character won't fit on the line normally, break.
  if (text_wrapping_point_x_ + char_width > normal_width) {
    return true;
  }

  // If this character will fit on the line, but the next n characters are
  // kinsoku characters OR wrapping roman characters and one of them won't,
  // then break.
  if (!cur_codepoint_is_kinsoku && rest != "") {
    int final_insertion_x = text_wrapping_point_x_ + char_width;

    string::const_iterator cur = rest.begin();
    string::const_iterator end = rest.end();
    while (cur != end) {
      int point = utf8::next(cur, end);
      if (IsKinsoku(point)) {
        final_insertion_x += GetWrappingWidthFor(point);

        if (final_insertion_x > extended_width) {
          return true;
        }
      // OK, is this correct? I'm now having places where we prematurely break.
      } else if (IsWrappingRomanCharacter(point)) {
        final_insertion_x += GetWrappingWidthFor(point);

        if (final_insertion_x > normal_width) {
          return true;
        }
      } else {
        break;
      }
    }
  }

  return false;
}

bool TextWindow::IsFull() const {
  return current_line_number_ >= y_window_size_in_chars_;
}

void TextWindow::KoeMarker(int id) {
  if (!koe_replay_info_) {
    koe_replay_info_.reset(new KoeReplayInfo);
    Gameexe& gexe = system_.gameexe();
    GameexeInterpretObject replay_icon(gexe("KOEREPLAYICON"));

    koe_replay_info_->icon =
        system_.graphics().GetSurfaceNamed(replay_icon("NAME"));
    std::vector<int> reppos = replay_icon("REPPOS");
    if (reppos.size() == 2)
      koe_replay_info_->repos = Size(reppos[0], reppos[1]);
  }

  Point p = Point(text_insertion_point_x_, text_insertion_point_y_) +
            koe_replay_info_->repos;
  koe_replay_button_.emplace_back(p, id);
}

void TextWindow::HardBrake() {
  text_insertion_point_x_ = current_indentation_in_pixels_;
  text_insertion_point_y_ += line_height();
  text_wrapping_point_x_ = current_indentation_in_chars_;
  current_line_number_++;
}

void TextWindow::SetIndentation() {
  current_indentation_in_pixels_ = text_insertion_point_x_;
  current_indentation_in_chars_ = text_wrapping_point_x_;
}

void TextWindow::ResetIndentation() {
  current_indentation_in_pixels_ = 0;
  current_indentation_in_chars_ = 0;
}

void TextWindow::MarkRubyBegin() {
  ruby_begin_point_ = text_insertion_point_x_;
}

void TextWindow::SetRGBAF(const vector<int>& attr) {
  colour_ = RGBAColour(attr.at(0), attr.at(1), attr.at(2), attr.at(3));
  set_filter(attr.at(4));
}

void TextWindow::SetMousePosition(const Point& pos) {
  if (in_selection_mode()) {
    for_each(selections_.begin(),
             selections_.end(),
             [&](unique_ptr<SelectionElement>& e) {
               e->SetMousePosition(pos); });
  }

  textbox_waku_->SetMousePosition(pos);
}

bool TextWindow::HandleMouseClick(RLMachine& machine,
                                  const Point& pos,
                                  bool pressed) {
  if (in_selection_mode()) {
    bool found =
      find_if(selections_.begin(), selections_.end(),
              [&](unique_ptr<SelectionElement> &e) {
                return e->HandleMouseClick(pos, pressed);
              }) != selections_.end();

    if (found)
      return true;
  }

  for (std::vector<std::pair<Point, int>>::const_iterator it =
           koe_replay_button_.begin();
       it != koe_replay_button_.end();
       ++it) {
    Rect r = Rect(GetTextSurfaceRect().origin() + it->first,
                  koe_replay_info_->icon->GetSize());
    if (r.Contains(pos)) {
      // We only want to actually replay the voice clip once, but we want to
      // catch both clicks.
      if (pressed)
        system_.sound().KoePlay(it->second);
      return true;
    }
  }

  if (is_visible() && !machine.system().graphics().is_interface_hidden()) {
    return textbox_waku_->HandleMouseClick(machine, pos, pressed);
  }

  return false;
}

void TextWindow::StartSelectionMode() { in_selection_mode_ = true; }

void TextWindow::SetSelectionCallback(const std::function<void(int)>& in) {
  selection_callback_ = in;
}

void TextWindow::EndSelectionMode() {
  in_selection_mode_ = false;
  selection_callback_ = nullptr;
  selections_.clear();
  ClearWin();
}

int TextWindow::GetWrappingWidthFor(int cur_codepoint) {
  if (cur_codepoint < 127) {
    return std::floor((font_size_in_pixels_ + x_spacing_) / 2.0f);
  } else {
    return font_size_in_pixels_ + x_spacing_;
  }
}
