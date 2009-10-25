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

#include "libReallive/defs.h"
#include "Systems/Base/TextWindow.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/SelectionElement.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWaku.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/Graphics.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/gameexe.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "utf8cpp/utf8.h"

using boost::bind;
using boost::ref;
using boost::shared_ptr;
using std::cerr;
using std::endl;
using std::ostringstream;
using std::setfill;
using std::setw;
using std::vector;

struct TextWindow::FaceSlot {
  explicit FaceSlot(const std::vector<int>& vec)
      : x(vec.at(0)), y(vec.at(1)), behind(vec.at(2)),
        hide_other_windows(vec.at(3)),
        unknown(vec.at(4)) { }

  int x, y;

  // 0 if layered in front or window background. 1 if behind.
  int behind;

  // Speculation: This makes ALMA work correctly and doesn't appear to harm
  // P_BRIDE.
  int hide_other_windows;

  // Unknown.
  int unknown;

  // The current face loaded. NULL whenever no face is loaded.
  boost::shared_ptr<Surface> face_surface;
};


// -----------------------------------------------------------------------
// TextWindow
// -----------------------------------------------------------------------

TextWindow::TextWindow(System& system, int window_num)
    : window_num_(window_num), ruby_begin_point_(-1), current_line_number_(0),
      current_indentation_in_pixels_(0), last_token_was_name_(false),
      use_indentation_(0), colour_(),
      filter_(0), is_visible_(0), in_selection_mode_(0), next_id_(0),
      system_(system) {
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

  setFontSizeInPixels(window("MOJI_SIZE").to_int(25));
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
    } catch (...) {
      // Parsing failure. Ignore this key.
    }
  }
}

// -----------------------------------------------------------------------

TextWindow::~TextWindow() {}

// -----------------------------------------------------------------------

void TextWindow::execute() {
  if (isVisible() && !system_.graphics().interfaceHidden()) {
    textbox_waku_->execute();
  }
}

// -----------------------------------------------------------------------

void TextWindow::setTextboxPadding(const vector<int>& pos_data) {
  upper_box_padding_ = pos_data.at(0);
  lower_box_padding_ = pos_data.at(1);
  left_box_padding_ = pos_data.at(2);
  right_box_padding_ = pos_data.at(3);
}

// -----------------------------------------------------------------------

void TextWindow::setName(const std::string& utf8name,
                         const std::string& next_char) {
  if (name_mod_ == 0) {
    // Display the name in one pass
    printTextToFunction(bind(&TextWindow::displayChar, ref(*this), _1, _2),
                        utf8name, next_char);
    setIndentation();
  }

  setNameWithoutDisplay(utf8name);
}

// -----------------------------------------------------------------------

void TextWindow::setNameWithoutDisplay(const std::string& utf8name) {
  if (name_mod_ == 1) {
    namebox_characters_ = 0;
    try {
      namebox_characters_ = utf8::distance(utf8name.begin(), utf8name.end());
    } catch(...) {
      // If utf8name isn't a real UTF-8 string, possibly overestimate:
      namebox_characters_ = utf8name.size();
    }

    namebox_characters_ = std::max(namebox_characters_, minimum_namebox_size_);

    renderNameInBox(utf8name);
  }

  last_token_was_name_ = true;
}

// -----------------------------------------------------------------------

void TextWindow::renderNameInBox(const std::string& utf8str) {
  RGBColour shadow = RGBAColour::Black().rgb();
  name_surface_ = system_.text().renderText(
      utf8str, fontSizeInPixels(), 0, 0, font_colour_, &shadow);
}

// -----------------------------------------------------------------------

void TextWindow::setDefaultTextColor(const vector<int>& colour) {
  default_colour_ = RGBColour(colour.at(0), colour.at(1), colour.at(2));
}

// -----------------------------------------------------------------------

void TextWindow::setFontColor(const vector<int>& colour) {
  font_colour_ = RGBColour(colour.at(0), colour.at(1), colour.at(2));
}

// -----------------------------------------------------------------------

void TextWindow::setWindowSizeInCharacters(const vector<int>& pos_data) {
  x_window_size_in_chars_ = pos_data.at(0);
  y_window_size_in_chars_ = pos_data.at(1);
}

// -----------------------------------------------------------------------

void TextWindow::setSpacingBetweenCharacters(const vector<int>& pos_data) {
  x_spacing_ = pos_data.at(0);
  y_spacing_ = pos_data.at(1);
}

// -----------------------------------------------------------------------

