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

#include "platforms/gcn/gcn_true_type_font.h"

#include <guichan/color.hpp>
#include <guichan/exception.hpp>
#include <guichan/graphics.hpp>
#include <guichan/opengl/openglimage.hpp>
#include <memory>
#include <string>

#include "base/notification_service.h"
#include "systems/base/rect.h"
#include "systems/sdl/sdl_surface.h"

// -----------------------------------------------------------------------
// GCNTrueTypeFont
// -----------------------------------------------------------------------
GCNTrueTypeFont::GCNTrueTypeFont(const std::string& filename, int size)
    : image_cache_(125) {
  row_spacing_ = 0;
  glyph_spacing_ = 0;
  anti_alias_ = true;
  filename_ = filename;
  font_ = NULL;

  font_ = TTF_OpenFont(filename.c_str(), size);

  if (font_ == NULL) {
    throw GCN_EXCEPTION("GCNTrueTypeFont::GCNTrueTypeFont. " +
                        std::string(TTF_GetError()));
  }

  registrar_.Add(this,
                 NotificationType::FULLSCREEN_STATE_CHANGED,
                 NotificationService::AllSources());
}

GCNTrueTypeFont::~GCNTrueTypeFont() { TTF_CloseFont(font_); }

int GCNTrueTypeFont::getWidth(const std::string& text) const {
  int w, h;
  TTF_SizeUTF8(font_, text.c_str(), &w, &h);

  return w;
}

int GCNTrueTypeFont::getHeight() const {
  return TTF_FontHeight(font_) + row_spacing_;
}

void GCNTrueTypeFont::drawString(gcn::Graphics* graphics,
                                 const std::string& text,
                                 const int x,
                                 const int y) {
  if (text == "")
    return;

  // This is needed for drawing the Glyph in the middle if we have spacing
  int yoffset = getRowSpacing() / 2;

  gcn::Color col = graphics->getColor();
  std::string colstr;
  colstr += static_cast<char>(col.r);
  colstr += static_cast<char>(col.g);
  colstr += static_cast<char>(col.b);

  std::shared_ptr<gcn::OpenGLImage> image =
      image_cache_.fetch(make_pair(text, colstr));
  if (!image) {
    SDL_Color sdlCol;
    sdlCol.b = col.b;
    sdlCol.r = col.r;
    sdlCol.g = col.g;

    SDL_Surface* textSurface;
    if (anti_alias_) {
      textSurface = TTF_RenderUTF8_Blended(font_, text.c_str(), sdlCol);
    } else {
      textSurface = TTF_RenderUTF8_Solid(font_, text.c_str(), sdlCol);
    }

    SDL_LockSurface(textSurface);
    {
      image.reset(new gcn::OpenGLImage((const unsigned int*)textSurface->pixels,
                                       textSurface->w,
                                       textSurface->h));
    }
    SDL_UnlockSurface(textSurface);
    SDL_FreeSurface(textSurface);

    image_cache_.insert(make_pair(text, colstr), image);
  }

  graphics->drawImage(
      image.get(), 0, 0, x, y + yoffset, image->getWidth(), image->getHeight());
}

void GCNTrueTypeFont::setRowSpacing(int spacing) { row_spacing_ = spacing; }

int GCNTrueTypeFont::getRowSpacing() { return row_spacing_; }

void GCNTrueTypeFont::setGlyphSpacing(int spacing) { glyph_spacing_ = spacing; }

int GCNTrueTypeFont::getGlyphSpacing() { return glyph_spacing_; }

void GCNTrueTypeFont::setAntiAlias(bool anti_alias) {
  anti_alias_ = anti_alias;
}

bool GCNTrueTypeFont::isAntiAlias() { return anti_alias_; }

void GCNTrueTypeFont::Observe(NotificationType type,
                              const NotificationSource& source,
                              const NotificationDetails& details) {
  image_cache_.clear();
}
