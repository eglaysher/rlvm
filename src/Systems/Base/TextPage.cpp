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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/System.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"

#include "MachineBase/RLMachine.hpp"

#include "utf8.h"
#include "Modules/TextoutLongOperation.hpp"

#include "libReallive/gameexe.h"

#include <string>
#include <iostream>

#include <boost/bind.hpp>

using namespace std;
using namespace boost;

// -----------------------------------------------------------------------
// TextPageElement
// -----------------------------------------------------------------------

class TextPageElement
{
public:
  virtual ~TextPageElement() { }
  virtual bool isTextElement() { return false; }
  virtual void replayElement(TextPage& ts, bool isActivePage) = 0;
  virtual TextPageElement* clone() const = 0;
};

//namespace boost {
inline TextPageElement* new_clone(const TextPageElement& in)
{
  return in.clone();
}
//}

// -----------------------------------------------------------------------
// TextTextPageElement
// -----------------------------------------------------------------------

class TextTextPageElement : public TextPageElement
{
private:
  /// A list of UTF-8 characters to print.
  string list_of_chars_to_print_;

  /// The nextChar on the last operation.
  string next_char_;

public:
  TextTextPageElement();
  virtual bool isTextElement() { return true; }
  virtual void replayElement(TextPage& page, bool isActivePage);
  void append(const string& c, const string& nextChar);

  virtual TextPageElement* clone() const { return new TextTextPageElement(*this); }
};

// -----------------------------------------------------------------------

TextTextPageElement::TextTextPageElement()
{}

// -----------------------------------------------------------------------

void TextTextPageElement::replayElement(TextPage& page, bool isActivePage)
{
  // Sometimes there are empty TextTextPageElements. I hypothesize these happen
  // because of empty strings which just set the speaker's name.
  if (list_of_chars_to_print_.size()) {
    printTextToFunction(bind(&TextPage::character_impl, ref(page), _1, _2),
                        list_of_chars_to_print_, next_char_);
  }
}

// -----------------------------------------------------------------------

void TextTextPageElement::append(const string& c,
                                 const string& nextChar)
{
  list_of_chars_to_print_.append(c);
  next_char_ = nextChar;
}

// -----------------------------------------------------------------------
// NamePageElement
// -----------------------------------------------------------------------

class NamePageElement : public TextPageElement
{
private:
  string name_;
  string nextchar_;

public:
  NamePageElement(const string& name, const string& nextChar)
    : name_(name), nextchar_(nextChar) {}

  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.name_impl(name_, nextchar_);
  }

  virtual TextPageElement* clone() const { return new NamePageElement(*this); }
};

// -----------------------------------------------------------------------
// HardBreakElement
// -----------------------------------------------------------------------
class HardBreakElement : public TextPageElement
{
public:
  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.hardBrake_impl();
  }

  virtual TextPageElement* clone() const { return new HardBreakElement(*this); }
};

// -----------------------------------------------------------------------
// ResetIndentationElement
// -----------------------------------------------------------------------
class ResetIndentationElement : public TextPageElement
{
public:
  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.hardBrake_impl();
  }

  virtual TextPageElement* clone() const
  { return new ResetIndentationElement(*this); }
};

// -----------------------------------------------------------------------
// FontColourElement
// -----------------------------------------------------------------------
class FontColourElement : public TextPageElement
{
private:
  int color;

public:
  FontColourElement(int inColor)
    : color(inColor) {}

  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.fontColour_impl(color);
  }

  virtual TextPageElement* clone() const {
    return new FontColourElement(*this);
  }
};

// -----------------------------------------------------------------------
// SetToRightStartingColorElement
// -----------------------------------------------------------------------
class SetToRightStartingColorElement : public TextPageElement
{
public:
  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.setToRightStartingColor_impl(isActivePage);
  }

  virtual TextPageElement* clone() const {
    return new SetToRightStartingColorElement(*this);
  }
};


// -----------------------------------------------------------------------
// MarkRubyBeginElement
// -----------------------------------------------------------------------
class MarkRubyBeginElement : public TextPageElement
{
public:
  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.markRubyBegin_impl();
  }

  virtual TextPageElement* clone() const {
    return new MarkRubyBeginElement(*this);
  }
};


// -----------------------------------------------------------------------
// DisplayRubyTextElement
// -----------------------------------------------------------------------
class DisplayRubyTextElement : public TextPageElement
{
private:
  string name_;

public:
  DisplayRubyTextElement(const string& name)
    : name_(name) {}

  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.displayRubyText_impl(name_);
  }

  virtual TextPageElement* clone() const {
    return new DisplayRubyTextElement(*this);
  }
};

// -----------------------------------------------------------------------
// TextPage
// -----------------------------------------------------------------------

TextPage::TextPage(RLMachine& machine, int windowNum)
  : machine_(&machine), window_num_(windowNum), number_of_chars_on_page_(0),
    in_ruby_gloss_(false)
{
  addSetToRightStartingColorElement();
}

// -----------------------------------------------------------------------

