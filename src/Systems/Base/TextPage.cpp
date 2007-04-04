// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 El Riot
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

#include "Modules/utf8.h"
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
  virtual void replayElement(TextPage& ts) = 0;
};

// -----------------------------------------------------------------------
// SetWindowTextPageElement
// -----------------------------------------------------------------------

class SetWindowTextPageElement : public TextPageElement
{
private:
  int m_toSetTo;
public:
  SetWindowTextPageElement(int in) : m_toSetTo(in) {}

  virtual void replayElement(TextPage& page)
  {
    page.setWindow_impl(m_toSetTo);
  }
};

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
  virtual void replayElement(TextPage& page);  
  void append(const string& c, const string& nextChar);
};

// -----------------------------------------------------------------------

TextTextPageElement::TextTextPageElement()
{}

// -----------------------------------------------------------------------

void TextTextPageElement::replayElement(TextPage& page)
{
  cerr << "Replay: print text: " << m_listOfCharsToPrint << endl;
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

  virtual void replayElement(TextPage& page)
  {
    page.name_impl(m_name, m_nextchar);
  }
};

// -----------------------------------------------------------------------
// HardBreakElement
// -----------------------------------------------------------------------
class HardBreakElement : public TextPageElement
{
public:
  HardBreakElement() {}
  virtual void replayElement(TextPage& page)
  {
    page.hardBrake_impl();
  }
};

// -----------------------------------------------------------------------
// ResetIndentationElement
// -----------------------------------------------------------------------
class ResetIndentationElement : public TextPageElement
{
public:
  ResetIndentationElement() { }
  virtual void replayElement(TextPage& page)
  {
    page.hardBrake_impl();
  }
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

  virtual void replayElement(TextPage& page)
  {
    page.fontColour_impl(color);
  }
};


// -----------------------------------------------------------------------
// TextPage
// -----------------------------------------------------------------------

TextPage::TextPage(RLMachine& machine)
  : m_machine(machine), m_currentWindow(0)
{}

// -----------------------------------------------------------------------

TextPage::~TextPage()
{}

// -----------------------------------------------------------------------

void TextPage::replay()
{
  for_each(m_elementsToReplay.begin(), m_elementsToReplay.end(),
           bind(&TextPageElement::replayElement, _1, ref(*this)));
}

// ------------------------------------------------- [ Public operations ]

void TextPage::setWindow(int windowNum)
{
  m_elementsToReplay.push_back(new SetWindowTextPageElement(windowNum) );
  setWindow_impl(windowNum);
}

// -----------------------------------------------------------------------

bool TextPage::character(const string& current, const string& next)
{
  bool rendered = character_impl(current, next);

  if(rendered)
  {
    if(!m_elementsToReplay.back().isTextElement())
      m_elementsToReplay.push_back(new TextTextPageElement);

    dynamic_cast<TextTextPageElement&>(m_elementsToReplay.back()).
      append(current, next);
  }

  return rendered;
}

// -----------------------------------------------------------------------

void TextPage::name(const string& name, const string& nextChar)
{
  m_elementsToReplay.push_back(new NamePageElement(name, nextChar));
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

// ------------------------------------------- [ Private implementations ]

void TextPage::setWindow_impl(int windowNum)
{
  m_currentWindow = windowNum;
}

// -----------------------------------------------------------------------

bool TextPage::character_impl(const string& c, 
                              const string& nextChar)
{
  return m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .displayChar(m_machine, c, nextChar);
}

// -----------------------------------------------------------------------

void TextPage::name_impl(const string& name, 
                         const string& nextChar)
{
  m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .setName(m_machine, name, nextChar);
}

// -----------------------------------------------------------------------

void TextPage::hardBrake_impl()
{
  m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .hardBrake();
}

// -----------------------------------------------------------------------

void TextPage::resetIndentation_impl()
{
  m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .resetIndentation();
}

// -----------------------------------------------------------------------

void TextPage::fontColour_impl(int color)
{
  m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .setFontColor(m_machine.system().gameexe()("COLOR_TABLE", color));
}

// -----------------------------------------------------------------------

bool TextPage::isFull() const
{
  return m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .isFull();
}
