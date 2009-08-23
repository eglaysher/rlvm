// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "LongOperations/TextoutLongOperation.hpp"
#include "LongOperations/PauseLongOperation.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include "Utilities/Exception.hpp"

#include <boost/utility.hpp>

#include <string>
#include <iostream>
#include <algorithm>

#include "utf8.h"

using namespace std;

// -----------------------------------------------------------------------
// TextoutLongOperation
// -----------------------------------------------------------------------

TextoutLongOperation::TextoutLongOperation(RLMachine& machine,
                                           const std::string& utf8string)
  : m_utf8string(utf8string), current_codepoint_(0),
    current_position_(m_utf8string.begin()), no_wait_(false) {
  // Retrieve the first character (prime the loop in operator())
  string::iterator tmp = current_position_;
  if (tmp == m_utf8string.end()) {
    current_char_ = "";
  } else {
    current_codepoint_ = utf8::next(tmp, m_utf8string.end());
    current_char_ = string(current_position_, tmp);
    current_position_ = tmp;
  }

  // If we are inside a ruby gloss right now, don't delay at
  // all. Render the entire gloss!
  if (machine.system().text().currentPage().inRubyGloss())
    no_wait_ = true;
}

// -----------------------------------------------------------------------

TextoutLongOperation::~TextoutLongOperation() {
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::mouseButtonStateChanged(MouseButton mouseButton,
                                                   bool pressed) {
  if (pressed && mouseButton == MOUSE_LEFT) {
    no_wait_ = true;
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::keyStateChanged(KeyCode keyCode, bool pressed) {
  if (pressed && (keyCode == RLKEY_LCTRL || keyCode == RLKEY_RCTRL)) {
    no_wait_ = true;
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::displayAsMuchAsWeCanThenPause(RLMachine& machine) {
  bool paused = false;
  while (!displayOneMoreCharacter(machine, paused))
    if (paused)
      return false;

  return true;
}

// -----------------------------------------------------------------------

/**
 * Extract a name and send it to the text system as an automic
 * operation.
 *
 * @todo Right now, this doesn't deal with \#\#\#PRINT() syntax in the
 *       name, even though character names are one of the places where
 *       that's evaluated.
 */
bool TextoutLongOperation::displayName(RLMachine& machine) {
  // Ignore the starting bracket
  string::iterator it = current_position_;
  string::iterator curend = it;
  string::iterator strend = m_utf8string.end();
  int codepoint = utf8::next(it, strend);

  // Eat all characters between the name brackets
  while (codepoint != 0x3011 && it != strend) {
    curend = it;
    codepoint = utf8::next(it, strend);
  }

  if (codepoint != 0x3011 && it == strend) {
    throw SystemError("Malformed string code. Opening bracket in \\{name}"
                      " construct,  but missing closing bracket.");
  }

  // Grab the name
  string name(current_position_, curend);

  // Consume the next character
  current_position_ = it;

  if (it != strend) {
    current_codepoint_ = utf8::next(it, strend);
    current_char_ = string(current_position_, it);
    current_position_ = it;
  }

  TextPage& page = machine.system().text().currentPage();
  page.name(name, current_char_);

  // Stop if this was the end of input
  return it == strend;
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::displayOneMoreCharacter(RLMachine& machine,
                                                   bool& paused) {
  if (current_codepoint_ == 0x3010) {
    // The current character is the opening character for a name. We
    // treat names as a single display operation
    return displayName(machine);
  } else {
    // Isolate the next character
    string::iterator it = current_position_;
    string::iterator strend = m_utf8string.end();

    if (it != strend) {
      int codepoint = utf8::next(it, strend);
      TextPage& page = machine.system().text().currentPage();
      if (codepoint) {
        string nextChar(current_position_, it);
        bool rendered = page.character(current_char_, nextChar);

        // Check to see if this character was rendered to the screen. If
        // this is false, then the page is probably full and the check
        // later on will do something about that.
        if (rendered) {
          current_char_ = nextChar;
          current_position_ = it;
        }
      } else {
        // advance to the next character if we've somehow hit an
        // embedded NULL that isn't the end of the string
        current_position_ = it;
      }

      // Call the pause operation if we've filled up the current page.
      if (page.isFull()) {
        paused = true;
        machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
        machine.pushLongOperation(
          new NewPageAfterLongop(new PauseLongOperation(machine)));
      }

      return false;
    } else {
      machine.system().text().currentPage().character(current_char_, "");

      return true;
    }
  }
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::operator()(RLMachine& machine) {
  // Check to make sure we're not trying to do a textout (impossible!)
  if (!machine.system().text().systemVisible())
    throw rlvm::Exception("Trying to Textout while TextSystem is hidden!");

  if (no_wait_)
    return displayAsMuchAsWeCanThenPause(machine);
  else {
    bool paused = false;
    return displayOneMoreCharacter(machine, paused);
  }
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::sleepEveryTick() {
  return !no_wait_;
}
