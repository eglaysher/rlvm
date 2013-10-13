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

#include "GL/glew.h"

#include <iostream>

#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Shaders.hpp"

using namespace std;

namespace {

const char kColorMaskShader[] =
    "uniform sampler2D current_values, mask;"
    ""
    "void main()"
    "{"
    "vec4 bg_colour = texture2D(current_values, gl_TexCoord[0].st);"
    "vec4 mask_vector = texture2D(mask, gl_TexCoord[1].st);"
    "float mask_colour = clamp(mask_vector.a * gl_Color.a, 0.0, 1.0);"
    "gl_FragColor = clamp(bg_colour - mask_colour + gl_Color * mask_colour, "
    "                     0.0, 1.0);"
    "}";

const char kObjectShader[] =
    "uniform sampler2D image;\n"
    "uniform vec4 colour;\n"
    "uniform float mono;\n"
    "uniform float invert;\n"
    "uniform float light;\n"
    "uniform vec3 tint;\n"
    "uniform float alpha;\n"
    "\n"
    "void tinter(in float pixel_val, in float tint_val, out float mixed) {\n"
    "  if (tint_val > 0.0) {\n"
    "    mixed = pixel_val + tint_val - (pixel_val * tint_val);\n"
    "  } else if (tint_val < 0.0) {\n"
    "    mixed = pixel_val * abs(tint_val);\n"
    "  } else {\n"
    "    mixed = pixel_val;\n"
    "  }\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  vec4 pixel = texture2D(image, gl_TexCoord[0].st);\n"
    "\n"
    "  // The colour is blended directly with the incoming pixel value.\n"
    "  vec3 coloured = mix(pixel.rgb, colour.rgb, colour.a);\n"
    "  pixel = vec4(coloured.r, coloured.g, coloured.b, pixel.a);\n"
    "\n"
    "  if (mono > 0.0) {\n"
    "    // NTSC grayscale\n"
    "    float gray = dot(pixel.rgb, vec3(0.299, 0.587, 0.114));\n"
    "    vec3 mixed = mix(pixel.rgb, vec3(gray, gray, gray), mono);\n"
    "    pixel = vec4(mixed.r, mixed.g, mixed.b, pixel.a);\n"
    "  }\n"
    "\n"
    "  if (invert > 0.0) {\n"
    "    vec3 inverted = vec3(1.0, 1.0, 1.0) - pixel.rgb;\n"
    "    vec3 mixed = mix(pixel.rgb, inverted, invert);\n"
    "    pixel = vec4(mixed.r, mixed.g, mixed.b, pixel.a);\n"
    "  }\n"
    "\n"
    "  float out_r, out_g, out_b;\n"
    "  tinter(pixel.r, light, out_r);\n"
    "  tinter(pixel.g, light, out_g);\n"
    "  tinter(pixel.b, light, out_b);\n"
    "  pixel = vec4(out_r, out_g, out_b, pixel.a);\n"
    "\n"
    "  tinter(pixel.r, tint.r, out_r);\n"
    "  tinter(pixel.g, tint.g, out_g);\n"
    "  tinter(pixel.b, tint.b, out_b);\n"
    "  pixel = vec4(out_r, out_g, out_b, pixel.a);\n"
    "\n"
    "  // We're responsible for doing the main alpha blending, too.\n"
    "  pixel.a = pixel.a * alpha;\n"
    "  gl_FragColor = pixel;\n"
    "}\n";

} // namespace

GLuint Shaders::color_mask_program_object_id_ = 0;
GLint Shaders::color_mask_current_values_ = 0;
GLint Shaders::color_mask_mask_ = 0;

GLuint Shaders::object_program_object_id_ = 0;
GLint Shaders::object_image_ = 0;
GLint Shaders::object_colour_ = 0;
GLint Shaders::object_tint_ = 0;
GLint Shaders::object_light_ = 0;
GLint Shaders::object_alpha_ = 0;
GLint Shaders::object_mono_ = 0;
GLint Shaders::object_invert_ = 0;

// static
GLuint Shaders::getColorMaskProgram() {
  if (color_mask_program_object_id_ == 0) {
    buildShader(kColorMaskShader,
                &color_mask_program_object_id_);
  }

  return color_mask_program_object_id_;
}

GLint Shaders::getColorMaskUniformCurrentValues() {
  if (color_mask_current_values_ == 0) {
    color_mask_current_values_ = glGetUniformLocationARB(
        getColorMaskProgram(), "current_values");
    if (color_mask_current_values_ == -1)
      throw SystemError("Bad uniform value: current_values");
  }

  return color_mask_current_values_;
}

