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
#include "Utilities/StringUtilities.hpp"
#include "libReallive/gameexe.h"
#include "utf8cpp/utf8.h"

using std::bind;
using std::ref;
using namespace std::placeholders;

// -----------------------------------------------------------------------
// TextPageElement
// -----------------------------------------------------------------------

class TextPageElement {
 public:
  virtual ~TextPageElement() {}
  virtual bool isTextElement() { return false; }
  virtual void replayElement(TextPage& ts, bool is_active_page) = 0;
  virtual TextPageElement* clone() const = 0;
};

inline TextPageElement* new_clone(const TextPageElement& in) {
  return in.clone();
}

// -----------------------------------------------------------------------
// TextTextPageElement
// -----------------------------------------------------------------------

class TextTextPageElement : public TextPageElement {
 public:
  TextTextPageElement();
  virtual bool isTextElement() { return true; }
  virtual void replayElement(TextPage& page, bool is_active_page);
  void append(const string& c);

  virtual TextPageElement* clone() const {
    return new TextTextPageElement(*this);
  }

 private:
  // A list of UTF-8 characters to print.
  string list_of_chars_to_print_;
};

TextTextPageElement::TextTextPageElement() {}

void TextTextPageElement::replayElement(TextPage& page, bool is_active_page) {
  // Sometimes there are empty TextTextPageElements. I hypothesize these happen
  // because of empty strings which just set the speaker's name.
  if (list_of_chars_to_print_.size()) {
    printTextToFunction(bind(&TextPage::CharacterImpl, ref(page), _1, _2),
                        list_of_chars_to_print_,
                        "");
  }
}

void TextTextPageElement::append(const string& c) {
  list_of_chars_to_print_.append(c);
}

// -----------------------------------------------------------------------
// ActionElement
// -----------------------------------------------------------------------
class ActionElement : public TextPageElement {
 public:
  ActionElement(const std::function<void(TextPage&, bool)>& action)
      : action_(action) {}

  virtual void replayElement(TextPage& page, bool is_active_page) {
    action_(page, is_active_page);
  }

  virtual TextPageElement* clone() const { return new ActionElement(*this); }

 private:
  std::function<void(TextPage&, bool)> action_;
};

// -----------------------------------------------------------------------
// TextPage
// -----------------------------------------------------------------------

TextPage::TextPage(System& system, int window_num)
    : system_(&system),
      window_num_(window_num),
      number_of_chars_on_page_(0),
      in_ruby_gloss_(false) {
  addSetToRightStartingColorElement();
}

TextPage::TextPage(const TextPage& rhs)
    : system_(rhs.system_),
      window_num_(rhs.window_num_),
      number_of_chars_on_page_(rhs.number_of_chars_on_page_),
      in_ruby_gloss_(rhs.in_ruby_gloss_) {
  elements_to_replay_.insert(elements_to_replay_.end(),
                             rhs.elements_to_replay_.begin(),
                             rhs.elements_to_replay_.end());
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
  for_each(
      elements_to_replay_.begin(),
      elements_to_replay_.end(),
      bind(&TextPageElement::replayElement, _1, ref(*this), is_active_page));
}

// ------------------------------------------------- [ Public operations ]

bool TextPage::character(const string& current, const string& rest) {
  bool rendered = CharacterImpl(current, rest);

  if (rendered) {
    if (elements_to_replay_.size() == 0 ||
        !elements_to_replay_.back().isTextElement())
      elements_to_replay_.push_back(new TextTextPageElement);

    dynamic_cast<TextTextPageElement&>(elements_to_replay_.back())
        .append(current);

    number_of_chars_on_page_++;
  }

  return rendered;
}

void TextPage::name(const string& name, const string& next_char) {
  addAction(bind(&TextPage::NameImpl, _1, name, next_char, _2));
  number_of_chars_on_page_++;
}

void TextPage::koeMarker(int id) {
  addAction(bind(&TextPage::KoeMarkerImpl, _1, id, _2));
}

void TextPage::hardBrake() {
  addAction(bind(&TextPage::HardBrakeImpl, _1, _2));
}

void TextPage::setIndentation() {
  addAction(bind(&TextPage::SetIndentationImpl, _1, _2));
}

void TextPage::resetIndentation() {
  addAction(bind(&TextPage::ResetIndentationImpl, _1, _2));
}

void TextPage::fontColour(int colour) {
  addAction(bind(&TextPage::FontColourImpl, _1, colour, _2));
}

void TextPage::defaultFontSize() {
  addAction(bind(&TextPage::DefaultFontSizeImpl, _1, _2));
}

void TextPage::fontSize(const int size) {
  addAction(bind(&TextPage::FontSizeImpl, _1, size, _2));
}

void TextPage::markRubyBegin() {
  addAction(bind(&TextPage::MarkRubyBeginImpl, _1, _2));
}

