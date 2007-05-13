// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

namespace boost {
inline TextPageElement* new_clone(const TextPageElement& in)
{
  return in.clone();
}
}

// -----------------------------------------------------------------------
// TextTextPageElement
// -----------------------------------------------------------------------

class TextTextPageElement : public TextPageElement
{
private:
  /// A list of UTF-8 characters to print.
  string m_listOfCharsToPrint;

  /// The nextChar on the last operation.
  string m_nextChar;

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
  printTextToFunction(bind(&TextPage::character_impl, ref(page), _1, _2),
                      m_listOfCharsToPrint, m_nextChar);
}

// -----------------------------------------------------------------------

void TextTextPageElement::append(const string& c, 
                                 const string& nextChar)
{
  m_listOfCharsToPrint.append(c); 
  m_nextChar = nextChar;
}

// -----------------------------------------------------------------------
// NamePageElement
// -----------------------------------------------------------------------

class NamePageElement : public TextPageElement
{
private:
  string m_name;
  string m_nextchar;

public:
  NamePageElement(const string& name, const string& nextChar)
    : m_name(name), m_nextchar(nextChar) {}

  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.name_impl(m_name, m_nextchar);
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
  string m_name;

public:
  DisplayRubyTextElement(const string& name)
    : m_name(name) {}

  virtual void replayElement(TextPage& page, bool isActivePage)
  {
    page.displayRubyText_impl(m_name);
  }

  virtual TextPageElement* clone() const { 
    return new DisplayRubyTextElement(*this); 
  }
};

// -----------------------------------------------------------------------
// TextPage
// -----------------------------------------------------------------------

TextPage::TextPage(RLMachine& machine, int windowNum)
  : m_machine(&machine), m_windowNum(windowNum), m_numberOfCharsOnPage(0),
    m_inRubyGloss(false)
{
  addSetToRightStartingColorElement();
}

// -----------------------------------------------------------------------

TextPage::TextPage(const TextPage& rhs)
  : m_machine(rhs.m_machine), m_windowNum(rhs.m_windowNum), 
    m_numberOfCharsOnPage(rhs.m_numberOfCharsOnPage), 
    m_inRubyGloss(rhs.m_inRubyGloss)
{
  m_elementsToReplay.insert(
    m_elementsToReplay.end(),
    rhs.m_elementsToReplay.begin(),
    rhs.m_elementsToReplay.end());
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
  m_elementsToReplay.swap(rhs.m_elementsToReplay);
  std::swap(m_machine, rhs.m_machine);
  std::swap(m_windowNum, rhs.m_windowNum);
  std::swap(m_numberOfCharsOnPage, rhs.m_numberOfCharsOnPage);
  std::swap(m_inRubyGloss, rhs.m_inRubyGloss);
}

// -----------------------------------------------------------------------

void TextPage::replay(bool isActivePage)
{
  for_each(m_elementsToReplay.begin(), m_elementsToReplay.end(),
           bind(&TextPageElement::replayElement, _1, ref(*this), 
                isActivePage));
}

// ------------------------------------------------- [ Public operations ]

bool TextPage::character(const string& current, const string& next)
{
  bool rendered = character_impl(current, next);

  if(rendered)
  {
    if(m_elementsToReplay.size() == 0 || 
       !m_elementsToReplay.back().isTextElement())
      m_elementsToReplay.push_back(new TextTextPageElement);

    dynamic_cast<TextTextPageElement&>(m_elementsToReplay.back()).
      append(current, next);

    m_numberOfCharsOnPage++;
  }

  return rendered;
}

// -----------------------------------------------------------------------

void TextPage::name(const string& name, const string& nextChar)
{
  m_elementsToReplay.push_back(new NamePageElement(name, nextChar));
  m_numberOfCharsOnPage++;
  name_impl(name, nextChar);
}

// -----------------------------------------------------------------------

void TextPage::hardBrake()
{
  m_elementsToReplay.push_back(new HardBreakElement());
  hardBrake_impl();
}

// -----------------------------------------------------------------------

void TextPage::resetIndentation()
{
  m_elementsToReplay.push_back(new ResetIndentationElement());
  resetIndentation_impl();
}

// -----------------------------------------------------------------------

void TextPage::fontColour(int color)
{
  m_elementsToReplay.push_back(new FontColourElement(color));
  fontColour_impl(color);
}

// -----------------------------------------------------------------------

void TextPage::markRubyBegin()
{
  m_elementsToReplay.push_back(new MarkRubyBeginElement());
  markRubyBegin_impl();
}

// -----------------------------------------------------------------------

void TextPage::displayRubyText(const std::string& utf8str)
{
  m_elementsToReplay.push_back(new DisplayRubyTextElement(utf8str));
  displayRubyText_impl(utf8str);
}

// -----------------------------------------------------------------------

void TextPage::addSetToRightStartingColorElement()
{
  m_elementsToReplay.push_back(new SetToRightStartingColorElement);
}

// -----------------------------------------------------------------------

bool TextPage::character_impl(const string& c, 
                              const string& nextChar)
{
  return m_machine->system().text().textWindow(*m_machine, m_windowNum)
    .displayChar(*m_machine, c, nextChar);
}

// -----------------------------------------------------------------------

void TextPage::name_impl(const string& name, 
                         const string& nextChar)
{
  m_machine->system().text().textWindow(*m_machine, m_windowNum)
    .setName(*m_machine, name, nextChar);
}

// -----------------------------------------------------------------------

void TextPage::hardBrake_impl()
{
  m_machine->system().text().textWindow(*m_machine, m_windowNum)
    .hardBrake();
}

// -----------------------------------------------------------------------

void TextPage::resetIndentation_impl()
{
  m_machine->system().text().textWindow(*m_machine, m_windowNum)
    .resetIndentation();
}

// -----------------------------------------------------------------------

void TextPage::fontColour_impl(int color)
{
  m_machine->system().text().textWindow(*m_machine, m_windowNum)
    .setFontColor(m_machine->system().gameexe()("COLOR_TABLE", color));
}

// -----------------------------------------------------------------------

void TextPage::markRubyBegin_impl()
{
  m_machine->system().text().textWindow(*m_machine, m_windowNum)
    .markRubyBegin();
  m_inRubyGloss = true;
}

// -----------------------------------------------------------------------

void TextPage::displayRubyText_impl(const std::string& utf8str)
{
  m_machine->system().text().textWindow(*m_machine, m_windowNum)
    .displayRubyText(*m_machine, utf8str);
  m_inRubyGloss = false;
}

// -----------------------------------------------------------------------

void TextPage::setToRightStartingColor_impl(bool isActivePage)
{
  Gameexe& gexe = m_machine->system().gameexe();
  TextWindow& window = m_machine->system().text().textWindow(
    *m_machine, m_windowNum);
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
  return m_machine->system().text().textWindow(*m_machine, m_windowNum)
    .isFull();
}
