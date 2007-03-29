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

#include <string>
#include <iostream>

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
  SetWindowTextPageElement(int in);
  virtual void replayElement(TextPage& ts);  
};

// -----------------------------------------------------------------------

SetWindowTextPageElement::SetWindowTextPageElement(int in)
  : m_toSetTo(in)
{}

// -----------------------------------------------------------------------

void SetWindowTextPageElement::replayElement(TextPage& page)
{
  page.setWindow_impl(m_toSetTo);
}

// -----------------------------------------------------------------------
// TextTextPageElement
// -----------------------------------------------------------------------

class TextTextPageElement : public TextPageElement
{
private:
  /// A list of UTF-8 characters to print.
  std::string m_listOfCharsToPrint;

  /// The nextChar on the last operation.
  std::string m_nextChar;

public:
  TextTextPageElement();
  virtual bool isTextElement() { return true; }
  virtual void replayElement(TextPage& ts);  
  void append(const std::string& c, const std::string& nextChar);
};

// -----------------------------------------------------------------------

TextTextPageElement::TextTextPageElement()
{}

// -----------------------------------------------------------------------

void TextTextPageElement::replayElement(TextPage& page)
{
  // Iterate over each incoming character to display (we do this
  // instead of rendering the entire string so that we can perform
  // indentation, et cetera.)
  string::const_iterator cur = m_listOfCharsToPrint.begin();
  string::const_iterator tmp = cur;
  string::const_iterator end = m_listOfCharsToPrint.end();
  utf8::next(tmp, end);
  string curChar(cur, tmp);
  for(cur = tmp; tmp != end; cur = tmp)
  {
    utf8::next(tmp, end);
    string next(cur, tmp);
    page.character_impl(curChar, next);
    curChar = next;
  }

  page.character_impl(curChar, m_nextChar);
  
//  page.text_impl(m_payload);
}

// -----------------------------------------------------------------------

void TextTextPageElement::append(const std::string& c, 
                                 const std::string& nextChar)
{
  m_listOfCharsToPrint.append(c); 
  m_nextChar = nextChar;
}

// -----------------------------------------------------------------------
// TextPage
// -----------------------------------------------------------------------

TextPage::TextPage(RLMachine& machine)
  : m_machine(machine), m_currentWindow(0)
{}

// -----------------------------------------------------------------------

TextPage::~TextPage()
{}

// ------------------------------------------------- [ Public operations ]

void TextPage::setWindow(int windowNum)
{
  m_elementsToReplay.push_back(new SetWindowTextPageElement(windowNum) );
  setWindow_impl(windowNum);
}

// -----------------------------------------------------------------------

bool TextPage::character(const std::string& current, const std::string& next)
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

// ------------------------------------------- [ Private implementations ]

void TextPage::setWindow_impl(int windowNum)
{
  m_currentWindow = windowNum;
}

// -----------------------------------------------------------------------

bool TextPage::character_impl(const std::string& c, 
                              const std::string& nextChar)
{
  return m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .displayChar(m_machine, c, nextChar);
}

// -----------------------------------------------------------------------


bool TextPage::isFull() const
{
  return m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .isFull();
}
