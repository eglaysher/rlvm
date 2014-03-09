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

#include "Systems/Base/TextWindow.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/SelectionElement.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWaku.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/Graphics.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/defs.h"
#include "libReallive/gameexe.h"
#include "utf8cpp/utf8.h"

using std::bind;
using std::endl;
using std::ostringstream;
using std::ref;
using std::setfill;
using std::setw;
using std::vector;

using namespace std::placeholders;

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
  boost::shared_ptr<const Surface> face_surface;
};

// -----------------------------------------------------------------------
// TextWindow
// -----------------------------------------------------------------------

TextWindow::TextWindow(System& system, int window_num)
    : window_num_(window_num),
      text_insertion_point_x_(0),
      text_insertion_point_y_(0),
      ruby_begin_point_(-1),
      current_line_number_(0),
      current_indentation_in_pixels_(0),
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
  Size size = getScreenSize(gexe);
  screen_width_ = size.width();
  screen_height_ = size.height();

  // Base form for everything to follow.
  GameexeInterpretObject window(gexe("WINDOW", window_num));

  // Handle: #WINDOW.index.ATTR_MOD, #WINDOW_ATTR, #WINDOW.index.ATTR
  window_attr_mod_ = window("ATTR_MOD");
  if (window_attr_mod_ == 0)
    setRGBAF(system.text().windowAttr());
  else
    setRGBAF(window("ATTR"));

  default_font_size_in_pixels_ = window("MOJI_SIZE").to_int(25);
  setFontSizeInPixels(default_font_size_in_pixels_);
  setWindowSizeInCharacters(window("MOJI_CNT"));
  setSpacingBetweenCharacters(window("MOJI_REP"));
  setRubyTextSize(window("LUBY_SIZE").to_int(0));
  setTextboxPadding(window("MOJI_POS"));

  setWindowPosition(window("POS"));

  setDefaultTextColor(gexe("COLOR_TABLE", 0));

  // INDENT_USE appears to default to on. See the first scene in the
  // game with Nagisa, paying attention to indentation; then check the
  // Gameexe.ini.
  setUseIndentation(window("INDENT_USE").to_int(1));

  setKeycurMod(window("KEYCUR_MOD"));
  setActionOnPause(window("R_COMMAND_MOD").to_int(0));

  // Main textbox waku
  waku_set_ = window("WAKU_SETNO").to_int(0);
  textbox_waku_.reset(TextWaku::Create(system_, *this, waku_set_, 0));

  // Name textbox if that setting has been enabled.
  setNameMod(window("NAME_MOD").to_int(0));
  if (name_mod_ == 1 && window("NAME_WAKU_SETNO").exists()) {
    name_waku_set_ = window("NAME_WAKU_SETNO");
    namebox_waku_.reset(TextWaku::Create(system_, *this, name_waku_set_, 0));
    setNameSpacingBetweenCharacters(window("NAME_MOJI_REP"));
    setNameboxPadding(window("NAME_MOJI_POS"));
    // Ignoring NAME_WAKU_MIN for now
    setNameboxPosition(window("NAME_POS"));
    name_waku_dir_set_ = window("NAME_WAKU_DIR").to_int(0);
    namebox_centering_ = window("NAME_CENTERING").to_int(0);
    minimum_namebox_size_ = window("NAME_MOJI_MIN").to_int(4);
    name_size_ = window("NAME_MOJI_SIZE");
  }

  // Load #FACE information.
  GameexeFilteringIterator it = gexe.filtering_begin(window.key() + ".FACE");
  GameexeFilteringIterator end = gexe.filtering_end();
  for (; it != end; ++it) {
    // Retrieve the face slot number
    std::vector<std::string> key_parts = it->key_parts();

    try {
      int slot = boost::lexical_cast<int>(key_parts.at(3));
      if (slot < kNumFaceSlots) {
        face_slot_[slot].reset(new FaceSlot(it->to_intVector()));
      }
    }
    catch (...) {
      // Parsing failure. Ignore this key.
    }
  }
}

TextWindow::~TextWindow() {}

void TextWindow::execute() {
  if (isVisible() && !system_.graphics().interfaceHidden()) {
    textbox_waku_->execute();
  }
}

