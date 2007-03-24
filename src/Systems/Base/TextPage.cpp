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

#include "Systems/Base/System.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"

#include "MachineBase/RLMachine.hpp"

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
  std::string m_payload;
public:
  TextTextPageElement(const std::string& payload);

  virtual bool isTextElement() { return true; }

  virtual void replayElement(TextPage& ts);  

  void append(const std::string& in) { m_payload.append(in); }
};

// -----------------------------------------------------------------------

TextTextPageElement::TextTextPageElement(const std::string& payload)
  : m_payload(payload)
{}

// -----------------------------------------------------------------------

void TextTextPageElement::replayElement(TextPage& page)
{
  page.text_impl(m_payload);
}

// -----------------------------------------------------------------------
// TextPage
// -----------------------------------------------------------------------

TextPage::TextPage(RLMachine& machine)
  : m_machine(machine), m_currentWindow(0)
{

}

// -----------------------------------------------------------------------

TextPage::~TextPage()
{

}

// ------------------------------------------------- [ Public operations ]

void TextPage::setWindow(int windowNum)
{
  m_elementsToReplay.push_back(new SetWindowTextPageElement(windowNum) );
  setWindow_impl(windowNum);
}

// -----------------------------------------------------------------------

void TextPage::character(const std::string& current, const std::string& next)
{
  if(!m_elementsToReplay.back().isTextElement())
    m_elementsToReplay.push_back(new TextTextPageElement(current));
  else
  {
    dynamic_cast<TextTextPageElement&>(m_elementsToReplay.back()).
      append(current);
  }

  m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .displayChar(m_machine, current, next);
}

// -----------------------------------------------------------------------

void TextPage::text(const std::string& text)
{
  m_elementsToReplay.push_back(new TextTextPageElement(text));
  text_impl(text);
}

// ------------------------------------------- [ Private implementations ]

void TextPage::setWindow_impl(int windowNum)
{
  m_currentWindow = windowNum;
}

// -----------------------------------------------------------------------

void TextPage::text_impl(const std::string& text)
{
//  cerr << "Displaying to " << m_currentWindow << endl;
  m_machine.system().text().textWindow(m_machine, m_currentWindow)
    .displayText(m_machine, text);
}
