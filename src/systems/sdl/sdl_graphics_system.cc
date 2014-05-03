// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "GL/glew.h"

#include "systems/sdl/sdl_graphics_system.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#if defined(__linux__)
#include <SDL2/SDL_image.h>
#endif

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <cstdio>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "base/notification_source.h"
#include "libreallive/gameexe.h"
#include "machine/rlmachine.h"
#include "systems/base/cgm_table.h"
#include "systems/base/colour.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_object.h"
#include "systems/base/mouse_cursor.h"
#include "systems/base/renderable.h"
#include "systems/base/system.h"
#include "systems/base/system_error.h"
#include "systems/base/text_system.h"
#include "systems/base/tone_curve.h"
#include "systems/sdl/sdl_colour_filter.h"
#include "systems/sdl/sdl_event_system.h"
#include "systems/sdl/sdl_render_to_texture_surface.h"
#include "systems/sdl/sdl_surface.h"
#include "systems/sdl/sdl_utils.h"
#include "systems/sdl/shaders.h"
#include "systems/sdl/texture.h"
#include "utilities/exception.h"
#include "utilities/graphics.h"
#include "utilities/lazy_array.h"
#include "utilities/string_utilities.h"
#include "xclannad/file.h"

// -----------------------------------------------------------------------
// Private Interface
// -----------------------------------------------------------------------

void SDLGraphicsSystem::SetCursor(int cursor) {
  GraphicsSystem::SetCursor(cursor);

  SDL_ShowCursor(ShouldUseCustomCursor() ? SDL_DISABLE : SDL_ENABLE);
}

void SDLGraphicsSystem::BeginFrame() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  DebugShowGLErrors();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  DebugShowGLErrors();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0,
          (GLdouble)screen_size().width(),
          (GLdouble)screen_size().height(),
          0.0,
          0.0,
          1.0);
  DebugShowGLErrors();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  DebugShowGLErrors();

  // Full screen shaking moves where the origin is.
  Point origin = GetScreenOrigin();
  glTranslatef(origin.x(), origin.y(), 0);
}

void SDLGraphicsSystem::MarkScreenAsDirty(GraphicsUpdateType type) {
  if (is_responsible_for_update() &&
      screen_update_mode() == SCREENUPDATEMODE_MANUAL && type == GUT_MOUSE_MOTION)
    redraw_last_frame_ = true;
  else
    GraphicsSystem::MarkScreenAsDirty(type);
}

void SDLGraphicsSystem::EndFrame() {
  FinalRenderers::iterator it = renderer_begin();
  FinalRenderers::iterator end = renderer_end();
  for (; it != end; ++it) {
    (*it)->Render(NULL);
  }

  if (screen_update_mode() == SCREENUPDATEMODE_MANUAL) {
    // Copy the area behind the cursor to the temporary buffer (drivers differ:
    // the contents of the back buffer is undefined after SDL_GL_SwapBuffers()
    // and I've just been lucky that the Intel i810 and whatever my Mac machine
    // has have been doing things that way.)
    glBindTexture(GL_TEXTURE_2D, screen_contents_texture_);
    glCopyTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        0,
                        0,
                        0,
                        0,
                        screen_size().width(),
                        screen_size().height());
    screen_contents_texture_valid_ = true;
  } else {
    screen_contents_texture_valid_ = false;
  }

  DrawCursor();

  // Swap the buffers
  glFlush();
  SDL_GL_SwapWindow(window_);
  ShowGLErrors();
}

void SDLGraphicsSystem::RedrawLastFrame() {
  // We won't redraw the screen between when the DrawManual() command is issued
  // by the bytecode and the first refresh() is called since we need a valid
  // copy of the screen to work with and we only snapshot the screen during
  // DrawManual() mode.
  if (screen_contents_texture_valid_) {
    // Redraw the screen
    glBindTexture(GL_TEXTURE_2D, screen_contents_texture_);
    glBegin(GL_QUADS);
    {
      int dx1 = 0;
      int dx2 = screen_size().width();
      int dy1 = 0;
      int dy2 = screen_size().height();

      float x_cord = dx2 / float(screen_tex_width_);
      float y_cord = dy2 / float(screen_tex_height_);

      glColor4ub(255, 255, 255, 255);
      glTexCoord2f(0, y_cord);
      glVertex2i(dx1, dy1);
      glTexCoord2f(x_cord, y_cord);
      glVertex2i(dx2, dy1);
      glTexCoord2f(x_cord, 0);
      glVertex2i(dx2, dy2);
      glTexCoord2f(0, 0);
      glVertex2i(dx1, dy2);
    }
    glEnd();

    DrawCursor();

    glFlush();

    // Swap the buffers
    SDL_GL_SwapWindow(window_);
    ShowGLErrors();
  }
}