void TextWindow::setTextboxPadding(const vector<int>& pos_data) {
  upper_box_padding_ = pos_data.at(0);
  lower_box_padding_ = pos_data.at(1);
  left_box_padding_ = pos_data.at(2);
  right_box_padding_ = pos_data.at(3);
}

void TextWindow::setName(const std::string& utf8name,
                         const std::string& next_char) {
  if (name_mod_ == 0) {
    // Display the name in one pass
    printTextToFunction(
        bind(&TextWindow::character, ref(*this), _1, _2), utf8name, next_char);
    setIndentation();
  }

  setNameWithoutDisplay(utf8name);
}

void TextWindow::setNameWithoutDisplay(const std::string& utf8name) {
  if (name_mod_ == 1) {
    namebox_characters_ = 0;
    try {
      namebox_characters_ = utf8::distance(utf8name.begin(), utf8name.end());
    }
    catch (...) {
      // If utf8name isn't a real UTF-8 string, possibly overestimate:
      namebox_characters_ = utf8name.size();
    }

    namebox_characters_ = std::max(namebox_characters_, minimum_namebox_size_);

    renderNameInBox(utf8name);
  }

  last_token_was_name_ = true;
}

void TextWindow::setDefaultTextColor(const vector<int>& colour) {
  default_colour_ = RGBColour(colour.at(0), colour.at(1), colour.at(2));
}

void TextWindow::setFontColor(const vector<int>& colour) {
  font_colour_ = RGBColour(colour.at(0), colour.at(1), colour.at(2));
}

void TextWindow::setWindowSizeInCharacters(const vector<int>& pos_data) {
  x_window_size_in_chars_ = pos_data.at(0);
  y_window_size_in_chars_ = pos_data.at(1);
}

void TextWindow::setSpacingBetweenCharacters(const vector<int>& pos_data) {
  x_spacing_ = pos_data.at(0);
  y_spacing_ = pos_data.at(1);
}

void TextWindow::setWindowPosition(const vector<int>& pos_data) {
  origin_ = pos_data.at(0);
  x_distance_from_origin_ = pos_data.at(1);
  y_distance_from_origin_ = pos_data.at(2);
}

Size TextWindow::textWindowSize() const {
  return Size(
      (x_window_size_in_chars_ * (default_font_size_in_pixels_ + x_spacing_)),
      (y_window_size_in_chars_ *
       (default_font_size_in_pixels_ + y_spacing_ + ruby_size_)));
}

Size TextWindow::textSurfaceSize() const {
  // There is one extra character in each line to accommodate squeezed
  // punctuation.
  return textWindowSize() + Size(default_font_size_in_pixels_, 0);
}

Rect TextWindow::windowRect() const {
  // This absolutely needs to know the size of the on main backing waku if we
  // want to draw things correctly! If we are going to offset this text box
  // from the top or the bottom, we MUST know what the size of the image
  // graphic is if we want accurate calculations, because some image graphics
  // are significantly larger than textWindowSize() + the paddings.
  //
  // RealLive is definitely correcting programmer errors which places textboxes
  // offscreen. For example, take P_BRAVE (please!): #WINDOW.002.POS=2:78,6,
  // and a waku that refers to PM_WIN.g00 for it's image. That image is 800x300
  // pixels. The image is still centered perfectly, even though it's supposed
  // to be shifted 78 pixels right since the origin is the bottom
  // left. Expanding this number didn't change the position offscreen.
  Size boxSize = textbox_waku_->getSize(textSurfaceSize());

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
  };

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

Rect TextWindow::textSurfaceRect() const {
  Rect window = windowRect();

  Point textOrigin =
      window.origin() + Size(left_box_padding_, upper_box_padding_);

  Size rectSize = textSurfaceSize();
  rectSize += Size(right_box_padding_, lower_box_padding_);

  return Rect(textOrigin, rectSize);
}

Rect TextWindow::nameboxWakuRect() const {
  // Like the main windowRect(), we need to ask the waku what size it wants to
  // be.
  Size boxSize = namebox_waku_->getSize(nameboxTextArea());

  // The waku is offset from the top left corner of the text window.
  Rect r = windowRect();
  return Rect(Point(r.x() + namebox_x_offset_,
                    r.y() + namebox_y_offset_ - boxSize.height()),
              boxSize);
}