void TextPage::displayRubyText(const std::string& utf8str) {
  addAction(bind(&TextPage::DisplayRubyTextImpl, _1, utf8str, _2));
}

void TextPage::setInsertionPointX(int x) {
  addAction(bind(&TextPage::SetInsertionPointXImpl, _1, x, _2));
}

void TextPage::setInsertionPointY(int y) {
  addAction(bind(&TextPage::SetInsertionPointYImpl, _1, y, _2));
}

void TextPage::offsetInsertionPointX(int offset) {
  addAction(bind(&TextPage::OffsetInsertionPointXImpl, _1, offset, _2));
}

void TextPage::offsetInsertionPointY(int offset) {
  addAction(bind(&TextPage::OffsetInsertionPointYImpl, _1, offset, _2));
}

void TextPage::faceOpen(const std::string& filename, int index) {
  addAction(bind(&TextPage::FaceOpenImpl, _1, filename, index, _2));
}

void TextPage::faceClose(int index) {
  addAction(bind(&TextPage::FaceCloseImpl, _1, index, _2));
}

void TextPage::addSetToRightStartingColorElement() {
  elements_to_replay_.push_back(
      new ActionElement(bind(&TextPage::SetToRightStartingColourImpl, _1, _2)));
}

bool TextPage::isFull() const {
  return system_->text().textWindow(window_num_)->isFull();
}

void TextPage::addAction(const std::function<void(TextPage&, bool)>& action) {
  action(*this, true);
  elements_to_replay_.push_back(new ActionElement(action));
}

bool TextPage::CharacterImpl(const string& c, const string& rest) {
  return system_->text().textWindow(window_num_)->character(c, rest);
}

void TextPage::NameImpl(const string& name,
                        const string& next_char,
                        bool is_active_page) {
  system_->text().textWindow(window_num_)->setName(name, next_char);
}

void TextPage::KoeMarkerImpl(int id, bool is_active_page) {
  if (!is_active_page) {
    system_->text().textWindow(window_num_)->koeMarker(id);
  }
}

void TextPage::HardBrakeImpl(bool is_active_page) {
  system_->text().textWindow(window_num_)->hardBrake();
}

void TextPage::SetIndentationImpl(bool is_active_page) {
  system_->text().textWindow(window_num_)->setIndentation();
}

void TextPage::ResetIndentationImpl(bool is_active_page) {
  system_->text().textWindow(window_num_)->resetIndentation();
}

void TextPage::FontColourImpl(int colour, bool is_active_page) {
  if (is_active_page) {
    system_->text().textWindow(window_num_)->setFontColor(
        system_->gameexe()("COLOR_TABLE", colour));
  }
}

void TextPage::DefaultFontSizeImpl(bool is_active_page) {
  system_->text().textWindow(window_num_)->setFontSizeToDefault();
}

void TextPage::FontSizeImpl(int size, bool is_active_page) {
  system_->text().textWindow(window_num_)->setFontSizeInPixels(size);
}

void TextPage::MarkRubyBeginImpl(bool is_active_page) {
  system_->text().textWindow(window_num_)->markRubyBegin();
  in_ruby_gloss_ = true;
}

void TextPage::DisplayRubyTextImpl(const std::string& utf8str,
                                   bool is_active_page) {
  system_->text().textWindow(window_num_)->displayRubyText(utf8str);
  in_ruby_gloss_ = false;
}

void TextPage::SetInsertionPointXImpl(int x, bool is_active_page) {
  system_->text().textWindow(window_num_)->setInsertionPointX(x);
}

void TextPage::SetInsertionPointYImpl(int y, bool is_active_page) {
  system_->text().textWindow(window_num_)->setInsertionPointY(y);
}

void TextPage::OffsetInsertionPointXImpl(int offset, bool is_active_page) {
  system_->text().textWindow(window_num_)->offsetInsertionPointX(offset);
}

void TextPage::OffsetInsertionPointYImpl(int offset, bool is_active_page) {
  system_->text().textWindow(window_num_)->offsetInsertionPointY(offset);
}

void TextPage::FaceOpenImpl(std::string filename,
                            int index,
                            bool is_active_page) {
  system_->text().textWindow(window_num_)->faceOpen(filename, index);
}

void TextPage::FaceCloseImpl(int index, bool is_active_page) {
  system_->text().textWindow(window_num_)->faceClose(index);
}

void TextPage::SetToRightStartingColourImpl(bool is_active_page) {
  Gameexe& gexe = system_->gameexe();
  boost::shared_ptr<TextWindow> window =
      system_->text().textWindow(window_num_);
  if (!is_active_page) {
    GameexeInterpretObject colour(gexe("COLOR_TABLE", 254));
    if (colour.exists())
      window->setFontColor(colour);
  }
}
