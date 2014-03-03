// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "Systems/Base/TextPage.hpp"

#include <algorithm>
#include <string>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/gameexe.h"
#include "utf8cpp/utf8.h"

using std::bind;
using std::ref;
using namespace std::placeholders;

// Represents the various commands.
enum CommandType {
  TYPE_CHARACTERS,
  TYPE_NAME,
  TYPE_KOE_MARKER,
  TYPE_HARD_BREAK,
  TYPE_SET_INDENTATION,
  TYPE_RESET_INDENTATION,
  TYPE_FONT_COLOUR,
  TYPE_DEFAULT_FONT_SIZE,
  TYPE_FONT_SIZE,
  TYPE_RUBY_BEGIN,
  TYPE_RUBY_END,
  TYPE_SET_INSERTION_X,
  TYPE_SET_INSERTION_Y,
  TYPE_OFFSET_INSERTION_X,
  TYPE_OFFSET_INSERTION_Y,
  TYPE_FACE_OPEN,
  TYPE_FACE_CLOSE,
};

// Storage for each command.
struct TextPage::Command {
  Command(CommandType type);
  Command(CommandType type, int one);
  Command(CommandType type, const std::string& one);
  Command(CommandType type, const std::string& one, const std::string& two);
  Command(CommandType type, const std::string& one, int two);
  Command(const Command& rhs);
  ~Command();

  enum CommandType command;
  union {
    // TYPE_CHARACTERS
    std::string characters;

    // TYPE_NAME
    struct {
      std::string name;
      std::string next_char;
    } name;

    // TYPE_KOE_MARKER
    int koe_id;

    // TYPE_FONT_COLOUR
    int font_colour;

    // TYPE_FONT_SIZE
    int font_size;

    // TYPE_RUBY_END
    std::string ruby_text;

    // TYPE_SET_INSERTION_X
    int set_insertion_x;

    // TYPE_SET_INSERTION_Y
    int set_insertion_y;

    // TYPE_OFFSET_INSERTION_X
    int offset_insertion_x;

    // TYPE_OFFSET_INSERTION_Y
    int offset_insertion_y;

    // TYPE_FACE_OPEN
    struct {
      std::string filename;
      int index;
    } face_open;

    // TYPE_FACE_CLOSE
    int face_close;

    // For the empty types:
    // TYPE_HARD_BREAK
    // TYPE_SET_INDENTATION
    // TYPE_RESET_INDENTATION
    // TYPE_DEFAULT_FONT_SIZE
    // TYPE_RUBY_BEGIN
    int empty;
  };
};

TextPage::Command::Command(CommandType type)
    : command(type) {
  switch (type) {
    case TYPE_HARD_BREAK:
    case TYPE_SET_INDENTATION:
    case TYPE_RESET_INDENTATION:
    case TYPE_DEFAULT_FONT_SIZE:
    case TYPE_RUBY_BEGIN:
      empty = 0;
      break;
    case TYPE_CHARACTERS:
      new (&characters) std::string;
      break;
    default:
      throw rlvm::Exception("Incorrect arrity");
  };
}

TextPage::Command::Command(CommandType type, int one)
    : command(type) {
  switch (type) {
    case TYPE_KOE_MARKER:
      koe_id = one;
      break;
    case TYPE_FONT_COLOUR:
      font_colour = one;
      break;
    case TYPE_FONT_SIZE:
      font_size = one;
      break;
    case TYPE_SET_INSERTION_X:
      set_insertion_x = one;
      break;
    case TYPE_SET_INSERTION_Y:
      set_insertion_y = one;
      break;
    case TYPE_OFFSET_INSERTION_X:
      offset_insertion_x = one;
    case TYPE_OFFSET_INSERTION_Y:
      offset_insertion_y = one;
    case TYPE_FACE_CLOSE:
      face_close = one;
    default:
      throw rlvm::Exception("Incorrect arrity");
  }
}

TextPage::Command::Command(CommandType type, const std::string& one)
    : command(type) {
  switch (type) {
    case TYPE_RUBY_END:
      new (&ruby_text) std::string(one);
      break;
    default:
      throw rlvm::Exception("Incorrect arrity");
  }
}

TextPage::Command::Command(CommandType type,
                           const std::string& one,
                           const std::string& two)
    : command(type) {
  switch (type) {
    case TYPE_NAME:
      new (&name.name) std::string(one);
      new (&name.next_char) std::string(two);
      break;
    default:
      throw rlvm::Exception("Incorrect arrity");
  }
}

TextPage::Command::Command(CommandType type,
                           const std::string& one,
                           int two)
    : command(type) {
  switch (type) {
    case TYPE_FACE_OPEN:
      new (&face_open.filename) std::string(one);
      face_open.index = two;
      break;
    default:
      throw rlvm::Exception("Incorrect arrity");
  }
}