TextPage::TextPage(const TextPage& rhs)
  : machine_(rhs.machine_), window_num_(rhs.window_num_),
    number_of_chars_on_page_(rhs.number_of_chars_on_page_),
    in_ruby_gloss_(rhs.in_ruby_gloss_)
{
  elements_to_replay_.insert(
    elements_to_replay_.end(),
    rhs.elements_to_replay_.begin(),
    rhs.elements_to_replay_.end());
}

// -----------------------------------------------------------------------

TextPage::~TextPage()
{}

// -----------------------------------------------------------------------

TextPage& TextPage::operator=(const TextPage& rhs)
{
  TextPage tmp(rhs);
  swap(tmp);
  return *this;
}

// -----------------------------------------------------------------------

void TextPage::swap(TextPage& rhs)
{
  elements_to_replay_.swap(rhs.elements_to_replay_);
  std::swap(machine_, rhs.machine_);
  std::swap(window_num_, rhs.window_num_);
  std::swap(number_of_chars_on_page_, rhs.number_of_chars_on_page_);
  std::swap(in_ruby_gloss_, rhs.in_ruby_gloss_);
}

// -----------------------------------------------------------------------

void TextPage::replay(bool isActivePage)
{
  for_each(elements_to_replay_.begin(), elements_to_replay_.end(),
           bind(&TextPageElement::replayElement, _1, ref(*this),
                isActivePage));
}

// ------------------------------------------------- [ Public operations ]

bool TextPage::character(const string& current, const string& next)
{
  bool rendered = character_impl(current, next);

  if(rendered)
  {
    if(elements_to_replay_.size() == 0 ||
       !elements_to_replay_.back().isTextElement())
      elements_to_replay_.push_back(new TextTextPageElement);

    dynamic_cast<TextTextPageElement&>(elements_to_replay_.back()).
      append(current, next);

    number_of_chars_on_page_++;
  }

  return rendered;
}

// -----------------------------------------------------------------------

void TextPage::name(const string& name, const string& nextChar)
{
  elements_to_replay_.push_back(new NamePageElement(name, nextChar));
  number_of_chars_on_page_++;
  name_impl(name, nextChar);
}

// -----------------------------------------------------------------------

void TextPage::hardBrake()
{
  elements_to_replay_.push_back(new HardBreakElement());
  hardBrake_impl();
}

// -----------------------------------------------------------------------

void TextPage::resetIndentation()
{
  elements_to_replay_.push_back(new ResetIndentationElement());
  resetIndentation_impl();
}

// -----------------------------------------------------------------------

void TextPage::fontColour(int color)
{
  elements_to_replay_.push_back(new FontColourElement(color));
  fontColour_impl(color);
}

// -----------------------------------------------------------------------

void TextPage::markRubyBegin()
{
  elements_to_replay_.push_back(new MarkRubyBeginElement());
  markRubyBegin_impl();
}

// -----------------------------------------------------------------------

void TextPage::displayRubyText(const std::string& utf8str)
{
  elements_to_replay_.push_back(new DisplayRubyTextElement(utf8str));
  displayRubyText_impl(utf8str);
}

// -----------------------------------------------------------------------

void TextPage::addSetToRightStartingColorElement()
{
  elements_to_replay_.push_back(new SetToRightStartingColorElement);
}

// -----------------------------------------------------------------------

bool TextPage::character_impl(const string& c,
                              const string& nextChar)
{
  return machine_->system().text().textWindow(*machine_, window_num_)
    .displayChar(*machine_, c, nextChar);
}

// -----------------------------------------------------------------------

void TextPage::name_impl(const string& name,
                         const string& nextChar)
{
  machine_->system().text().textWindow(*machine_, window_num_)
    .setName(*machine_, name, nextChar);
}

// -----------------------------------------------------------------------

void TextPage::hardBrake_impl()
{
  machine_->system().text().textWindow(*machine_, window_num_)
    .hardBrake();
}

// -----------------------------------------------------------------------

void TextPage::resetIndentation_impl()
{
  machine_->system().text().textWindow(*machine_, window_num_)
    .resetIndentation();
}

// -----------------------------------------------------------------------

void TextPage::fontColour_impl(int color)
{
  machine_->system().text().textWindow(*machine_, window_num_)
    .setFontColor(machine_->system().gameexe()("COLOR_TABLE", color));
}

// -----------------------------------------------------------------------

void TextPage::markRubyBegin_impl()
{
  machine_->system().text().textWindow(*machine_, window_num_)
    .markRubyBegin();
  in_ruby_gloss_ = true;
}

// -----------------------------------------------------------------------

void TextPage::displayRubyText_impl(const std::string& utf8str)
{
  machine_->system().text().textWindow(*machine_, window_num_)
    .displayRubyText(*machine_, utf8str);
  in_ruby_gloss_ = false;
}

// -----------------------------------------------------------------------

void TextPage::setToRightStartingColor_impl(bool isActivePage)
{
  Gameexe& gexe = machine_->system().gameexe();
  TextWindow& window = machine_->system().text().textWindow(
    *machine_, window_num_);
  if(!isActivePage)
  {
    GameexeInterpretObject color(gexe("COLOR_TABLE", 254));
    if(color.exists())
      window.setFontColor(color);
  }
}

// -----------------------------------------------------------------------

bool TextPage::isFull() const
{
  return machine_->system().text().textWindow(*machine_, window_num_)
    .isFull();
}