Size TextWindow::nameboxTextArea() const {
  // TODO: This seems excessively wide.
  return Size(
      2 * horizontal_namebox_padding_ + namebox_characters_ * name_size_,
      vertical_namebox_padding_ + name_size_);
}

void TextWindow::setNameSpacingBetweenCharacters(
    const std::vector<int>& pos_data) {
  name_x_spacing_ = pos_data.at(0);
}

void TextWindow::setNameboxPadding(const std::vector<int>& pos_data) {
  if (pos_data.size() >= 1)
    horizontal_namebox_padding_ = pos_data.at(0);
  if (pos_data.size() >= 2)
    vertical_namebox_padding_ = pos_data.at(1);
}

void TextWindow::setNameboxPosition(const vector<int>& pos_data) {
  namebox_x_offset_ = pos_data.at(0);
  namebox_y_offset_ = pos_data.at(1);
}

void TextWindow::setKeycurMod(const vector<int>& keycur) {
  keycursor_type_ = keycur.at(0);
  keycursor_pos_ = Point(keycur.at(1), keycur.at(2));
}

Point TextWindow::keycursorPosition(const Size& cursor_size) const {
  switch (keycursor_type_) {
    case 0:
      return textSurfaceRect().lowerRight() - cursor_size;
    case 1:
      return textSurfaceRect().origin() +
             Point(text_insertion_point_x_, text_insertion_point_y_);
    case 2:
      return textSurfaceRect().origin() + keycursor_pos_;
    default:
      throw SystemError("Invalid keycursor type");
  }
}

void TextWindow::faceOpen(const std::string& filename, int index) {
  if (face_slot_[index]) {
    face_slot_[index]->face_surface =
        system_.graphics().getSurfaceNamed(filename);

    if (face_slot_[index]->hide_other_windows) {
      system_.text().hideAllTextWindowsExcept(windowNumber());
    }
  }
}

void TextWindow::faceClose(int index) {
  if (face_slot_[index]) {
    face_slot_[index]->face_surface.reset();

    if (face_slot_[index]->hide_other_windows) {
      system_.text().hideAllTextWindowsExcept(windowNumber());
    }
  }
}

void TextWindow::NextCharIsItalic() {
  next_char_italic_ = true;
}

// TODO(erg): Make this pass the #WINDOW_ATTR colour off wile rendering the
// waku_backing.
void TextWindow::render(std::ostream* tree) {
  boost::shared_ptr<Surface> text_surface = textSurface();

  if (text_surface && isVisible()) {
    Size surface_size = text_surface->size();

    // POINT
    Point box = windowRect().origin();

    if (tree) {
      *tree << "  Text Window #" << window_num_ << endl;
    }

    Point textOrigin = textSurfaceRect().origin();

    textbox_waku_->render(tree, box, surface_size);
    renderFaces(tree, 1);

    if (inSelectionMode()) {
      for_each(selections_.begin(),
               selections_.end(),
               bind(&SelectionElement::render, _1));
    } else {
      boost::shared_ptr<Surface> name_surface = nameSurface();
      if (name_surface) {
        Rect r = nameboxWakuRect();

        if (namebox_waku_) {
          // TODO(erg): The waku needs to be adjusted to be the minimum size of
          // the window in characters
          namebox_waku_->render(tree, r.origin(), nameboxTextArea());
        }

        Point insertion_point = namebox_waku_->insertionPoint(
            r,
            Size(horizontal_namebox_padding_, vertical_namebox_padding_),
            name_surface->size(),
            namebox_centering_);
        name_surface->renderToScreen(
            name_surface->rect(),
            Rect(insertion_point, name_surface->size()),
            255);

        if (tree) {
          *tree << "    Name Area: " << r << endl;
        }
      }

      renderFaces(tree, 0);
      renderKoeReplayButtons(tree);

      text_surface->renderToScreen(
          Rect(Point(0, 0), surface_size), Rect(textOrigin, surface_size), 255);

      if (tree) {
        *tree << "    Text Area: " << Rect(textOrigin, surface_size) << endl;
      }
    }
  }
}

