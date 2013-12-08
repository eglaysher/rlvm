// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_SDL_SHADERS_H_
#define SRC_SYSTEMS_SDL_SHADERS_H_

#include <SDL2/SDL_opengl.h>

class GraphicsObject;

// Static state about shaders. We just leak them.
class Shaders {
 public:
  // Immediately frees all OpenGL resources associated with shaders.
  static void Reset();

  // Returns the shader for the subtractive color mask used in text boxes.
  static GLuint getColorMaskProgram();

  // Returns the parameters for the color mask program.
  static GLint getColorMaskUniformCurrentValues();
  static GLint getColorMaskUniformMask();

  // Returns the shader that implements tint/light/colour on objects.
  static GLuint GetObjectProgram();

  // Returns the parameters to the object program.
  static GLint GetObjectUniformImage();
  static GLint GetObjectUniformAlpha();

  // Set the colour/tint/light/mono/invert properties from |go|. (Individual
  // setters also exposed.)
  static void loadObjectUniformFromGraphicsObject(const GraphicsObject& go);
  static GLint GetObjectUniformColour();
  static GLint GetObjectUniformTint();
  static GLint GetObjectUniformLight();
  static GLint GetObjectUniformMono();
  static GLint GetObjectUniformInvert();

 private:
  // Compiles and links the text program in |shader| into a shader and program
  // object.
  static void buildShader(const char* shader, GLuint* program_object);

  static GLuint color_mask_program_object_id_;
  static GLuint color_mask_shader_object_id_;
  static GLint color_mask_current_values_;
  static GLint color_mask_mask_;

  static GLuint object_program_object_id_;
  static GLuint object_shader_object_id_;
  static GLint object_image_;
  static GLint object_colour_;
  static GLint object_tint_;
  static GLint object_light_;
  static GLint object_alpha_;
  static GLint object_mono_;
  static GLint object_invert_;
};

#endif  // SRC_SYSTEMS_SDL_SHADERS_H_