void SDLGraphicsSystem::DrawCursor() {
  if (ShouldUseCustomCursor()) {
    std::shared_ptr<MouseCursor> cursor;
    if (static_cast<SDLEventSystem&>(system().event()).mouse_inside_window())
      cursor = GetCurrentCursor();
    if (cursor) {
      Point hotspot = cursor_pos();
      cursor->RenderHotspotAt(hotspot);
    }
  }
}

std::shared_ptr<Surface> SDLGraphicsSystem::EndFrameToSurface() {
  return std::shared_ptr<Surface>(
      new SDLRenderToTextureSurface(this, screen_size()));
}

// -----------------------------------------------------------------------
// Public Interface
// -----------------------------------------------------------------------

SDLGraphicsSystem::SDLGraphicsSystem(System& system, Gameexe& gameexe)
    : GraphicsSystem(system, gameexe),
      redraw_last_frame_(false),
      display_data_in_titlebar_(false),
      time_of_last_titlebar_update_(0),
      last_seen_number_(0),
      last_line_number_(0),
      screen_contents_texture_valid_(false),
      screen_tex_width_(0),
      screen_tex_height_(0) {
  haikei_.reset(new SDLSurface(this));
  for (int i = 0; i < 16; ++i)
    display_contexts_[i].reset(new SDLSurface(this));

  SetScreenSize(GetScreenSize(gameexe));
  Texture::SetScreenSize(screen_size());

  // Grab the caption
  std::string cp932caption = gameexe("CAPTION").ToString();
  int name_enc = gameexe("NAME_ENC").ToInt(0);
  caption_title_ = cp932toUTF8(cp932caption, name_enc);

  SetupVideo();

  // Now we allocate the first two display contexts with equal size to
  // the display
  display_contexts_[0]->allocate(screen_size(), true);
  display_contexts_[1]->allocate(screen_size());

  SetWindowTitle();

#if defined(__linux__)
  // We only set the icon on linux because OSX will use the icns file
  // automatically and this doesn't look too awesome.
  SDL_Surface* icon = IMG_Load("/usr/share/icons/hicolor/48x48/apps/rlvm.png");
  if (icon) {
    SDL_SetColorKey(icon, SDL_TRUE, SDL_MapRGB(icon->format, 0, 0, 0));
    SDL_SetWindowIcon(window_, icon);
    SDL_FreeSurface(icon);
  }
#endif

  // When debug is set, display trace data in the titlebar
  if (gameexe("MEMORY").Exists()) {
    display_data_in_titlebar_ = true;
  }

  SDL_ShowCursor(ShouldUseCustomCursor() ? SDL_DISABLE : SDL_ENABLE);

  registrar_.Add(this,
                 NotificationType::FULLSCREEN_STATE_CHANGED,
                 Source<GraphicsSystem>(static_cast<GraphicsSystem*>(this)));
}

