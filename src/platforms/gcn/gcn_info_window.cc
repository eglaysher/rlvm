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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#include "platforms/gcn/gcn_info_window.h"

#include <algorithm>
#include <vector>

#include "platforms/gcn/gcn_button.h"
#include "platforms/gcn/gcn_platform.h"
#include "systems/base/rlvm_info.h"
#include "utilities/string_utilities.h"

using std::vector;

const int PADDING = 5;

// -----------------------------------------------------------------------

namespace {

int max_space(const std::vector<gcn::Label*>& labels) {
  std::vector<gcn::Label*>::const_iterator it = std::max_element(
      labels.begin(), labels.end(), [](gcn::Label* one, gcn::Label* two) {
        return one->getWidth() < two->getWidth();
      });
  if (it != labels.end())
    return (*it)->getWidth();
  else
    return 0;
}

void set_size_and_align(std::vector<gcn::Label*>& items,
                        int max_size,
                        gcn::Graphics::Alignment alignment) {
  for (auto* label : items) {
    label->setWidth(max_size);
    label->setAlignment(alignment);
  }
}

}  // anonymous namespace

// -----------------------------------------------------------------------

GCNInfoWindow::GCNInfoWindow(RLMachine& machine,
                             const RlvmInfo& info,
                             GCNPlatform* platform)
    : GCNWindow(platform) {
  int height = PADDING;

  vector<gcn::Label*> keys;
  keys.push_back(new gcn::Label("Name: "));
  keys.push_back(new gcn::Label("Brand: "));
  keys.push_back(new gcn::Label("Game Version: "));
  keys.push_back(new gcn::Label("Game Path: "));
  keys.push_back(new gcn::Label("RLVM Version: "));
  keys.push_back(new gcn::Label("rlBabel: "));
  keys.push_back(new gcn::Label("Text Encoding: "));

  vector<gcn::Label*> values;
  values.push_back(new gcn::Label(info.game_name));
  values.push_back(new gcn::Label(info.game_brand));
  values.push_back(new gcn::Label(info.game_version));
  values.push_back(new gcn::Label(info.game_path));
  values.push_back(new gcn::Label(info.rlvm_version));
  values.push_back(
      new gcn::Label(info.rlbabel_loaded ? "Enabled" : "Disabled"));
  values.push_back(
      new gcn::Label(TransformationName(info.text_transformation)));

  int max_key_space = max_space(keys);
  int max_value_space = max_space(values);
  set_size_and_align(keys, max_key_space, gcn::Graphics::RIGHT);
  set_size_and_align(values, max_value_space, gcn::Graphics::LEFT);

  vector<gcn::Label*>::const_iterator key_it = keys.begin();
  vector<gcn::Label*>::const_iterator values_it = values.begin();
  for (; key_it != keys.end() && values_it != values.end();
       ++key_it, ++values_it) {
    Container::add(*key_it, PADDING, height);
    Container::add(*values_it, PADDING + max_key_space + PADDING, height);

    height += (*values_it)->getHeight() + PADDING;
  }

  int width = PADDING + max_key_space + PADDING + max_value_space + PADDING;
  ok_button_ = new GCNButton("OK");
  ok_button_->setWidth(ok_button_->getWidth() + PADDING);
  ok_button_->setAlignment(gcn::Graphics::CENTER);
  ok_button_->setActionEventId(EVENT_CANCEL);
  ok_button_->addActionListener(this);

  Container::add(ok_button_, width - PADDING - ok_button_->getWidth(), height);
  height += ok_button_->getHeight() + PADDING;

  setSize(width, height);
}

// -----------------------------------------------------------------------

GCNInfoWindow::~GCNInfoWindow() {}

// -----------------------------------------------------------------------

void GCNInfoWindow::action(const gcn::ActionEvent& actionEvent) {
  if (actionEvent.getId() == EVENT_CANCEL)
    platform_->windowCanceled(this);
}