void TextWindow::renderFaces(std::ostream* tree, int behind) {
  for (int i = 0; i < kNumFaceSlots; ++i) {
    if (face_slot_[i] && face_slot_[i]->face_surface &&
        face_slot_[i]->behind == behind) {
      const boost::shared_ptr<const Surface>& surface =
          face_slot_[i]->face_surface;

      Rect dest(windowRect().x() + face_slot_[i]->x,
                windowRect().y() + face_slot_[i]->y,
                surface->size());
      surface->renderToScreen(surface->rect(), dest, 255);

      if (tree) {
        *tree << "    Face Slot #" << i << ": " << dest << endl;
      }
    }
  }
}

void TextWindow::renderKoeReplayButtons(std::ostream* tree) {
  for (std::vector<std::pair<Point, int>>::const_iterator it =
           koe_replay_button_.begin();
       it != koe_replay_button_.end();
       ++it) {
    koe_replay_info_->icon->renderToScreen(
        Rect(Point(0, 0), koe_replay_info_->icon->size()),
        Rect(textSurfaceRect().origin() + it->first,
             koe_replay_info_->icon->size()),
        255);
  }
}

void TextWindow::clearWin() {
  text_insertion_point_x_ = 0;
  text_insertion_point_y_ = rubyTextSize();
  current_indentation_in_pixels_ = 0;
  current_line_number_ = 0;
  ruby_begin_point_ = -1;
  font_colour_ = default_colour_;
  koe_replay_button_.clear();
}

bool TextWindow::character(const std::string& current,
                           const std::string& rest) {
  // If this text page is already full, save some time and reject
  // early.
  if (isFull())
    return false;

  setVisible(true);

  if (current != "") {
    int cur_codepoint = codepoint(current);
    bool indent_after_spacing = false;

    // But if the last character was a lenticular bracket, we need to indent
    // now. See doc/notes/NamesAndIndentation.txt for more details.
    if (last_token_was_name_) {
      if (name_mod_ == 0) {
        if (isOpeningQuoteMark(cur_codepoint))
          indent_after_spacing = true;
      } else if (name_mod_ == 2) {
        if (isOpeningQuoteMark(cur_codepoint)) {
          indent_after_spacing = true;
        } else {
          // An implicit wide space is printed instead on lines that don't have
          // an opening quote mark.
          std::string wide_space;
          utf8::append(0x3000, back_inserter(wide_space));

          // Prevent infinite recursion.
          last_token_was_name_ = false;

          if (!character(wide_space, current)) {
            last_token_was_name_ = true;
            return false;
          }
        }
      }
    }

    // If the width of this glyph plus the spacing will put us over the
    // edge of the window, then line increment.
    if (mustLineBreak(cur_codepoint, rest)) {
      hardBrake();

      if (isFull())
        return false;
    }

    RGBColour shadow = RGBAColour::Black().rgb();
    text_system_.renderGlyphOnto(current,
                                 fontSizeInPixels(),
                                 next_char_italic_,
                                 font_colour_,
                                 &shadow,
                                 text_insertion_point_x_,
                                 text_insertion_point_y_,
                                 textSurface());
    next_char_italic_ = false;

    // Move the insertion point forward one character
    text_insertion_point_x_ += font_size_in_pixels_ + x_spacing_;

    if (indent_after_spacing)
      setIndentation();
  }

  // When we aren't rendering a piece of text with a ruby gloss, mark
  // the screen as dirty so that this character renders.
  if (ruby_begin_point_ == -1) {
    system_.graphics().markScreenAsDirty(GUT_TEXTSYS);
  }

  last_token_was_name_ = false;

  return true;
}