void SDLGraphicsSystem::SetupVideo() {
  // // Let's get some video information.
  // const SDL_VideoInfo* info = SDL_GetVideoInfo();

  // if (!info) {
  //   std::ostringstream ss;
  //   ss << "Video query failed: " << SDL_GetError();
  //   throw SystemError(ss.str());
  // }

  // int bpp = info->vfmt->BitsPerPixel;

  // Sets up OpenGL double buffering
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  window_ = SDL_CreateWindow("rlvm",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             screen_size().width(),
                             screen_size().height(),
                             SDL_WINDOW_OPENGL);
  gl_context_ = SDL_GL_CreateContext(window_);

  // Initialize glew
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::ostringstream oss;
    oss << "Failed to initialize GLEW: " << glewGetErrorString(err);
    throw SystemError(oss.str());
  }

  glEnable(GL_TEXTURE_2D);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Enable Texture Mapping ( NEW )
  glEnable(GL_TEXTURE_2D);

  // Enable smooth shading
  glShadeModel(GL_SMOOTH);

  // Set the background black
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // Depth buffer setup
  glClearDepth(1.0f);

  // Enables Depth Testing
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);

  // The Type Of Depth Test To Do
  glDepthFunc(GL_LEQUAL);

  // Really Nice Perspective Calculations
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  // Full Brightness, 50% Alpha ( NEW )
  glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

  // Create a small 32x32 texture for storing what's behind the mouse
  // cursor.
  glGenTextures(1, &screen_contents_texture_);
  glBindTexture(GL_TEXTURE_2D, screen_contents_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  screen_tex_width_ = SafeSize(screen_size().width());
  screen_tex_height_ = SafeSize(screen_size().height());
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               screen_tex_width_,
               screen_tex_height_,
               0,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               NULL);

  ShowGLErrors();
}

SDLGraphicsSystem::~SDLGraphicsSystem() {
  SDL_GL_DeleteContext(gl_context_);
  SDL_DestroyWindow(window_);
}

void SDLGraphicsSystem::ExecuteGraphicsSystem(RLMachine& machine) {
  // For now, nothing, but later, we need to put all code each cycle
  // here.
  if (is_responsible_for_update() && screen_needs_refresh()) {
    Refresh(NULL);
    OnScreenRefreshed();
    redraw_last_frame_ = false;
  } else if (is_responsible_for_update() && redraw_last_frame_) {
    RedrawLastFrame();
    redraw_last_frame_ = false;
  }

  // Update the seen.
  int current_time = machine.system().event().GetTicks();
  if ((current_time - time_of_last_titlebar_update_) > 60) {
    time_of_last_titlebar_update_ = current_time;

    if (machine.SceneNumber() != last_seen_number_ ||
        machine.line_number() != last_line_number_) {
      last_seen_number_ = machine.SceneNumber();
      last_line_number_ = machine.line_number();
      SetWindowTitle();
    }
  }

  GraphicsSystem::ExecuteGraphicsSystem(machine);
}

void SDLGraphicsSystem::SetWindowTitle() {
  std::ostringstream oss;
  oss << caption_title_;

  if (should_display_subtitle() && subtitle_ != "") {
    oss << ": " << subtitle_;
  }

  if (display_data_in_titlebar_) {
    oss << " - (SEEN" << last_seen_number_ << ")(Line " << last_line_number_
        << ")";
  }

  // PulseAudio allocates a string each time we set the title. Make sure we
  // don't do this unnecessarily.
  std::string new_caption = oss.str();
  if (new_caption != currently_set_title_) {
    SDL_SetWindowTitle(window_, new_caption.c_str());
    currently_set_title_ = new_caption;
  }
}

void SDLGraphicsSystem::Observe(NotificationType type,
                                const NotificationSource& source,
                                const NotificationDetails& details) {
  Shaders::Reset();
}

void SDLGraphicsSystem::SetWindowSubtitle(const std::string& cp932str,
                                          int text_encoding) {
  // TODO(erg): Still not restoring title correctly!
  subtitle_ = cp932toUTF8(cp932str, text_encoding);

  GraphicsSystem::SetWindowSubtitle(cp932str, text_encoding);
}

void SDLGraphicsSystem::SetScreenMode(const int in) {
  GraphicsSystem::SetScreenMode(in);

  SetupVideo();
}

void SDLGraphicsSystem::AllocateDC(int dc, Size size) {
  if (dc >= 16) {
    std::ostringstream ss;
    ss << "Invalid DC number \"" << dc
       << "\" in SDLGraphicsSystem::allocate_dc";
    throw rlvm::Exception(ss.str());
  }

  // We can't reallocate the screen!
  if (dc == 0)
    throw rlvm::Exception("Attempting to reallocate DC 0!");

  // DC 1 is a special case and must always be at least the size of
  // the screen.
  if (dc == 1) {
    SDL_Surface* dc0 = *(display_contexts_[0]);
    if (size.width() < dc0->w)
      size.set_width(dc0->w);
    if (size.height() < dc0->h)
      size.set_height(dc0->h);
  }

  // Allocate a new obj.
  display_contexts_[dc]->allocate(size);
}

