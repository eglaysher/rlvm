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

/**
 * @file   Texture.hpp
 * @author Elliot Glaysher
 * @date   Sat Feb 17 20:21:59 2007
 *
 * @brief  Contains an OpenGL texture and some metadata.
 */

#ifndef SRC_SYSTEMS_SDL_TEXTURE_HPP_
#define SRC_SYSTEMS_SDL_TEXTURE_HPP_

#include <string>
#include <boost/scoped_array.hpp>
#include <SDL/SDL_opengl.h>

struct SDL_Surface;
class SDLSurface;
class GraphicsObject;

struct render_to_texture { };

/**
 * Contains one or more OpenGL textures, representing a single image,
 * and provides a logical interface to working with them.
 */
// POINT

// TODO: The entire Texture class's internals need to be transitioned
// to the Point and Rect classes.
class Texture {
 public:
  static void SetScreenSize(const Size& s);

 public:
  Texture(SDL_Surface* surface, int x, int y, int w, int h,
          unsigned int bytes_per_pixel, int byte_order, int byte_type);
  Texture(render_to_texture, int screen_width, int screen_height);
  ~Texture();

  void reupload(SDL_Surface* surface, int x, int y, int w, int h,
                unsigned int bytes_per_pixel, int byte_order, int byte_type);

  int width() { return logical_width_; }
  int height() { return logical_height_; }
  GLuint textureId() { return texture_id_; }

  void renderToScreenAsObject(
    const GraphicsObject& go,
    SDLSurface& surface,
    const Rect& srcRect, const Rect& dstRect,
    int alpha);

  void renderToScreen(const Rect& src, const Rect& dst, int opacity);

  void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter);

  void renderToScreen(const Rect& src, const Rect& dst,
                      const int opacity[4]);

  void rawRenderQuad(const int src_coords[8],
                     const int dest_coords[8],
                     const int opacity[4]);

  void buildShader();
  std::string getSubtractiveShaderString();

 private:
  // Returns a shared buffer of at least size. This is not thread safe
  // or reenterant in the least; it is merely meant to prevent
  // allocations. This is the proper way to access s_upload_buffer,
  // since it will automatically reallocate it for you if it isn't
  // large enough.
  static char* uploadBuffer(unsigned int size);

  void render_to_screen_as_colour_mask_subtractive_glsl(
    const Rect& src, const Rect& dst, const RGBAColour& rgba);
  void render_to_screen_as_colour_mask_subtractive_fallback(
    const Rect& src, const Rect& dst, const RGBAColour& rgba);
  void render_to_screen_as_colour_mask_additive(
    const Rect& src, const Rect& dst, const RGBAColour& rgba);

  bool filterCoords(int& x1, int& y1, int& x2, int& y2,
                    float& dx1, float& dy1, float& dx2, float& dy2);

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

  GLuint shader_object_id_;
  GLuint program_object_id_;

  /// Is this texture upside down? (Because it's a screenshot, et cetera.)
  bool is_upside_down_;

  // TODO: Dead code?
  static unsigned int s_screen_width;
  static unsigned int s_screen_height;

  // The size of s_upload_buffer. Initialized to 0.
  static unsigned int s_upload_buffer_size;

  // To prevent new-ing in a loop, save the dynamically allocated
  // buffer used to upload data into.
  static boost::scoped_array<char> s_upload_buffer;
};

#endif  // SRC_SYSTEMS_SDL_TEXTURE_HPP_
