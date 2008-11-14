// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Platforms/gcn/GCNButton.hpp"

#include "Platforms/gcn/gcnUtils.hpp"

#include <guichan/exception.hpp>
#include <guichan/font.hpp>

ImageRect GCNButton::s_button;
ImageRect GCNButton::s_buttonhi;
ImageRect GCNButton::s_buttonpress;
ImageRect GCNButton::s_button_disabled;

// -----------------------------------------------------------------------
// GCNButton
// -----------------------------------------------------------------------
GCNButton::GCNButton()
{
  init();
}

// -----------------------------------------------------------------------

GCNButton::GCNButton(const std::string& label)
  : gcn::Button(label)
{
  init();
}

// -----------------------------------------------------------------------

GCNButton::~GCNButton()
{}

// -----------------------------------------------------------------------

void GCNButton::draw(gcn::Graphics* graphics)
{
  static_cast<GCNGraphics*>(graphics)->
    drawImageRect(0, 0, getWidth(), getHeight(), currentButtonImage());

  graphics->setColor(getForegroundColor());

  int textX;
  int textY = getHeight() / 2 - getFont()->getHeight() / 2;

  switch (getAlignment()) {
  case gcn::Graphics::LEFT:
    textX = 4;
    break;
  case gcn::Graphics::CENTER:
    textX = getWidth() / 2;
    break;
  case gcn::Graphics::RIGHT:
    textX = getWidth() - 4;
    break;
  default:
    throw GCN_EXCEPTION("GCNButton::draw. Unknown alignment.");
  }

  graphics->setFont(getFont());

  if (isPressed())
    graphics->drawText(getCaption(), textX + 1, textY + 1, getAlignment());
  else
    graphics->drawText(getCaption(), textX, textY, getAlignment());
}

// -----------------------------------------------------------------------

void GCNButton::init()
{
  setFrameSize(0);

  if (s_button.image == NULL) {
    s_button.image.reset(getThemeImage("button.png"));
    s_buttonhi.image.reset(getThemeImage("buttonhi.png"));
    s_buttonpress.image.reset(getThemeImage("buttonpress.png"));
    s_button_disabled.image.reset(getThemeImage("button_disabled.png"));

    static int xpos[] = {0, 9, 16, 26};
    static int ypos[] = {0, 4, 19, 24};
    s_button.setCoordinates(xpos, ypos);
    s_buttonhi.setCoordinates(xpos, ypos);
    s_buttonpress.setCoordinates(xpos, ypos);
    s_button_disabled.setCoordinates(xpos, ypos);
  }
}

// -----------------------------------------------------------------------

ImageRect& GCNButton::currentButtonImage()
{
  if (!isEnabled())
    return s_button_disabled;
  else if (isPressed())
    return s_buttonpress;
  else if (mHasMouse || isFocused())
    return s_buttonhi;
  else
    return s_button;
}