void SDLGraphicsSystem::SetMinimumSizeForDC(int dc, Size size) {
  if (display_contexts_[dc] == NULL || !display_contexts_[dc]->allocated()) {
    AllocateDC(dc, size);
  } else {
    Size current = display_contexts_[dc]->GetSize();
    if (current.width() < size.width() || current.height() < size.height()) {
      // Make a new surface of the maximum size.
      Size maxSize = current.SizeUnion(size);

      std::shared_ptr<SDLSurface> newdc(new SDLSurface(this));
      newdc->allocate(maxSize);

      display_contexts_[dc]->BlitToSurface(
          *newdc, display_contexts_[dc]->GetRect(),
          display_contexts_[dc]->GetRect());

      display_contexts_[dc] = newdc;
    }
  }
}

void SDLGraphicsSystem::FreeDC(int dc) {
  if (dc == 0) {
    throw rlvm::Exception("Attempt to deallocate DC[0]");
  } else if (dc == 1) {
    // DC[1] never gets freed; it only gets blanked
    GetDC(1)->Fill(RGBAColour::Black());
  } else {
    display_contexts_[dc]->deallocate();
  }
}

void SDLGraphicsSystem::VerifySurfaceExists(int dc, const std::string& caller) {
  if (dc >= 16) {
    std::ostringstream ss;
    ss << "Invalid DC number (" << dc << ") in " << caller;
    throw rlvm::Exception(ss.str());
  }

  if (display_contexts_[dc] == NULL) {
    std::ostringstream ss;
    ss << "Parameter DC[" << dc << "] not allocated in " << caller;
    throw rlvm::Exception(ss.str());
  }
}

// -----------------------------------------------------------------------

typedef enum { NO_MASK, ALPHA_MASK, COLOR_MASK } MaskType;

// Note to self: These describe the byte order IN THE RAW G00 DATA!
// These should NOT be switched to native byte order.
#define DefaultRmask 0xff0000
#define DefaultGmask 0xff00
#define DefaultBmask 0xff
#define DefaultAmask 0xff000000
#define DefaultBpp 32

static SDL_Surface* newSurfaceFromRGBAData(int w,
                                           int h,
                                           char* data,
                                           MaskType with_mask) {
  int amask = (with_mask == ALPHA_MASK) ? DefaultAmask : 0;
  SDL_Surface* tmp = SDL_CreateRGBSurfaceFrom(data,
                                              w,
                                              h,
                                              DefaultBpp,
                                              w * 4,
                                              DefaultRmask,
                                              DefaultGmask,
                                              DefaultBmask,
                                              amask);

  // We now need to convert this surface to a format suitable for use across
  // the rest of the program. We can't (regretfully) rely on
  // SDL_DisplayFormat[Alpha] to decide on a format that we can send to OpenGL
  // (see some Intel macs) so use convert surface to a pixel order our data
  // correctly while still using the appropriate alpha flags. So use the above
  // format with only the flags that would have been set by
  // SDL_DisplayFormat[Alpha].

  // TODO(sdl2): port
  //  Uint32 flags;
  // if (with_mask == ALPHA_MASK) {
  //   //flags = tmp->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
  //   std::cerr << "with ALPHA_MASK" << std::endl;
  // } else {
  //   //flags = tmp->flags & (SDL_SRCCOLORKEY | SDL_SRCALPHA | SDL_RLEACCELOK);
  //   std::cerr << "without ALPHA_MASK" << std::endl;
  // }

  SDL_Surface* surf = SDL_ConvertSurface(tmp, tmp->format, 0);
  SDL_FreeSurface(tmp);
  return surf;
}

// Helper function for load_surface_from_file; invoked in a stl loop.
static SDLSurface::GrpRect xclannadRegionToGrpRect(
    const GRPCONV::REGION& region) {
  SDLSurface::GrpRect rect;
  rect.rect =
      Rect(Point(region.x1, region.y1), Point(region.x2 + 1, region.y2 + 1));
  rect.originX = region.origin_x;
  rect.originY = region.origin_y;
  return rect;
}