GLint Shaders::getColorMaskUniformMask() {
  if (color_mask_mask_ == 0) {
    color_mask_mask_ = glGetUniformLocationARB(
        getColorMaskProgram(), "mask");
    if (color_mask_mask_ == -1)
      throw SystemError("Bad uniform value: mask");
  }

  return color_mask_mask_;
}

GLuint Shaders::getObjectProgram() {
  if (object_program_object_id_ == 0) {
    buildShader(kObjectShader,
                &object_program_object_id_);
  }

  return object_program_object_id_;
}

GLint Shaders::getObjectUniformImage() {
  if (object_image_ == 0) {
    object_image_ = glGetUniformLocationARB(
        getObjectProgram(), "image");
    if (object_image_ == -1)
      throw SystemError("Bad uniform value: image");
  }

  return object_image_;
}

void Shaders::loadObjectUniformFromGraphicsObject(const GraphicsObject& go) {
  RGBAColour colour = go.colour();
  glUniform4fARB(Shaders::getObjectUniformColour(),
                 colour.r_float(), colour.g_float(),
                 colour.b_float(), colour.a_float());

  RGBColour tint = go.tint();
  glUniform3fARB(Shaders::getObjectUniformTint(),
                 tint.r_float(), tint.g_float(), tint.b_float());

  glUniform1fARB(Shaders::getObjectUniformLight(),
                 go.light() / 255.0f);

  glUniform1fARB(Shaders::getObjectUniformMono(),
                 go.mono() / 255.0f);

  glUniform1fARB(Shaders::getObjectUniformInvert(),
                 go.invert() / 255.0f);
}

GLint Shaders::getObjectUniformColour() {
  if (object_colour_ == 0) {
    object_colour_ = glGetUniformLocationARB(
        getObjectProgram(), "colour");
    if (object_colour_ == -1)
      throw SystemError("Bad uniform value: colour");
  }

  return object_colour_;
}

GLint Shaders::getObjectUniformTint() {
  if (object_tint_ == 0) {
    object_tint_ = glGetUniformLocationARB(
        getObjectProgram(), "tint");
    if (object_tint_ == -1)
      throw SystemError("Bad uniform value: tint");
  }

  return object_tint_;
}

GLint Shaders::getObjectUniformLight() {
  if (object_light_ == 0) {
    object_light_ = glGetUniformLocationARB(
        getObjectProgram(), "light");
    if (object_light_ == -1)
      throw SystemError("Bad uniform value: light");
  }

  return object_light_;
}

GLint Shaders::getObjectUniformAlpha() {
  if (object_alpha_ == 0) {
    object_alpha_ = glGetUniformLocationARB(
        getObjectProgram(), "alpha");
    if (object_alpha_ == -1)
      throw SystemError("Bad uniform value: alpha");
  }

  return object_alpha_;
}

GLint Shaders::getObjectUniformMono() {
  if (object_mono_ == 0) {
    object_mono_ = glGetUniformLocationARB(
        getObjectProgram(), "mono");
    if (object_mono_ == -1)
      throw SystemError("Bad uniform value: mono");
  }

  return object_mono_;
}

GLint Shaders::getObjectUniformInvert() {
  if (object_invert_ == 0) {
    object_invert_ = glGetUniformLocationARB(
        getObjectProgram(), "invert");
    if (object_invert_ == -1)
      throw SystemError("Bad uniform value: invert");
  }

  return object_invert_;
}

// static
void Shaders::buildShader(const char* shader,
                          GLuint* program_object) {
  GLuint shader_object = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
  DebugShowGLErrors();

  glShaderSourceARB(shader_object, 1, &shader, NULL);
  DebugShowGLErrors();

  glCompileShaderARB(shader_object);
  DebugShowGLErrors();

#ifndef NDEBUG
  GLint blen = 0;
  GLsizei slen = 0;
  glGetShaderiv(shader_object, GL_INFO_LOG_LENGTH , &blen);
  if (blen > 1) {
    GLchar* compiler_log = new GLchar[blen];
    glGetInfoLogARB(shader_object, blen, &slen, compiler_log);
    cout << "compiler_log: " << endl << compiler_log << endl;
    delete [] compiler_log;
  }
#endif

  *program_object = glCreateProgramObjectARB();
  glAttachObjectARB(*program_object, shader_object);
  DebugShowGLErrors();

  glLinkProgramARB(*program_object);
  glDeleteObjectARB(shader_object);
  ShowGLErrors();
}