TextPage::Command::Command(const Command& rhs)
    : command(rhs.command) {
  switch (rhs.command) {
    case TYPE_HARD_BREAK:
    case TYPE_SET_INDENTATION:
    case TYPE_RESET_INDENTATION:
    case TYPE_DEFAULT_FONT_SIZE:
    case TYPE_RUBY_BEGIN:
      empty = 0;
      break;
    case TYPE_KOE_MARKER:
      koe_id = rhs.koe_id;
      break;
    case TYPE_FONT_COLOUR:
      font_colour = rhs.font_colour;
      break;
    case TYPE_FONT_SIZE:
      font_size = rhs.font_size;
      break;
    case TYPE_SET_INSERTION_X:
      set_insertion_x = rhs.set_insertion_x;
      break;
    case TYPE_SET_INSERTION_Y:
      set_insertion_y = rhs.set_insertion_y;
      break;
    case TYPE_OFFSET_INSERTION_X:
      offset_insertion_x = rhs.offset_insertion_x;
      break;
    case TYPE_OFFSET_INSERTION_Y:
      offset_insertion_y = rhs.offset_insertion_y;
      break;
    case TYPE_FACE_CLOSE:
      face_close = rhs.face_close;
      break;
    case TYPE_CHARACTERS:
      new (&characters) std::string(rhs.characters);
      break;
    case TYPE_RUBY_END:
      new (&ruby_text) std::string(rhs.ruby_text);
      break;
    case TYPE_NAME:
      new (&name.name) std::string(rhs.name.name);
      new (&name.next_char) std::string(rhs.name.next_char);
      break;
    case TYPE_FACE_OPEN:
      command = TYPE_FACE_OPEN;
      new (&face_open.filename) std::string(rhs.face_open.filename);
      face_open.index = rhs.face_open.index;
      break;
  }
}

TextPage::Command::~Command() {
  // Needed to get around a quirk of the language
  using string_type = std::string;

  switch (command) {
    case TYPE_CHARACTERS:
      characters.~string_type();
      break;
    case TYPE_RUBY_END:
      ruby_text.~string_type();
      break;
    case TYPE_NAME:
      name.name.~string_type();
      name.next_char.~string_type();
      break;
    case TYPE_FACE_OPEN:
      face_open.filename.~string_type();
      break;
    default:
      break;
  }
}

// -----------------------------------------------------------------------
// TextPage
// -----------------------------------------------------------------------

TextPage::TextPage(System& system, int window_num)
    : system_(&system),
      window_num_(window_num),
      number_of_chars_on_page_(0),
      in_ruby_gloss_(false) {
}

TextPage::TextPage(const TextPage& rhs)
    : system_(rhs.system_),
      window_num_(rhs.window_num_),
      number_of_chars_on_page_(rhs.number_of_chars_on_page_),
      in_ruby_gloss_(rhs.in_ruby_gloss_),
      elements_to_replay_(rhs.elements_to_replay_) {
}

TextPage::~TextPage() {}

TextPage& TextPage::operator=(const TextPage& rhs) {
  TextPage tmp(rhs);
  swap(tmp);
  return *this;
}

void TextPage::swap(TextPage& rhs) {
  elements_to_replay_.swap(rhs.elements_to_replay_);
  std::swap(system_, rhs.system_);
  std::swap(window_num_, rhs.window_num_);
  std::swap(number_of_chars_on_page_, rhs.number_of_chars_on_page_);
  std::swap(in_ruby_gloss_, rhs.in_ruby_gloss_);
}

void TextPage::replay(bool is_active_page) {
  // Reset the font color.
  if (!is_active_page) {
    Gameexe& gexe = system_->gameexe();
    GameexeInterpretObject colour(gexe("COLOR_TABLE", 254));
    if (colour.exists()) {
      system_->text().textWindow(window_num_)->setFontColor(colour);
    }
  }

  for_each(elements_to_replay_.begin(),
           elements_to_replay_.end(),
           [&](Command& c) { RunTextPageCommand(c, is_active_page); });
}

// ------------------------------------------------- [ Public operations ]

bool TextPage::character(const string& current, const string& rest) {
  bool rendered = CharacterImpl(current, rest);

  if (rendered) {
    if (elements_to_replay_.size() == 0 ||
        elements_to_replay_.back().command != TYPE_CHARACTERS) {
      elements_to_replay_.push_back(Command(TYPE_CHARACTERS));
    }

    elements_to_replay_.back().characters.append(current);

    number_of_chars_on_page_++;
  }

  return rendered;
}

