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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   SDLGraphicsSystem.cpp
 * @brief  Exposed interface for the SDL Graphics system.
 * @author Elliot Glaysher
 * @date   Fri Oct  6 13:34:08 2006
 *
 */

#include "GL/glew.h"

#include "Systems/SDL/SDLGraphicsSystem.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/CGMTable.hpp"
#include "Systems/Base/Colour.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/MouseCursor.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/Platform.hpp"
#include "Systems/Base/Renderable.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/SDL/SDLEventSystem.hpp"
#include "Systems/SDL/SDLRenderToTextureSurface.hpp"
#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"
#include "Utilities/Graphics.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/gameexe.h"

#include "file.h"
#include "Utilities/LazyArray.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdio>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost;
using namespace std;
using namespace libReallive;

// -----------------------------------------------------------------------
// Private Interface
// -----------------------------------------------------------------------

void SDLGraphicsSystem::setCursor(int cursor) {
  GraphicsSystem::setCursor(cursor);

  SDL_ShowCursor(useCustomCursor() ? SDL_DISABLE : SDL_ENABLE);
}

void SDLGraphicsSystem::beginFrame()
{
  glClearColor(0,0,0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ShowGLErrors();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  ShowGLErrors();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (GLdouble)screenSize().width(), (GLdouble)screenSize().height(),
          0.0, 0.0, 1.0);
  ShowGLErrors();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  ShowGLErrors();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::markScreenAsDirty(GraphicsUpdateType type)
{
  if (isResponsibleForUpdate() &&
      screenUpdateMode() == SCREENUPDATEMODE_MANUAL &&
      type == GUT_MOUSE_MOTION)
    redraw_last_frame_ = true;
  else
    GraphicsSystem::markScreenAsDirty(type);
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLGraphicsSystem::renderToSurfaceWithBg(
  boost::shared_ptr<Surface> bg)
{
  beginFrame();

  // Display DC0
  bg->renderToScreen(screenRect(), screenRect(), 255);

  renderObjects(NULL);

  // Render text
  if(!interfaceHidden())
    system().text().render(NULL);

  return endFrameToSurface();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::endFrame() {
  FinalRenderers::iterator it = renderer_begin();
  FinalRenderers::iterator end = renderer_end();
  for (; it != end; ++it) {
    (*it)->render(NULL);
  }

  if (screenUpdateMode() == SCREENUPDATEMODE_MANUAL) {
    // Copy the area behind the cursor to the temporary buffer (drivers differ:
    // the contents of the back buffer is undefined after SDL_GL_SwapBuffers()
    // and I've just been lucky that the Intel i810 and whatever my Mac machine
    // has have been doing things that way.)
    glBindTexture(GL_TEXTURE_2D, screen_contents_texture_);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0,
                        screenSize().width(), screenSize().height());
    screen_contents_texture_valid_ = true;
  } else {
    screen_contents_texture_valid_ = false;
  }

  drawCursor();

  // Swap the buffers
  glFlush();
  SDL_GL_SwapBuffers();
  ShowGLErrors();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::redrawLastFrame()
{
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
      int dx2 = screenSize().width();
      int dy1 = 0;
      int dy2 = screenSize().height();

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

    drawCursor();

    glFlush();

    // Swap the buffers
    SDL_GL_SwapBuffers();
    ShowGLErrors();
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::drawCursor()
{
  boost::shared_ptr<MouseCursor> cursor;
  if(static_cast<SDLEventSystem&>(system().event()).mouseInsideWindow())
    cursor = currentCursor();
  if(cursor)
  {
    Point hotspot = cursorPos();
    Point render_loc = cursor->getTopLeftForHotspotAt(hotspot);
    cursor->renderHotspotAt(hotspot);
  }
}

// -----------------------------------------------------------------------

shared_ptr<Surface> SDLGraphicsSystem::endFrameToSurface()
{
  return shared_ptr<Surface>(new SDLRenderToTextureSurface(screenSize()));
}

// -----------------------------------------------------------------------
// Public Interface
// -----------------------------------------------------------------------

/**
 *
 * @pre SDL is initialized.
 */
SDLGraphicsSystem::SDLGraphicsSystem(System& system, Gameexe& gameexe)
  : GraphicsSystem(system, gameexe), redraw_last_frame_(false),
    display_data_in_titlebar_(false), time_of_last_titlebar_update_(0),
    last_seen_number_(0), last_line_number_(0),
    screen_contents_texture_valid_(false),
    screen_tex_width_(0),
    screen_tex_height_(0),
    image_cache_(20)
{
  for(int i = 0; i < 16; ++i)
    display_contexts_[i].reset(new SDLSurface);

  // Let's get some video information.
  const SDL_VideoInfo* info = SDL_GetVideoInfo( );

  if( !info ) {
    ostringstream ss;
    ss << "Video query failed: " << SDL_GetError();
    throw SystemError(ss.str());
  }

  setScreenSize(getScreenSize(gameexe));
  Texture::SetScreenSize(screenSize());

  int bpp = info->vfmt->BitsPerPixel;

  /// Grab the caption
  std::string cp932caption = gameexe("CAPTION").to_string();
  int name_enc = gameexe("NAME_ENC").to_int(0);
  caption_title_ = cp932toUTF8(cp932caption, name_enc);

  /* the flags to pass to SDL_SetVideoMode */
  int video_flags;
  video_flags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
  video_flags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
  video_flags |= SDL_HWPALETTE;       /* Store the palette in hardware */

  /* This checks to see if surfaces can be stored in memory */
  if ( info->hw_available )
    video_flags |= SDL_HWSURFACE;
  else
    video_flags |= SDL_SWSURFACE;

  /* This checks if hardware blits can be done */
  if ( info->blit_hw )
    video_flags |= SDL_HWACCEL;

  /* Sets up OpenGL double buffering */
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  // Set the video mode
  if((screen_ = SDL_SetVideoMode(
        screenSize().width(), screenSize().height(), bpp, video_flags)) == 0 )
  {
    // This could happen for a variety of reasons,
    // including DISPLAY not being set, the specified
    // resolution not being available, etc.
    ostringstream ss;
    ss << "Video mode set failed: " << SDL_GetError();
    throw SystemError(ss.str());
  }

  // Initialize glew
  GLenum err = glewInit();
  if(GLEW_OK != err)
  {
    ostringstream oss;
    oss << "Failed to initialize GLEW: " << glewGetErrorString(err);
    throw SystemError(oss.str());
  }

  glEnable(GL_TEXTURE_2D);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  /* Enable Texture Mapping ( NEW ) */
  glEnable( GL_TEXTURE_2D );

  /* Enable smooth shading */
  glShadeModel( GL_SMOOTH );

  /* Set the background black */
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

  /* Depth buffer setup */
  glClearDepth( 1.0f );

  /* Enables Depth Testing */
  glEnable( GL_DEPTH_TEST );

  glEnable( GL_BLEND);

  /* The Type Of Depth Test To Do */
  glDepthFunc( GL_LEQUAL );

  /* Really Nice Perspective Calculations */
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

  /* Full Brightness, 50% Alpha ( NEW ) */
  glColor4f( 1.0f, 1.0f, 1.0f, 0.5f);

  // Now we allocate the first two display contexts with equal size to
  // the display
  display_contexts_[0]->allocate(screenSize(), this);
  display_contexts_[1]->allocate(screenSize());

  // Create a small 32x32 texture for storing what's behind the mouse
  // cursor.
  glGenTextures(1, &screen_contents_texture_);
  glBindTexture(GL_TEXTURE_2D, screen_contents_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  screen_tex_width_ = SafeSize(screenSize().width());
  screen_tex_height_ = SafeSize(screenSize().height());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               screen_tex_width_, screen_tex_height_, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  ShowGLErrors();

  setWindowTitle();

  // When debug is set, display trace data in the titlebar
  if(gameexe("MEMORY").exists())
  {
    display_data_in_titlebar_ = true;
  }

  SDL_ShowCursor(useCustomCursor() ? SDL_DISABLE : SDL_ENABLE);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::executeGraphicsSystem(RLMachine& machine)
{
  // For now, nothing, but later, we need to put all code each cycle
  // here.
  if (isResponsibleForUpdate() && screenNeedsRefresh()) {
    refresh(NULL);
    screenRefreshed();
    redraw_last_frame_ = false;
  } else if (isResponsibleForUpdate() && redraw_last_frame_) {
    redrawLastFrame();
    redraw_last_frame_ = false;
  }

  // Check to see if any of the graphics objects are reporting that
  // they want to force a redraw
  for_each(foregroundObjects().allocated_begin(),
           foregroundObjects().allocated_end(),
           bind(&GraphicsObject::execute, _1));

  // Update the seen.
  int current_time = machine.system().event().getTicks();
  if((current_time - time_of_last_titlebar_update_) > 60)
  {
    time_of_last_titlebar_update_ = current_time;

    if(machine.sceneNumber() != last_seen_number_ ||
       machine.lineNumber() != last_line_number_)
    {
      last_seen_number_ = machine.sceneNumber();
      last_line_number_ = machine.lineNumber();
      setWindowTitle();
    }
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::setWindowTitle()
{
  ostringstream oss;
  oss << caption_title_;

  if(displaySubtitle() && subtitle_ != "")
  {
    oss << ": " << subtitle_;
  }

  if(display_data_in_titlebar_)
  {
    oss << " - (SEEN" << last_seen_number_ << ")(Line "
        << last_line_number_ << ")";
  }

  SDL_WM_SetCaption(oss.str().c_str(), NULL);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::setWindowSubtitle(const std::string& cp932str,
                                          int text_encoding)
{
  // @todo Still not restoring title correctly!
  subtitle_ = cp932toUTF8(cp932str, text_encoding);

  GraphicsSystem::setWindowSubtitle(cp932str, text_encoding);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::allocateDC(int dc, Size size)
{
  if(dc >= 16)
    throw rlvm::Exception("Invalid DC number in SDLGrpahicsSystem::allocate_dc");

  // We can't reallocate the screen!
  if(dc == 0)
    throw rlvm::Exception("Attempting to reallocate DC 0!");

  // DC 1 is a special case and must always be at least the size of
  // the screen.
  if(dc == 1)
  {
    SDL_Surface* dc0 = *(display_contexts_[0]);
    if(size.width() < dc0->w)
      size.setWidth(dc0->w);
    if(size.height() < dc0->h)
      size.setHeight(dc0->h);
  }

  // Allocate a new obj.
  display_contexts_[dc]->allocate(size);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::freeDC(int dc)
{
  if(dc == 0)
    throw rlvm::Exception("Attempt to deallocate DC[0]");
  else if(dc == 1)
  {
    // DC[1] never gets freed; it only gets blanked
    getDC(1)->fill(RGBAColour::Black());
  }
  else
    display_contexts_[dc]->deallocate();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::verifySurfaceExists(int dc, const std::string& caller)
{
  if(dc >= 16)
  {
    ostringstream ss;
    ss << "Invalid DC number (" << dc << ") in " << caller;
    throw rlvm::Exception(ss.str());
  }

  if(display_contexts_[dc] == NULL)
  {
    ostringstream ss;
    ss << "Parameter DC[" << dc << "] not allocated in " << caller;
    throw rlvm::Exception(ss.str());
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::verifyDCAllocation(int dc, const std::string& caller)
{
  if(display_contexts_[dc] == NULL)
  {
    ostringstream ss;
    ss << "Couldn't allocate DC[" << dc << "] in " << caller
       << ": " << SDL_GetError();
    throw SystemError(ss.str());
  }
}

// -----------------------------------------------------------------------

typedef enum { NO_MASK, ALPHA_MASK, COLOR_MASK} MaskType;

// Note to self: These describe the byte order IN THE RAW G00 DATA!
// These should NOT be switched to native byte order.
#define DefaultRmask 0xff0000
#define DefaultGmask 0xff00
#define DefaultBmask 0xff
#define DefaultAmask 0xff000000
#define DefaultBpp 32

static SDL_Surface* newSurfaceFromRGBAData(int w, int h, char* data,
                                           MaskType with_mask)
{
  int amask = (with_mask == ALPHA_MASK) ? DefaultAmask : 0;
  SDL_Surface* tmp = SDL_CreateRGBSurfaceFrom(
    data, w, h, DefaultBpp, w*4, DefaultRmask, DefaultGmask,
    DefaultBmask, amask);

  // This is the perfect example of why I need to come back and
  // really understand this part of the code I'm stealing. WTF is
  // this!?
  // :Surface doesn't use preallocated memory? (the malloc'd data argument)
  // :This is silly! --RT
  //tmp->flags &= ~SDL_PREALLOC;

  SDL_Surface* surf;
  if (with_mask == ALPHA_MASK)
    surf = SDL_DisplayFormatAlpha(tmp);
  else
    surf = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);
  return surf;
};

// -----------------------------------------------------------------------

/**
 * Helper function for load_surface_from_file; invoked in a stl loop.
 */
SDLSurface::GrpRect xclannadRegionToGrpRect(const GRPCONV::REGION& region)
{
  SDLSurface::GrpRect rect;
  rect.rect = Rect(Point(region.x1, region.y1),
                   Point(region.x2 + 1, region.y2 + 1));
  rect.originX = region.origin_x;
  rect.originY = region.origin_y;
  return rect;
}

// -----------------------------------------------------------------------

/**
 * @author Jagarl
 * @author Elliot Glaysher
 *
 * Loads a file from disk into a Surface object. The file loaded
 * should be a type 0 or type 1 g00 bitmap.
 *
 * Jagarl's original implementation used an inline, intrusive caching
 * system. I've substituted this with an LRU cache class.
 *
 * @param filename File to load (a full filename, not the basename)
 * @return A Surface object with the data from this file
 *
 * @todo Considering that I really don't understand this whole section
 * of the code, I'll probably *NEED* to come back and rewrite this,
 * simply because I'll find out that this doesn't do what I think it does.
 *
 * @warning This function probably isn't basic exception safe.
 *
 * @note The surface returned by this function will never be
 * modified; it will either be put in an object, which is immutable,
 * or it will be copied into the DC.
 */
boost::shared_ptr<Surface> SDLGraphicsSystem::loadNonCGSurfaceFromFile(
    const std::string& short_filename) {
  // First check to see if this surface is already in our internal cache
  shared_ptr<Surface> cached_surface = image_cache_.fetch(short_filename);
  if (cached_surface)
    return cached_surface;

  boost::filesystem::path filename =
      findFile(system(), short_filename, IMAGE_FILETYPES);

  // Glue code to allow my stuff to work with Jagarl's loader
  FILE* file = fopen(filename.file_string().c_str(), "rb");
  if(!file)
  {
    ostringstream oss;
    oss << "Could not open file: " << filename;
    throw rlvm::Exception(oss.str());
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  scoped_array<char> d(new char[size + 1]);
  fseek(file, 0, SEEK_SET);
  fread(d.get(), size, 1, file);
  fclose(file);

  // For the time being, and against my better judgement, we simply
  // call the image loading methods stolen from xclannad. The
  // following code is stolen verbatim from picture.cc in xclannad.
  scoped_ptr<GRPCONV> conv(GRPCONV::AssignConverter(d.get(), size, "???"));
  if (conv == 0) {
    throw SystemError("Failure in GRPCONV.");
  }
  // do not free until SDL_FreeSurface() is called on the surface using it
  char* mem = (char*)malloc(conv->Width() * conv->Height() * 4 + 1024);
  SDL_Surface* s = 0;
  if (conv->Read(mem)) {
    MaskType is_mask = conv->IsMask() ? ALPHA_MASK : NO_MASK;
    if (is_mask == ALPHA_MASK) { // alpha §¨§π§Ÿ§∆ 0xff § §È•ﬁ•π•ØÃµ§∑§»§π§ÅE//      cerr << "Loading alpha mask..." << endl;
      int len = conv->Width()*conv->Height();
      unsigned int* d = (unsigned int*)mem;
      int i; for (i=0; i<len; i++) {
        if ( (*d&0xff000000) != 0xff000000) break;
        d++;
      }
      if (i == len) {
//        cerr << "No mask found!" << endl;
        is_mask = NO_MASK;
      }
    }

    s = newSurfaceFromRGBAData(conv->Width(), conv->Height(), mem, is_mask);
  }
  free(mem);

  // Grab the Type-2 information out of the converter or create one
  // default region if none exist
  vector<SDLSurface::GrpRect> region_table;
  if(conv->region_table.size())
  {
    transform(conv->region_table.begin(), conv->region_table.end(),
              back_inserter(region_table),
              xclannadRegionToGrpRect);
  }
  else
  {
    SDLSurface::GrpRect rect;
    rect.rect = Rect(Point(0, 0), Size(conv->Width(), conv->Height()));
    rect.originX = 0;
    rect.originY = 0;
    region_table.push_back(rect);
  }

  shared_ptr<Surface> surface_to_ret(new SDLSurface(s, region_table));
  image_cache_.insert(short_filename, surface_to_ret);
  return surface_to_ret;
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLGraphicsSystem::getDC(int dc)
{
  verifySurfaceExists(dc, "SDLGraphicsSystem::get_dc");

  // If requesting a DC that doesn't exist, allocate it first.
  if (display_contexts_[dc]->rawSurface() == NULL)
    allocateDC(dc, display_contexts_[0]->size());

  return display_contexts_[dc];
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLGraphicsSystem::buildSurface(const Size& size)
{
  return shared_ptr<Surface>(new SDLSurface(size));
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::reset()
{
  last_seen_number_ = 0;
  last_line_number_ = 0;

  GraphicsSystem::reset();
}