bool TextWindow::mustLineBreak(int cur_codepoint, const std::string& rest) {
  int char_width = system().text().charWidth(fontSizeInPixels(), cur_codepoint);
  bool cur_codepoint_is_kinsoku = isKinsoku(cur_codepoint);
  int normal_width =
      x_window_size_in_chars_ * (default_font_size_in_pixels_ + x_spacing_);
  int extended_width = normal_width + default_font_size_in_pixels_;

  // If this character is a kinsoku, and will squeeze onto this line, don't
  // break and don't follow any of the further rules.
  if (cur_codepoint_is_kinsoku &&
      (text_insertion_point_x_ + char_width <= extended_width)) {
    return false;
  }

  // If this character won't fit on the line normally, break.
  if (text_insertion_point_x_ + char_width + x_spacing_ > normal_width) {
    return true;
  }

  // If this character will fit on the line, but the next n characters are
  // kinsoku characters and one of them won't, then break.
  if (!cur_codepoint_is_kinsoku && rest != "") {
    int final_insertion_x = text_insertion_point_x_ + char_width + x_spacing_;

    string::const_iterator cur = rest.begin();
    string::const_iterator end = rest.end();
    while (cur != end) {
      int point = utf8::next(cur, end);
      if (isKinsoku(point)) {
        final_insertion_x += char_width + x_spacing_;

        if (final_insertion_x > extended_width) {
          return true;
        }
      } else {
        break;
      }
    }
  }

  return false;
}

bool TextWindow::isFull() const {
  return current_line_number_ >= y_window_size_in_chars_;
}

void TextWindow::koeMarker(int id) {
  if (!koe_replay_info_) {
    koe_replay_info_.reset(new KoeReplayInfo);
    Gameexe& gexe = system_.gameexe();
    GameexeInterpretObject replay_icon(gexe("KOEREPLAYICON"));

    koe_replay_info_->icon =
        system_.graphics().getSurfaceNamed(replay_icon("NAME"));
    std::vector<int> reppos = replay_icon("REPPOS");
    if (reppos.size() == 2)
      koe_replay_info_->repos = Size(reppos[0], reppos[1]);
  }

  Point p = Point(text_insertion_point_x_, text_insertion_point_y_) +
            koe_replay_info_->repos;
  koe_replay_button_.push_back(std::make_pair(p, id));
}

void TextWindow::hardBrake() {
  text_insertion_point_x_ = current_indentation_in_pixels_;
  text_insertion_point_y_ += lineHeight();
  current_line_number_++;
}

void TextWindow::setIndentation() {
  current_indentation_in_pixels_ = text_insertion_point_x_;
}

void TextWindow::resetIndentation() { current_indentation_in_pixels_ = 0; }

void TextWindow::markRubyBegin() {
  ruby_begin_point_ = text_insertion_point_x_;
}

void TextWindow::setRGBAF(const vector<int>& attr) {
  colour_ = RGBAColour(attr.at(0), attr.at(1), attr.at(2), attr.at(3));
  setFilter(attr.at(4));
}

void TextWindow::setMousePosition(const Point& pos) {
  using namespace boost;

  if (inSelectionMode()) {
    for_each(selections_.begin(),
             selections_.end(),
             bind(&SelectionElement::setMousePosition, _1, pos));
  }

  textbox_waku_->setMousePosition(pos);
}

bool TextWindow::handleMouseClick(RLMachine& machine,
                                  const Point& pos,
                                  bool pressed) {
  using namespace boost;

  if (inSelectionMode()) {
    bool found =
        find_if(selections_.begin(),
                selections_.end(),
                bind(&SelectionElement::handleMouseClick, _1, pos, pressed)) !=
        selections_.end();

    if (found)
      return true;
  }

  for (std::vector<std::pair<Point, int>>::const_iterator it =
           koe_replay_button_.begin();
       it != koe_replay_button_.end();
       ++it) {
    Rect r = Rect(textSurfaceRect().origin() + it->first,
                  koe_replay_info_->icon->size());
    if (r.contains(pos)) {
      // We only want to actually replay the voice clip once, but we want to
      // catch both clicks.
      if (pressed)
        system_.sound().koePlay(it->second);
      return true;
    }
  }

  if (isVisible() && !machine.system().graphics().interfaceHidden()) {
    return textbox_waku_->handleMouseClick(machine, pos, pressed);
  }

  return false;
}

void TextWindow::startSelectionMode() { in_selection_mode_ = true; }

void TextWindow::setSelectionCallback(const std::function<void(int)>& in) {
  selection_callback_ = in;
}

void TextWindow::endSelectionMode() {
  in_selection_mode_ = false;
  selection_callback_ = nullptr;
  selections_.clear();
  clearWin();
}

const std::function<void(int)>& TextWindow::selectionCallback() {
  return selection_callback_;
}