void TextPage::name(const string& name, const string& next_char) {
  AddAction(Command(TYPE_NAME, name, next_char));
  number_of_chars_on_page_++;
}

void TextPage::koeMarker(int id) {
  AddAction(Command(TYPE_KOE_MARKER, id));
}

void TextPage::hardBrake() {
  AddAction(Command(TYPE_HARD_BREAK));
}

void TextPage::setIndentation() {
  AddAction(Command(TYPE_SET_INDENTATION));
}

void TextPage::resetIndentation() {
  AddAction(Command(TYPE_RESET_INDENTATION));
}

void TextPage::fontColour(int colour) {
  AddAction(Command(TYPE_FONT_COLOUR, colour));
}

void TextPage::defaultFontSize() {
  AddAction(Command(TYPE_DEFAULT_FONT_SIZE));
}

void TextPage::fontSize(const int size) {
  AddAction(Command(TYPE_FONT_SIZE, size));
}

void TextPage::markRubyBegin() {
  AddAction(Command(TYPE_RUBY_BEGIN));
}

void TextPage::displayRubyText(const std::string& utf8str) {
  AddAction(Command(TYPE_RUBY_END, utf8str));
}

void TextPage::setInsertionPointX(int x) {
  AddAction(Command(TYPE_SET_INSERTION_X, x));
}

void TextPage::setInsertionPointY(int y) {
  AddAction(Command(TYPE_SET_INSERTION_Y, y));
}

void TextPage::offsetInsertionPointX(int offset) {
  AddAction(Command(TYPE_OFFSET_INSERTION_X, offset));
}

void TextPage::offsetInsertionPointY(int offset) {
  AddAction(Command(TYPE_OFFSET_INSERTION_Y, offset));
}

void TextPage::faceOpen(const std::string& filename, int index) {
  AddAction(Command(TYPE_FACE_OPEN, filename, index));
}

void TextPage::faceClose(int index) {
  AddAction(Command(TYPE_FACE_CLOSE, index));
}

bool TextPage::isFull() const {
  return system_->text().textWindow(window_num_)->isFull();
}

void TextPage::AddAction(const Command& command) {
  RunTextPageCommand(command, true);
  elements_to_replay_.push_back(command);
}

bool TextPage::CharacterImpl(const string& c, const string& rest) {
  return system_->text().textWindow(window_num_)->character(c, rest);
}

void TextPage::RunTextPageCommand(const Command& command,
                                  bool is_active_page) {
  boost::shared_ptr<TextWindow> window =
      system_->text().textWindow(window_num_);

  switch (command.command) {
    case TYPE_CHARACTERS:
      if (command.characters.size()) {
        printTextToFunction(
            bind(&TextPage::CharacterImpl, ref(*this), _1, _2),
            command.characters,
            "");
      }
      break;
    case TYPE_NAME:
      window->setName(command.name.name, command.name.next_char);
      break;
    case TYPE_KOE_MARKER:
      if (!is_active_page)
        window->koeMarker(command.koe_id);
      break;
    case TYPE_HARD_BREAK:
      window->hardBrake();
      break;
    case TYPE_SET_INDENTATION:
      window->setIndentation();
      break;
    case TYPE_RESET_INDENTATION:
      window->resetIndentation();
      break;
    case TYPE_FONT_COLOUR:
      if (is_active_page) {
        window->setFontColor(
            system_->gameexe()("COLOR_TABLE", command.font_colour));
      }
      break;
    case TYPE_DEFAULT_FONT_SIZE:
      window->setFontSizeToDefault();
      break;
    case TYPE_FONT_SIZE:
      window->setFontSizeInPixels(command.font_size);
      break;
    case TYPE_RUBY_BEGIN:
      window->markRubyBegin();
      in_ruby_gloss_ = true;
      break;
    case TYPE_RUBY_END:
      window->displayRubyText(command.ruby_text);
      in_ruby_gloss_ = false;
      break;
    case TYPE_SET_INSERTION_X:
      window->setInsertionPointX(command.set_insertion_x);
      break;
    case TYPE_SET_INSERTION_Y:
      window->setInsertionPointY(command.set_insertion_y);
      break;
    case TYPE_OFFSET_INSERTION_X:
      window->offsetInsertionPointX(command.offset_insertion_x);
      break;
    case TYPE_OFFSET_INSERTION_Y:
      window->offsetInsertionPointY(command.offset_insertion_y);
      break;
    case TYPE_FACE_OPEN:
      window->faceOpen(command.face_open.filename, command.face_open.index);
      break;
    case TYPE_FACE_CLOSE:
      window->faceClose(command.face_close);
      break;
  }
}
