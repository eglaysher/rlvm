// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef __SDLRenderToTextureSurface_hpp__
#define __SDLRenderToTextureSurface_hpp__

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "Systems/Base/Surface.hpp"

class Texture;

/** 
 * Fake SDLSurface used to 
 */
class SDLRenderToTextureSurface : public Surface, public boost::noncopyable
{
private:
  /// The SDLTexture which wraps one or more OpenGL textures
  boost::scoped_ptr<Texture> m_texture;

public:
  SDLRenderToTextureSurface(int w, int h);
  ~SDLRenderToTextureSurface();

  virtual void dump();

  /// Blits to another surface
  virtual void blitToSurface(Surface& surface, 
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                             int alpha = 255, bool useSrcAlpha = true);

  virtual void renderToScreen(
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     int alpha = 255);

  virtual void renderToScreen(
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     const int opacity[4]);

  virtual void rawRenderQuad(const int srcCoords[8], 
                             const int destCoords[8],
                             const int opacity[4]);

  virtual void fill(int r, int g, int b, int alpha);
  virtual void fill(int r, int g, int b, int alpha, int x, int y, 
                    int width, int height);

  virtual int width() const;
  virtual int height() const;

  virtual Surface* clone() const;
};


#endif 