void TextWindow::setWindowPosition(const vector<int>& pos_data) {
  origin_ = pos_data.at(0);
  x_distance_from_origin_ = pos_data.at(1);
  y_distance_from_origin_ = pos_data.at(2);
}

// -----------------------------------------------------------------------

Size TextWindow::textWindowSize() const {
  return Size((x_window_size_in_chars_ *
               (font_size_in_pixels_ + x_spacing_)),
              (y_window_size_in_chars_ *
               (font_size_in_pixels_ + y_spacing_ + ruby_size_)));
}

// -----------------------------------------------------------------------

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
  Size boxSize;
  if (!textbox_waku_->getSize(boxSize)) {
    // This is an estimate; it was what I was using before and worked fine for
    // all the KEY games I originally targeted, but broke on ALMA.
    //
    // This should work as long as the background image is fully on screen or
    // doesn't exist.
    //
    // TODO(erg): This looks wrong maybe. Shouldn't I be adding the paddings to
    // both sides or deleting from all sides? (Or just not add it here in the
    // first place?
    boxSize = textWindowSize() + Size(left_box_padding_ - right_box_padding_,
                                      upper_box_padding_ - lower_box_padding_);
  }

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

// -----------------------------------------------------------------------

Rect TextWindow::textRect() const {
  Rect window = windowRect();

  Point textOrigin = window.origin() +
                     Size(left_box_padding_, upper_box_padding_);

  Size rectSize = textWindowSize();
  rectSize += Size(right_box_padding_, lower_box_padding_);

  return Rect(textOrigin, rectSize);
}

// -----------------------------------------------------------------------

int TextWindow::nameboxX1() const {
  return windowRect().origin().x() + namebox_x_offset_;
}

// -----------------------------------------------------------------------

int TextWindow::nameboxY1() const {
  // We cheat with the size calculation here.
  return windowRect().origin().y() + namebox_y_offset_ -
      (2 * vertical_namebox_padding_ + name_size_);
}

// -----------------------------------------------------------------------

// THIS IS A HACK! THIS IS SUCH AN UGLY HACK. ALL OF THE NAMEBOX POSITIONING
// CODE SIMPLY NEEDS TO BE REDONE.
Size TextWindow::nameboxSize() {
  return Size(2 * horizontal_namebox_padding_ +
              namebox_characters_ * name_size_,
              2 * vertical_namebox_padding_ + name_size_);
}

// -----------------------------------------------------------------------

void TextWindow::setNameSpacingBetweenCharacters(
    const std::vector<int>& pos_data) {
  name_x_spacing_ = pos_data.at(0);
}

// -----------------------------------------------------------------------

void TextWindow::setNameboxPadding(const std::vector<int>& pos_data) {
  horizontal_namebox_padding_ = pos_data.at(0);
  vertical_namebox_padding_ = pos_data.at(1);
}

// -----------------------------------------------------------------------

void TextWindow::setNameboxPosition(const vector<int>& pos_data) {
  namebox_x_offset_ = pos_data.at(0);
  namebox_y_offset_ = pos_data.at(1);
}

// -----------------------------------------------------------------------

void TextWindow::setKeycurMod(const vector<int>& keycur) {
  keycursor_type_ = keycur.at(0);
  keycursor_pos_ = Point(keycur.at(1), keycur.at(2));
}

// -----------------------------------------------------------------------

Point TextWindow::keycursorPosition() const {
  // TODO(erg): This is the next thing to look at after I have the rects layed
  // out. The keycursor in ALMA isn't positioned correctly.
  switch (keycursor_type_) {
  case 0:
    return textRect().lowerRight();
  case 1:
    return Point(text_insertion_point_x_, text_insertion_point_y_);
  case 2:
    return textRect().origin() + keycursor_pos_;
  default:
    throw SystemError("Invalid keycursor type");
  }
}

// -----------------------------------------------------------------------

void TextWindow::faceOpen(const std::string& filename, int index) {
  if (face_slot_[index]) {
    face_slot_[index]->face_surface =
        system_.graphics().loadNonCGSurfaceFromFile(filename);

    if (face_slot_[index]->hide_other_windows) {
      system_.text().hideAllTextWindowsExcept(windowNumber());
    }
  }
}

// -----------------------------------------------------------------------

void TextWindow::faceClose(int index) {
  if (face_slot_[index]) {
    face_slot_[index]->face_surface.reset();

    if (face_slot_[index]->hide_other_windows) {
      system_.text().hideAllTextWindowsExcept(windowNumber());
    }
  }
}

