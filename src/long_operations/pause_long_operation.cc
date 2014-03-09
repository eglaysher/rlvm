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

#include "long_operations/pause_long_operation.h"

#include <vector>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "libReallive/gameexe.h"

using namespace std;

// -----------------------------------------------------------------------
// PauseLongOperation
// -----------------------------------------------------------------------

PauseLongOperation::PauseLongOperation(RLMachine& machine)
    : LongOperation(), machine_(machine), is_done_(false) {
  TextSystem& text = machine_.system().text();
  EventSystem& event = machine_.system().event();

  // Initialize Auto Mode (in case it's activated, or in case it gets
  // activated)
  int numChars = text.currentPage().number_of_chars_on_page();
  automode_time_ = text.getAutoTime(numChars);
  time_at_last_pass_ = event.getTicks();
  total_time_ = 0;

  machine_.system().graphics().markScreenAsDirty(GUT_TEXTSYS);

  // We undo this in the destructor
  text.setInPauseState(true);
}

PauseLongOperation::~PauseLongOperation() {
  machine_.system().text().setInPauseState(false);
}

void PauseLongOperation::mouseMotion(const Point& p) {
  // Tell the text system about the move
  machine_.system().text().setMousePosition(p);
}

bool PauseLongOperation::mouseButtonStateChanged(MouseButton mouseButton,
                                                 bool pressed) {
  GraphicsSystem& graphics = machine_.system().graphics();
  EventSystem& es = machine_.system().event();

  TextSystem& text = machine_.system().text();

  switch (mouseButton) {
    case MOUSE_LEFT: {
      Point pos = es.getCursorPos();
      // Only unhide the interface on release of the left mouse button
      if (graphics.interfaceHidden()) {
        if (!pressed) {
          graphics.toggleInterfaceHidden();
          return true;
        }
      } else if (!text.handleMouseClick(machine_, pos, pressed)) {
        // We *must* only respond on mouseups! This detail matters because in
        // rlBabel, if glosses are enabled, an spause() is called and then the
        // mouse button value returned by GetCursorPos needs to be "2" for the
        // rest of the gloss implementation to work. If we respond on a
        // mousedown, then it'll return "1" instead.
        if (!pressed) {
          if (text.isReadingBacklog()) {
            // Move back to the main page.
            text.stopReadingBacklog();
          } else {
            is_done_ = true;
          }

          return true;
        }
      }
      break;
    }
    case MOUSE_RIGHT:
      if (!pressed) {
        machine_.system().showSyscomMenu(machine_);
        return true;
      }
      break;
    case MOUSE_WHEELUP:
      if (pressed) {
        text.backPage();
        return true;
      }
      break;
    case MOUSE_WHEELDOWN:
      if (pressed) {
        text.forwardPage();
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

bool PauseLongOperation::keyStateChanged(KeyCode keyCode, bool pressed) {
  bool handled = false;

  if (pressed) {
    GraphicsSystem& graphics = machine_.system().graphics();

    if (graphics.interfaceHidden()) {
      graphics.toggleInterfaceHidden();
      handled = true;
    } else {
      TextSystem& text = machine_.system().text();
      bool ctrlKeySkips = text.ctrlKeySkip();

      if (ctrlKeySkips && (keyCode == RLKEY_RCTRL || keyCode == RLKEY_LCTRL)) {
        is_done_ = true;
        handled = true;
      } else if (keyCode == RLKEY_SPACE) {
        graphics.toggleInterfaceHidden();
        handled = true;
      } else if (keyCode == RLKEY_UP) {
        text.backPage();
        handled = true;
      } else if (keyCode == RLKEY_DOWN) {
        text.forwardPage();
        handled = true;
      } else if (keyCode == RLKEY_RETURN) {
        if (text.isReadingBacklog())
          text.stopReadingBacklog();
        else
          is_done_ = true;

        handled = true;
      }
    }
  }

  return handled;
}

bool PauseLongOperation::operator()(RLMachine& machine) {
  // Check to see if we're done because of the auto mode timer
  if (machine_.system().text().autoMode()) {
    if (AutomodeTimerFired() && !machine_.system().sound().koePlaying())
      is_done_ = true;
  }

  // Check to see if we're done because we're being asked to pause on a piece
  // of text we've already hit.
  if (machine_.system().fastForward())
    is_done_ = true;

  if (is_done_) {
    // Stop all voices before continuing.
    machine_.system().sound().koeStop();
  }

  return is_done_;
}

bool PauseLongOperation::AutomodeTimerFired() {
  int current_time = machine_.system().event().getTicks();
  int time_since_last_pass = current_time - time_at_last_pass_;
  time_at_last_pass_ = current_time;

  if (machine_.system().event().timeOfLastMouseMove() < (current_time - 2000)) {
    // If the mouse has been moved within the last two seconds, don't advance
    // the timer so the user has a chance to click on buttons.
    total_time_ += time_since_last_pass;
    return total_time_ >= automode_time_;
  }

  return false;
}

// -----------------------------------------------------------------------
// NewPageAfterLongop
// -----------------------------------------------------------------------
NewPageAfterLongop::NewPageAfterLongop(LongOperation* inOp)
    : PerformAfterLongOperationDecorator(inOp) {}

NewPageAfterLongop::~NewPageAfterLongop() {}

void NewPageAfterLongop::performAfterLongOperation(RLMachine& machine) {
  TextSystem& text = machine.system().text();
  text.snapshot();
  text.currentWindow()->clearWin();
  text.newPageOnWindow(text.activeWindow());
}

// -----------------------------------------------------------------------
// NewParagraphAfterLongop
// -----------------------------------------------------------------------
NewParagraphAfterLongop::NewParagraphAfterLongop(LongOperation* inOp)
    : PerformAfterLongOperationDecorator(inOp) {}

NewParagraphAfterLongop::~NewParagraphAfterLongop() {}

void NewParagraphAfterLongop::performAfterLongOperation(RLMachine& machine) {
  TextPage& page = machine.system().text().currentPage();
  page.ResetIndentation();
  page.HardBrake();
}
