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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_SDL_TEXTURE_H_
#define SRC_SYSTEMS_SDL_TEXTURE_H_

#include <SDL2/SDL_opengl.h>

#include <memory>
#include <string>

struct SDL_Surface;
class SDLSurface;
class GraphicsObject;

struct render_to_texture {};

// Contains one or more OpenGL textures, representing a single image,
// and provides a logical interface to working with them.

// TODO(erg): The entire Texture class's internals need to be transitioned
// to the Point and Rect classes.
class Texture {
 public:
  static void SetScreenSize(const Size& s);

  static int ScreenHeight();

 public:
  Texture(SDL_Surface* surface,
          int x,
          int y,
          int w,
          int h,
          unsigned int bytes_per_pixel,
          int byte_order,
          int byte_type);
  Texture(render_to_texture, int screen_width, int screen_height);
  ~Texture();

  // Uploads Rect(x, y, w, h) offset by (offset_x, offset_y) onto our texture
  // backend. We work like this so we can cut out dirty rectangles and upload
  // only what has changed.
  void reupload(SDL_Surface* surface,
                int offset_x,
                int offset_y,
                int x,
                int y,
                int w,
                int h,
                unsigned int bytes_per_pixel,
                int byte_order,
                int byte_type);

  int width() { return logical_width_; }
  int height() { return logical_height_; }
  GLuint textureId() { return texture_id_; }

  void RenderToScreenAsObject(const GraphicsObject& go,
                              const SDLSurface& surface,
                              const Rect& srcRect,
                              const Rect& dstRect,
                              int alpha);

  void RenderToScreen(const Rect& src, const Rect& dst, int opacity);

  void RenderToScreenAsColorMask(const Rect& src,
                                 const Rect& dst,
                                 const RGBAColour& rgba,
                                 int filter);

  void RenderToScreen(const Rect& src, const Rect& dst, const int opacity[4]);

 private:
  // Returns a shared buffer of at least size. This is not thread safe
  // or reenterant in the least; it is merely meant to prevent
  // allocations. This is the proper way to access s_upload_buffer,
  // since it will automatically reallocate it for you if it isn't
  // large enough.
  static char* uploadBuffer(unsigned int size);

  void render_to_screen_as_colour_mask_subtractive_glsl(const Rect& src,
                                                        const Rect& dst,
                                                        const RGBAColour& rgba);
  void render_to_screen_as_colour_mask_subtractive_fallback(
      const Rect& src,
      const Rect& dst,
      const RGBAColour& rgba);
  void render_to_screen_as_colour_mask_additive(const Rect& src,
                                                const Rect& dst,
                                                const RGBAColour& rgba);

  bool filterCoords(int& x1,
                    int& y1,
                    int& x2,
                    int& y2,
                    int& dx1,
                    int& dy1,
                    int& dx2,
                    int& dy2);

  int x_offset_;
  int y_offset_;

  int logical_width_;
  int logical_height_;

  int total_width_;
  int total_height_;

  unsigned int texture_width_;
  unsigned int texture_height_;

  GLuint texture_id_;

  GLuint back_texture_id_;

  // Is this texture upside down? (Because it's a screenshot, etc.)
  bool is_upside_down_;

  // Size of the screen. Used during color mask calculations.
  static unsigned int s_screen_width;
  static unsigned int s_screen_height;

  // The size of s_upload_buffer. Initialized to 0.
  static unsigned int s_upload_buffer_size;

  // To prevent new-ing in a loop, save the dynamically allocated
  // buffer used to upload data into.
  static std::unique_ptr<char[]> s_upload_buffer;
};

#endif  // SRC_SYSTEMS_SDL_TEXTURE_H_