// -----------------------------------------------------------------------

/**
 * @todo Make this pass the \#WINDOW_ATTR colour off wile rendering the
 *       waku_backing.
 */
void TextWindow::render(std::ostream* tree) {
  if (text_surface_ && isVisible()) {
    Size surface_size = text_surface_->size();

    // POINT
    Point box = windowRect().origin();

    if (tree) {
      *tree << "  Text Window #" << window_num_ << endl;
    }

    textbox_waku_->render(tree, box, surface_size);
    renderFaces(tree, 1);

    Point textOrigin = textRect().origin();

    if (inSelectionMode()) {
      for_each(selections_.begin(), selections_.end(),
               bind(&SelectionElement::render, _1));
    } else {
      if (name_surface_) {
        Point namebox_location(nameboxX1(), nameboxY1());
        Size namebox_size = nameboxSize();

        if (namebox_waku_) {
          // TODO(erg): The waku needs to be adjusted to be the minimum size of
          // the window in characters
          namebox_waku_->render(tree, namebox_location, namebox_size);
        }

        Point insertion_point =
            namebox_location +
            Point((namebox_size.width() / 2) -
                  (name_surface_->size().width() / 2),
                  (namebox_size.height() / 2) -
                  (name_surface_->size().height() / 2));
        name_surface_->renderToScreen(
            name_surface_->rect(),
            Rect(insertion_point, name_surface_->size()),
            255);

        if (tree) {
          *tree << "     Name Area: " << Rect(namebox_location, namebox_size)
                << endl;
        }
      }

      renderFaces(tree, 0);

      text_surface_->renderToScreen(
        Rect(Point(0, 0), surface_size),
        Rect(textOrigin, surface_size),
        255);

      if (tree) {
        *tree << "    Text Area: " << Rect(textOrigin, surface_size) << endl;
      }
    }
  }
}

// -----------------------------------------------------------------------

void TextWindow::renderFaces(std::ostream* tree, int behind) {
  for (int i = 0; i < kNumFaceSlots; ++i) {
    if (face_slot_[i] &&
        face_slot_[i]->face_surface &&
        face_slot_[i]->behind == behind) {
      const boost::shared_ptr<Surface>& surface = face_slot_[i]->face_surface;
      surface->renderToScreen(
          surface->rect(),
          Rect(windowRect().x() + face_slot_[i]->x,
               windowRect().y() + face_slot_[i]->y,
               surface->size()), 255);
    }
  }
}

// -----------------------------------------------------------------------

void TextWindow::clearWin() {
  text_insertion_point_x_ = 0;
  text_insertion_point_y_ = rubyTextSize();
  current_indentation_in_pixels_ = 0;
  current_line_number_ = 0;
  ruby_begin_point_ = -1;
  font_colour_ = default_colour_;

  // Allocate the text window surface
  if (!text_surface_)
    text_surface_ = system_.graphics().buildSurface(textWindowSize());
  text_surface_->fill(RGBAColour::Clear());

  name_surface_.reset();
}

// -----------------------------------------------------------------------

bool TextWindow::displayChar(const std::string& current,
                             const std::string& next) {
  // If this text page is already full, save some time and reject
  // early.
  if (isFull())
    return false;

  setVisible(true);

  if (current != "") {
    int cur_codepoint = codepoint(current);
    int next_codepoint = codepoint(next);
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

          if (!displayChar(wide_space, current)) {
            last_token_was_name_ = true;
            return false;
          }
        }
      }
    }

    // Render glyph to surface
    RGBColour shadow = RGBAColour::Black().rgb();
    boost::shared_ptr<Surface> character = system().text().renderUTF8Glyph(
        current, fontSizeInPixels(), font_colour_, &shadow);
    if (character == NULL) {
      // Bug during Kyou's path. The string is printed "". Regression in parser?
      cerr << "WARNING. TTF_RenderUTF8_Blended didn't render the string \""
           << current << "\". Hopefully continuing..." << endl;

      return true;
    }

    // If the width of this glyph plus the spacing will put us over the
    // edge of the window, then line increment.
    //
    // If the current character will fit on this line, and it is NOT
    // in this set, then we should additionally check the next
    // character.  If that IS in this set and will not fit on the
    // current line, then we break the line before the current
    // character instead, to prevent the next character being stranded
    // at the start of a line.
    //
    int char_width = character->size().width();
    bool char_will_fit_on_line =
        text_insertion_point_x_ + char_width + x_spacing_ <=
        textWindowSize().width();
    bool next_char_will_fit_on_line =
        text_insertion_point_x_ + 2 * (char_width + x_spacing_) <=
        textWindowSize().width();
    if (!char_will_fit_on_line ||
        (char_will_fit_on_line && !isKinsoku(cur_codepoint) &&
         !next_char_will_fit_on_line && isKinsoku(next_codepoint))) {
      hardBrake();

      if (isFull())
        return false;
    }

    Size s(character->size());
    character->blitToSurface(
        *text_surface_,
        Rect(Point(0, 0), s),
        Rect(Point(text_insertion_point_x_, text_insertion_point_y_), s),
        255, false);

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