std::shared_ptr<const Surface> SDLGraphicsSystem::LoadSurfaceFromFile(
    const std::string& short_filename) {
  boost::filesystem::path filename =
      system().FindFile(short_filename, IMAGE_FILETYPES);
  if (filename.empty()) {
    std::ostringstream oss;
    oss << "Could not find image file \"" << short_filename << "\".";
    throw rlvm::Exception(oss.str());
  }

  // Glue code to allow my stuff to work with Jagarl's loader
  FILE* file = fopen(filename.string().c_str(), "rb");
  if (!file) {
    std::ostringstream oss;
    oss << "Could not open file: " << filename;
    throw rlvm::Exception(oss.str());
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  std::unique_ptr<char[]> d(new char[size + 1]);
  fseek(file, 0, SEEK_SET);
  fread(d.get(), size, 1, file);
  fclose(file);

  std::unique_ptr<GRPCONV> conv(
      GRPCONV::AssignConverter(d.get(), size, "???"));
  if (conv == 0) {
    throw SystemError("Failure in GRPCONV.");
  }
  // do not free until SDL_FreeSurface() is called on the surface using it
  char* mem = (char*)malloc(conv->Width() * conv->Height() * 4 + 1024);
  SDL_Surface* s = 0;
  if (conv->Read(mem)) {
    MaskType is_mask = conv->IsMask() ? ALPHA_MASK : NO_MASK;
    if (is_mask == ALPHA_MASK) {
      int len = conv->Width() * conv->Height();
      unsigned int* d = (unsigned int*)mem;
      int i;
      for (i = 0; i < len; i++) {
        if ((*d & 0xff000000) != 0xff000000)
          break;
        d++;
      }
      if (i == len) {
        is_mask = NO_MASK;
      }
    }

    s = newSurfaceFromRGBAData(conv->Width(), conv->Height(), mem, is_mask);
  }
  free(mem);

  // Grab the Type-2 information out of the converter or create one
  // default region if none exist
  std::vector<SDLSurface::GrpRect> region_table;
  if (conv->region_table.size()) {
    std::transform(conv->region_table.begin(),
                   conv->region_table.end(),
                   std::back_inserter(region_table),
                   xclannadRegionToGrpRect);
  } else {
    SDLSurface::GrpRect rect;
    rect.rect = Rect(Point(0, 0), Size(conv->Width(), conv->Height()));
    rect.originX = 0;
    rect.originY = 0;
    region_table.push_back(rect);
  }

  std::shared_ptr<Surface> surface_to_ret(
      new SDLSurface(this, s, region_table));
  // handle tone curve effect loading
  if (short_filename.find("?") != short_filename.npos) {
    std::string effect_no_str =
        short_filename.substr(short_filename.find("?") + 1);
    int effect_no = std::stoi(effect_no_str);
    // the effect number is an index that goes from 10 to GetEffectCount() * 10,
    // so keep that in mind here
    if ((effect_no / 10) > globals().tone_curves.GetEffectCount() ||
        effect_no < 10) {
      std::ostringstream oss;
      oss << "Tone curve index " << effect_no << " is invalid.";
      throw rlvm::Exception(oss.str());
    }
    surface_to_ret.get()->ToneCurve(
        globals().tone_curves.GetEffect(effect_no / 10 - 1),
        Rect(Point(0, 0), Size(conv->Width(), conv->Height())));
  }

  return surface_to_ret;
}

std::shared_ptr<Surface> SDLGraphicsSystem::GetHaikei() {
  if (haikei_->rawSurface() == NULL) {
    haikei_->allocate(screen_size(), true);
  }

  return haikei_;
}

std::shared_ptr<Surface> SDLGraphicsSystem::GetDC(int dc) {
  VerifySurfaceExists(dc, "SDLGraphicsSystem::get_dc");

  // If requesting a DC that doesn't exist, allocate it first.
  if (display_contexts_[dc]->rawSurface() == NULL)
    AllocateDC(dc, display_contexts_[0]->GetSize());

  return display_contexts_[dc];
}

std::shared_ptr<Surface> SDLGraphicsSystem::BuildSurface(const Size& size) {
  return std::shared_ptr<Surface>(new SDLSurface(this, size));
}

ColourFilter* SDLGraphicsSystem::BuildColourFiller() {
  return new SDLColourFilter();
}

void SDLGraphicsSystem::Reset() {
  last_seen_number_ = 0;
  last_line_number_ = 0;

  GraphicsSystem::Reset();
}
