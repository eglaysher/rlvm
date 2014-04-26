// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "systems/base/text_waku_normal.h"

#include <functional>
#include <iomanip>
#include <sstream>
#include <string>

#include "systems/base/graphics_system.h"
#include "systems/base/rect.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"
#include "systems/base/text_window.h"
#include "systems/base/text_window_button.h"
#include "libreallive/gameexe.h"

using std::endl;
using std::ostringstream;
using std::setfill;
using std::setw;
using std::placeholders::_1;

namespace {

// Definitions for the location and Gameexe.ini keys describing various text
// window buttons.
//
// Previously was using a map keyed on strings. In rendering code. With keys
// that had similar prefixes. WTF was I smoking...
static struct ButtonInfo {
  int index;
  const char* button_name;
  int waku_offset;
} BUTTON_INFO[] = {{0, "CLEAR_BOX", 8},
                   {1, "MSGBKLEFT_BOX", 24},
                   {2, "MSGBKRIGHT_BOX", 32},
                   {3, "EXBTN_000_BOX", 40},
                   {4, "EXBTN_001_BOX", 48},
                   {5, "EXBTN_002_BOX", 56},
                   {6, "EXBTN_003_BOX", 64},
                   {7, "EXBTN_004_BOX", 72},
                   {8, "EXBTN_005_BOX", 80},
                   {9, "EXBTN_006_BOX", 88},
                   {10, "READJUMP_BOX", 104},
                   {11, "AUTOMODE_BOX", 112},
                   {-1, NULL, -1}};

}  // namespace

// -----------------------------------------------------------------------
// TextWakuNormal
// -----------------------------------------------------------------------
TextWakuNormal::TextWakuNormal(System& system,
                               TextWindow& window,
                               int setno,
                               int no)
    : system_(system), window_(window), setno_(setno), no_(no) {
  LoadWindowWaku();
}

TextWakuNormal::~TextWakuNormal() {}

void TextWakuNormal::Execute() {
  for (int i = 0; BUTTON_INFO[i].index != -1; ++i) {
    if (button_map_[i]) {
      button_map_[i]->Execute();
    }
  }
}

void TextWakuNormal::Render(std::ostream* tree,
                            Point box_location,
                            Size namebox_size) {
  if (tree) {
    *tree << "    Window Waku(" << setno_ << ", " << no_ << "):" << endl;
  }

  if (waku_backing_) {
    Size backing_size = waku_backing_->GetSize();
    waku_backing_->RenderToScreenAsColorMask(Rect(Point(0, 0), backing_size),
                                             Rect(box_location, backing_size),
                                             window_.colour(),
                                             window_.filter());

    if (tree) {
      *tree << "      Backing Area: " << Rect(box_location, backing_size)
            << endl;
    }
  }

  if (waku_main_) {
    Size main_size = waku_main_->GetSize();
    waku_main_->RenderToScreen(
        Rect(Point(0, 0), main_size), Rect(box_location, main_size), 255);

    if (tree) {
      *tree << "      Main Area: " << Rect(box_location, main_size) << endl;
    }
  }

  if (waku_button_)
    RenderButtons();
}

void TextWakuNormal::RenderButtons() {
  for (int i = 0; BUTTON_INFO[i].index != -1; ++i) {
    if (button_map_[i]) {
      button_map_[i]->Render(window_, waku_button_, BUTTON_INFO[i].waku_offset);
    }
  }
}

Size TextWakuNormal::GetSize(const Size& text_surface) const {
  if (waku_main_)
    return waku_main_->GetSize();
  else if (waku_backing_)
    return waku_backing_->GetSize();
  else
    return text_surface;
}

Point TextWakuNormal::InsertionPoint(const Rect& waku_rect,
                                     const Size& padding,
                                     const Size& surface_size,
                                     bool center) const {
  // In normal type 5 wakus, we just offset from the top left corner by padding
  // amounts.
  Point insertion_point = waku_rect.origin() + padding;
  if (center) {
    int half_width = (waku_rect.width() - 2 * padding.width()) / 2;
    int half_text_width = surface_size.width() / 2;
    insertion_point += Point(half_width - half_text_width, 0);
  }

  return insertion_point;
}

void TextWakuNormal::SetMousePosition(const Point& pos) {
  for (int i = 0; BUTTON_INFO[i].index != -1; ++i) {
    if (button_map_[i]) {
      button_map_[i]->SetMousePosition(window_, pos);
    }
  }
}