// -----------------------------------------------------------------------

bool TextWindow::isFull() const {
  return current_line_number_ >= y_window_size_in_chars_;
}

// -----------------------------------------------------------------------

void TextWindow::hardBrake() {
  text_insertion_point_x_ = current_indentation_in_pixels_;
  text_insertion_point_y_ += lineHeight();
  current_line_number_++;
}

// -----------------------------------------------------------------------

void TextWindow::setIndentation() {
  current_indentation_in_pixels_ = text_insertion_point_x_;
}

// -----------------------------------------------------------------------

void TextWindow::resetIndentation() {
  current_indentation_in_pixels_ = 0;
}

// -----------------------------------------------------------------------

void TextWindow::markRubyBegin() {
  ruby_begin_point_ = text_insertion_point_x_;
}

// -----------------------------------------------------------------------

void TextWindow::displayRubyText(const std::string& utf8str) {
  if (ruby_begin_point_ != -1) {
    int end_point = text_insertion_point_x_ - x_spacing_;

    if (ruby_begin_point_ > end_point) {
      ruby_begin_point_ = -1;
      throw rlvm::Exception("We don't handle ruby across line breaks yet!");
    }

    boost::shared_ptr<Surface> ruby = system_.text().renderText(
      utf8str, rubyTextSize(), 0, 0, font_colour_, NULL);

    // Render glyph to surface
    int height_location = text_insertion_point_y_ - rubyTextSize();
    int width_start =
      int(ruby_begin_point_ + ((end_point - ruby_begin_point_) * 0.5f) -
          (ruby->size().width() * 0.5f));
    ruby->blitToSurface(
        *text_surface_,
        ruby->rect(),
        Rect(Point(width_start, height_location), ruby->size()),
        255, false);

    system_.graphics().markScreenAsDirty(GUT_TEXTSYS);

    ruby_begin_point_ = -1;
  }

  last_token_was_name_ = false;
}

// -----------------------------------------------------------------------

void TextWindow::setRGBAF(const vector<int>& attr) {
  colour_ = RGBAColour(attr.at(0), attr.at(1), attr.at(2), attr.at(3));
  setFilter(attr.at(4));
}

// -----------------------------------------------------------------------

void TextWindow::setMousePosition(const Point& pos) {
  using namespace boost;

  if (inSelectionMode()) {
    for_each(selections_.begin(), selections_.end(),
             bind(&SelectionElement::setMousePosition, _1, pos));
  }

  textbox_waku_->setMousePosition(pos);
}

// -----------------------------------------------------------------------

bool TextWindow::handleMouseClick(RLMachine& machine, const Point& pos,
                                  bool pressed) {
  using namespace boost;

  if (inSelectionMode()) {
    bool found =
      find_if(selections_.begin(), selections_.end(),
              bind(&SelectionElement::handleMouseClick, _1, pos, pressed))
      != selections_.end();

    if (found)
      return true;
  }


  if (isVisible() && !machine.system().graphics().interfaceHidden()) {
    return textbox_waku_->handleMouseClick(machine, pos, pressed);
  }

  return false;
}

// -----------------------------------------------------------------------

void TextWindow::startSelectionMode() {
  in_selection_mode_ = true;
  next_id_ = 0;
}

// -----------------------------------------------------------------------

void TextWindow::setSelectionCallback(const boost::function<void(int)>& in) {
  selection_callback_ = in;
}

// -----------------------------------------------------------------------

void TextWindow::endSelectionMode() {
  in_selection_mode_ = false;
  selection_callback_.clear();
  selections_.clear();
  clearWin();
}

// -----------------------------------------------------------------------

const boost::function<void(int)>& TextWindow::selectionCallback() {
  return selection_callback_;
}
