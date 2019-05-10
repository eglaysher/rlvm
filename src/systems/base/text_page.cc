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

#include "systems/base/text_page.h"

#include <algorithm>
#include <string>

#include "libreallive/gameexe.h"
#include "machine/rlmachine.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"
#include "systems/base/text_window.h"
#include "utf8cpp/utf8.h"
#include "utilities/exception.h"
#include "utilities/string_utilities.h"

using std::bind;
using std::ref;
using std::placeholders::_1;
using std::placeholders::_2;

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
  TYPE_NEXT_CHAR_IS_ITALIC,
};

// Storage for each command.
struct TextPage::Command {
  explicit Command(CommandType type);
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
    case TYPE_NEXT_CHAR_IS_ITALIC:
      empty = 0;
      break;
    case TYPE_CHARACTERS:
      new (&characters) std::string;
      break;
    default:
      throw rlvm::Exception("Incorrect arity");
  }
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
      break;
    case TYPE_OFFSET_INSERTION_Y:
      offset_insertion_y = one;
      break;
    case TYPE_FACE_CLOSE:
      face_close = one;
      break;
    default:
      throw rlvm::Exception("Incorrect arity");
  }
}

TextPage::Command::Command(CommandType type, const std::string& one)
    : command(type) {
  switch (type) {
    case TYPE_RUBY_END:
      new (&ruby_text) std::string(one);
      break;
    default:
      throw rlvm::Exception("Incorrect arity");
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
      throw rlvm::Exception("Incorrect arity");
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
      throw rlvm::Exception("Incorrect arity");
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
    case TYPE_NEXT_CHAR_IS_ITALIC:
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

TextPage::TextPage(const TextPage& rhs) = default;

TextPage::TextPage(TextPage&& rhs) = default;

TextPage::~TextPage() {}

void TextPage::Replay(bool is_active_page) {
  // Reset the font color.
  if (!is_active_page) {
    Gameexe& gexe = system_->gameexe();
    GameexeInterpretObject colour(gexe("COLOR_TABLE", 254));
    if (colour.Exists()) {
      system_->text().GetTextWindow(window_num_)->SetFontColor(colour);
    }
  }

  for_each(elements_to_replay_.begin(),
           elements_to_replay_.end(),
           [&](Command& c) { RunTextPageCommand(c, is_active_page); });
}

// ------------------------------------------------- [ Public operations ]

bool TextPage::Character(const string& current, const string& rest) {
  bool rendered = CharacterImpl(current, rest);

  if (rendered) {
    if (elements_to_replay_.size() == 0 ||
        elements_to_replay_.back().command != TYPE_CHARACTERS) {
      elements_to_replay_.emplace_back(TYPE_CHARACTERS);
    }

    elements_to_replay_.back().characters.append(current);

    number_of_chars_on_page_++;
  }

  return rendered;
}

void TextPage::Name(const string& name, const string& next_char) {
  AddAction(Command(TYPE_NAME, name, next_char));
  number_of_chars_on_page_++;
}

void TextPage::KoeMarker(int id) {
  AddAction(Command(TYPE_KOE_MARKER, id));
}

void TextPage::HardBrake() {
  AddAction(Command(TYPE_HARD_BREAK));
}

void TextPage::SetIndentation() {
  AddAction(Command(TYPE_SET_INDENTATION));
}

void TextPage::ResetIndentation() {
  AddAction(Command(TYPE_RESET_INDENTATION));
}

void TextPage::FontColour(int colour) {
  AddAction(Command(TYPE_FONT_COLOUR, colour));
}

void TextPage::DefaultFontSize() {
  AddAction(Command(TYPE_DEFAULT_FONT_SIZE));
}

void TextPage::FontSize(const int size) {
  AddAction(Command(TYPE_FONT_SIZE, size));
}

void TextPage::MarkRubyBegin() {
  AddAction(Command(TYPE_RUBY_BEGIN));
}

void TextPage::DisplayRubyText(const std::string& utf8str) {
  AddAction(Command(TYPE_RUBY_END, utf8str));
}

void TextPage::SetInsertionPointX(int x) {
  AddAction(Command(TYPE_SET_INSERTION_X, x));
}

void TextPage::SetInsertionPointY(int y) {
  AddAction(Command(TYPE_SET_INSERTION_Y, y));
}

void TextPage::Offset_insertion_point_x(int offset) {
  AddAction(Command(TYPE_OFFSET_INSERTION_X, offset));
}

void TextPage::Offset_insertion_point_y(int offset) {
  AddAction(Command(TYPE_OFFSET_INSERTION_Y, offset));
}

void TextPage::FaceOpen(const std::string& filename, int index) {
  AddAction(Command(TYPE_FACE_OPEN, filename, index));
}

void TextPage::FaceClose(int index) {
  AddAction(Command(TYPE_FACE_CLOSE, index));
}

void TextPage::NextCharIsItalic() {
  AddAction(Command(TYPE_NEXT_CHAR_IS_ITALIC));
}

bool TextPage::IsFull() const {
  return system_->text().GetTextWindow(window_num_)->IsFull();
}

void TextPage::AddAction(const Command& command) {
  RunTextPageCommand(command, true);
  elements_to_replay_.push_back(command);
}

bool TextPage::CharacterImpl(const string& c, const string& rest) {
  return system_->text().GetTextWindow(window_num_)->DisplayCharacter(c, rest);
}

void TextPage::RunTextPageCommand(const Command& command,
                                  bool is_active_page) {
  std::shared_ptr<TextWindow> window =
      system_->text().GetTextWindow(window_num_);

  switch (command.command) {
    case TYPE_CHARACTERS:
      if (command.characters.size()) {
        PrintTextToFunction(
            bind(&TextPage::CharacterImpl, ref(*this), _1, _2),
            command.characters,
            "");
      }
      break;
    case TYPE_NAME:
      window->SetName(command.name.name, command.name.next_char);
      break;
    case TYPE_KOE_MARKER:
      if (!is_active_page)
        window->KoeMarker(command.koe_id);
      break;
    case TYPE_HARD_BREAK:
      window->HardBrake();
      break;
    case TYPE_SET_INDENTATION:
      window->SetIndentation();
      break;
    case TYPE_RESET_INDENTATION:
      window->ResetIndentation();
      break;
    case TYPE_FONT_COLOUR:
      if (is_active_page) {
        window->SetFontColor(
            system_->gameexe()("COLOR_TABLE", command.font_colour));
      }
      break;
    case TYPE_DEFAULT_FONT_SIZE:
      window->set_font_size_to_default();
      break;
    case TYPE_FONT_SIZE:
      window->set_font_size_in_pixels(command.font_size);
      break;
    case TYPE_RUBY_BEGIN:
      window->MarkRubyBegin();
      in_ruby_gloss_ = true;
      break;
    case TYPE_RUBY_END:
      window->DisplayRubyText(command.ruby_text);
      in_ruby_gloss_ = false;
      break;
    case TYPE_SET_INSERTION_X:
      window->set_insertion_point_x(command.set_insertion_x);
      break;
    case TYPE_SET_INSERTION_Y:
      window->set_insertion_point_y(command.set_insertion_y);
      break;
    case TYPE_OFFSET_INSERTION_X:
      window->offset_insertion_point_x(command.offset_insertion_x);
      break;
    case TYPE_OFFSET_INSERTION_Y:
      window->offset_insertion_point_y(command.offset_insertion_y);
      break;
    case TYPE_FACE_OPEN:
      window->FaceOpen(command.face_open.filename, command.face_open.index);
      break;
    case TYPE_FACE_CLOSE:
      window->FaceClose(command.face_close);
      break;
    case TYPE_NEXT_CHAR_IS_ITALIC:
      window->NextCharIsItalic();
  }
}