bool TextWakuNormal::HandleMouseClick(RLMachine& machine,
                                      const Point& pos,
                                      bool pressed) {
  for (int i = 0; BUTTON_INFO[i].index != -1; ++i) {
    if (button_map_[i]) {
      if (button_map_[i]->HandleMouseClick(machine, window_, pos, pressed))
        return true;
    }
  }

  return false;
}

void TextWakuNormal::LoadWindowWaku() {
  GameexeInterpretObject waku(system_.gameexe()("WAKU", setno_, no_));

  SetWakuMain(waku("NAME").ToString(""));
  SetWakuBacking(waku("BACK").ToString(""));
  SetWakuButton(waku("BTN").ToString(""));

  TextSystem& ts = system_.text();
  GraphicsSystem& gs = system_.graphics();

  if (waku("CLEAR_BOX").Exists()) {
    button_map_[0].reset(new ActionTextWindowButton(
        system_,
        ts.window_clear_use(),
        waku("CLEAR_BOX"),
        std::bind(&GraphicsSystem::ToggleInterfaceHidden, std::ref(gs))));
  }
  if (waku("MSGBKLEFT_BOX").Exists()) {
    button_map_[1].reset(new RepeatActionWhileHoldingWindowButton(
        system_,
        ts.window_msgbkleft_use(),
        waku("MSGBKLEFT_BOX"),
        std::bind(&TextSystem::BackPage, std::ref(ts)),
        250));
  }
  if (waku("MSGBKRIGHT_BOX").Exists()) {
    button_map_[2].reset(new RepeatActionWhileHoldingWindowButton(
        system_,
        ts.window_msgbkright_use(),
        waku("MSGBKRIGHT_BOX"),
        std::bind(&TextSystem::ForwardPage, std::ref(ts)),
        250));
  }

  for (int i = 0; i < 7; ++i) {
    GameexeInterpretObject wbcall(system_.gameexe()("WBCALL", i));
    ostringstream oss;
    oss << "EXBTN_" << setw(3) << setfill('0') << i << "_BOX";
    if (waku(oss.str()).Exists()) {
      button_map_[3 + i].reset(new ExbtnWindowButton(
          system_, ts.window_exbtn_use(), waku(oss.str()), wbcall));
    }
  }

  if (waku("READJUMP_BOX").Exists()) {
    ActivationTextWindowButton* readjump_box = new ActivationTextWindowButton(
        system_,
        ts.window_read_jump_use(),
        waku("READJUMP_BOX"),
        std::bind(&TextSystem::SetSkipMode, std::ref(ts), _1));
    readjump_box->SetEnabledNotification(
        NotificationType::SKIP_MODE_ENABLED_CHANGED);
    readjump_box->SetChangeNotification(
        NotificationType::SKIP_MODE_STATE_CHANGED);

    // Set the initial enabled state. If true, we'll get a signal enabling it
    // immediately.
    readjump_box->SetEnabled(ts.kidoku_read());

    button_map_[10].reset(readjump_box);
  }

  if (waku("AUTOMODE_BOX").Exists()) {
    ActivationTextWindowButton* automode_button =
        new ActivationTextWindowButton(
            system_,
            ts.window_automode_use(),
            waku("AUTOMODE_BOX"),
            std::bind(&TextSystem::SetAutoMode, std::ref(ts), _1));
    automode_button->SetChangeNotification(
        NotificationType::AUTO_MODE_STATE_CHANGED);

    button_map_[11].reset(automode_button);
  }

  /*
   * TODO: I didn't translate these to the new way of doing things. I don't
   * seem to be rendering them. Must deal with this later.
   *
  string key = "MOVE_BOX";
  button_map_.insert(
    key, new TextWindowButton(ts.window_move_use(), waku("MOVE_BOX")));

  key = string("MSGBK_BOX");
  button_map_.insert(
    key, new TextWindowButton(ts.window_msgbk_use(), waku("MSGBK_BOX")));
  */
}

void TextWakuNormal::SetWakuMain(const std::string& name) {
  if (name != "")
    waku_main_ = system_.graphics().GetSurfaceNamed(name);
  else
    waku_main_.reset();
}

void TextWakuNormal::SetWakuBacking(const std::string& name) {
  if (name != "") {
    waku_backing_.reset(system_.graphics().GetSurfaceNamed(name)->Clone());
    waku_backing_->SetIsMask(true);
  } else {
    waku_backing_.reset();
  }
}

void TextWakuNormal::SetWakuButton(const std::string& name) {
  if (name != "")
    waku_button_ = system_.graphics().GetSurfaceNamed(name);
  else
    waku_button_.reset();
}
