// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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

/**
 * @file   SDLGraphicsSystem.cpp
 * @author Elliot Glaysher
 * @date   Fri Oct  6 13:34:08 2006
 * 
 * @brief  Exposed interface for the SDL Graphics system.
 * 
 * While there are other 
 * 
 * @todo Make the constructor take the Gameexe.ini, and read the
 * initial window size from it.
 */

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"

#include "Systems/SDL/SDLGraphicsSystem.hpp"
#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/SDLRenderToTextureSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Texture.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/TextSystem.hpp"

#include "libReallive/defs.h"
#include "libReallive/gameexe.h"
#include "file.h"
#include "Utilities.h"

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

using namespace boost;
using namespace std;
using namespace libReallive;

// -----------------------------------------------------------------------
// Private Interface
// -----------------------------------------------------------------------

void SDLGraphicsSystem::markScreenAsDirty()
{
  switch(screenUpdateMode())
  {
  case SCREENUPDATEMODE_AUTOMATIC:
  case SCREENUPDATEMODE_SEMIAUTOMATIC:
  {
    // Perform a blit of DC0 to the screen, and update it.
    m_screenDirty = true;
    break;
  }
  case SCREENUPDATEMODE_MANUAL:
  {
    // Don't really do anything.
    break;
  }
  default:
  {
    ostringstream oss;
    oss << "Invalid screen update mode value: " << screenUpdateMode();
    throw Error(oss.str());
  }
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::markScreenForRefresh()
{
  m_screenNeedsRefresh = true;
}

// -----------------------------------------------------------------------

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
//  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, (GLdouble)m_width, (GLdouble)m_height, 0.0, 0.0, 1.0);
  ShowGLErrors();
 	
  glMatrixMode(GL_MODELVIEW);
//  glPushMatrix();
  glLoadIdentity();
  ShowGLErrors();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::refresh(RLMachine& machine) 
{
  beginFrame();

  // Display DC0
  m_displayContexts[0]->renderToScreen(0, 0, m_width, m_height, 
                                       0, 0, m_width, m_height, 255);

  // Render all visible foreground objects
  for_each(foregroundObjects, foregroundObjects + 256,
           bind(&GraphicsObject::render, _1, ref(machine)));

  // Render text
  machine.system().text().render(machine);

  endFrame();
}

// -----------------------------------------------------------------------

shared_ptr<Surface> SDLGraphicsSystem::renderToSurfaceWithBg(
  RLMachine& machine, shared_ptr<Surface> bg)
{
  beginFrame();

  // Display DC0
  bg->renderToScreen(0, 0, m_width, m_height, 
                     0, 0, m_width, m_height, 255);

  // Render all visible foreground objects
  for_each(foregroundObjects, foregroundObjects + 256,
           bind(&GraphicsObject::render, _1, ref(machine)));

  return endFrameToSurface();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::endFrame()
{
  glFlush();
  
  // Swap the buffers
  SDL_GL_SwapBuffers();
  ShowGLErrors();
}

// -----------------------------------------------------------------------

shared_ptr<Surface> SDLGraphicsSystem::endFrameToSurface()
{
  return shared_ptr<Surface>(new SDLRenderToTextureSurface(m_width, m_height));
}

// -----------------------------------------------------------------------
// Public Interface
// -----------------------------------------------------------------------

/** 
 *
 * @pre SDL is initialized.
 */
SDLGraphicsSystem::SDLGraphicsSystem(Gameexe& gameexe)
  : m_screenDirty(false), m_screenNeedsRefresh(false), 
    m_displayDataInTitlebar(false), m_lastSeenNumber(0), 
    m_lastLineNumber(0)
{
  for(int i = 0; i < 16; ++i)
    m_displayContexts[i].reset(new SDLSurface);

  // Let's get some video information.
  const SDL_VideoInfo* info = SDL_GetVideoInfo( );
  
  if( !info ) {
    ostringstream ss;
    ss << "Video query failed: " << SDL_GetError();
    throw Error(ss.str());
  }

  int graphicsMode = gameexe("SCREENSIZE_MOD").to_int();
  if(graphicsMode == 0)
  {
    m_width = 640;
    m_height = 480;
  }
  else if(graphicsMode == 1)
  {
    m_width = 800;
    m_height = 600;
  }
  else
  {
    ostringstream oss;
    oss << "Illegal #SCREENSIZE_MOD value: " << graphicsMode << endl;
    throw Error(oss.str());
  }
  int bpp = info->vfmt->BitsPerPixel;

  /* the flags to pass to SDL_SetVideoMode */
  int videoFlags;
  videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
  videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
  videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
  videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */
  
  /* This checks to see if surfaces can be stored in memory */
  if ( info->hw_available )
	videoFlags |= SDL_HWSURFACE;
  else
	videoFlags |= SDL_SWSURFACE;

  /* This checks if hardware blits can be done */
  if ( info->blit_hw )
	videoFlags |= SDL_HWACCEL;

  /* Sets up OpenGL double buffering */
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  // Set the video mode
  if((m_screen =
      SDL_SetVideoMode( m_width, m_height, bpp, videoFlags)) == 0 )
  {
    // This could happen for a variety of reasons,
    // including DISPLAY not being set, the specified
    // resolution not being available, etc.
    ostringstream ss;
    ss << "Video mode set failed: " << SDL_GetError();
    throw Error(ss.str());
  }	

  // Se tthe title
  SDL_WM_SetCaption("RLVM", NULL);


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

  /* Setup The Ambient Light */
//    glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient );

  /* Setup The Diffuse Light */
//    glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse );

  /* Position The Light */
//    glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );

  /* Enable Light One */
//    glEnable( GL_LIGHT1 );

  /* Full Brightness, 50% Alpha ( NEW ) */
  glColor4f( 1.0f, 1.0f, 1.0f, 0.5f);

  // Now we allocate the first two display contexts with equal size to
  // the display
  m_displayContexts[0]->allocate(m_width, m_height, this);
  m_displayContexts[1]->allocate(m_width, m_height);

  // When debug is set, display trace data in the titlebar
  if(gameexe("MEMORY").exists())
  {
    m_displayDataInTitlebar = true;
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::executeGraphicsSystem(RLMachine& machine)
{
//  cerr << "executeGraphicsSystm()" << endl;
  // For now, nothing, but later, we need to put all code each cycle
  // here.
  if(m_screenNeedsRefresh || 
     (screenUpdateMode() != SCREENUPDATEMODE_MANUAL && m_screenDirty))
  {
    refresh(machine);
    m_screenNeedsRefresh = false;
    m_screenDirty = false;
  }
    
  // For example, we should probably do something when the screen is
  // dirty.

  // Update the seen.
  if(m_displayDataInTitlebar)
  {
    int currentTime = machine.system().event().getTicks();

    if((currentTime - m_timeOfLastTitlebarUpdate) > 20)
    {
      m_timeOfLastTitlebarUpdate = currentTime;
      if(machine.sceneNumber() != m_lastSeenNumber ||
         machine.lineNumber() != m_lastLineNumber)
      {
        m_lastSeenNumber = machine.sceneNumber();
        m_lastLineNumber = machine.lineNumber();

        ostringstream oss;
        oss << "RLVM - (SEEN" << m_lastSeenNumber << ")(Line " 
            << m_lastLineNumber << ")";

        SDL_WM_SetCaption(oss.str().c_str(), NULL);
      }
    }
  }
}

// -----------------------------------------------------------------------

int SDLGraphicsSystem::screenWidth() const
{
  return m_width;
}

// -----------------------------------------------------------------------

int SDLGraphicsSystem::screenHeight() const 
{
  return m_height;
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::allocateDC(int dc, int width, int height)
{
  if(dc >= 16)
    throw Error("Invalid DC number in SDLGrpahicsSystem::allocateDC");

  // We can't reallocate the screen!
  if(dc == 0)
    throw Error("Attempting to reallocate DC 0!");

  // DC 1 is a special case and must always be at least the size of
  // the screen.
  if(dc == 1)
  {
    SDL_Surface* dc0 = *(m_displayContexts[0]);
    if(width < dc0->w)
      width = dc0->w;
    if(height < dc0->h)
      height = dc0->h;
  }

  // Allocate a new obj.
  m_displayContexts[dc]->allocate(width, height);
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::freeDC(int dc)
{
  if(dc == 0)
    throw Error("Attempt to deallocate DC[0]");
  else if(dc == 1)
  {
    // DC[1] never gets freed; it only gets blanked
    getDC(1)->fill(0, 0, 0, 255);
  }
  else
    m_displayContexts[dc]->deallocate();
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::verifySurfaceExists(int dc, const std::string& caller)
{
  if(dc >= 16)
  {
    ostringstream ss;
    ss << "Invalid DC number (" << dc << ") in " << caller;
    throw Error(ss.str());
  }

  if(m_displayContexts[dc] == NULL)
  {
    ostringstream ss;
    ss << "Parameter DC[" << dc << "] not allocated in " << caller;
    throw Error(ss.str());
  }
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::verifyDCAllocation(int dc, const std::string& caller)
{
  if(m_displayContexts[dc] == NULL)
  {
    ostringstream ss;
    ss << "Couldn't allocate DC[" << dc << "] in " << caller 
       << ": " << SDL_GetError();
    throw Error(ss.str());
  }
}

// -----------------------------------------------------------------------

GraphicsObject& SDLGraphicsSystem::getObject(int layer, int objNumber)
{
  if(layer < 0 || layer > 1)
    throw Error("Invalid layer number");

  if(objNumber < 0 || objNumber > 512)
    throw Error("Out of rnage object number");

  if(layer == OBJ_BG_LAYER)
    return backgroundObjects[objNumber];
  else
    return foregroundObjects[objNumber];
}

// -----------------------------------------------------------------------

void SDLGraphicsSystem::setObject(int layer, int objNumber, GraphicsObject& obj)
{
  if(layer < 0 || layer > 1)
    throw Error("Invalid layer number");

  if(objNumber < 0 || objNumber > 256)
    throw Error("Out of range object number");

  foregroundObjects[objNumber] = obj;
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
//  int amask = (with_mask == ALPHA_MASK) ? DefaultAmask : 0;
  int amask = DefaultAmask;
  SDL_Surface* tmp = SDL_CreateRGBSurfaceFrom(
    data, w, h, DefaultBpp, w*4, DefaultRmask, DefaultGmask, 
    DefaultBmask, amask);

  // This is the perfect example of why I need to come back and
  // really understand this part of the code I'm stealing. WTF is
  // this!?
  tmp->flags &= ~SDL_PREALLOC;

  SDL_Surface* surf = SDL_DisplayFormatAlpha(tmp);
  SDL_FreeSurface(tmp);
  return surf;
};

// -----------------------------------------------------------------------

/** 
 * Helper function for loadSurfaceFromFile; invoked in a stl loop.
 */
SDLSurface::GrpRect xclannadRegionToGrpRect(const GRPCONV::REGION& region)
{
  SDLSurface::GrpRect rect;
  rect.x1 = region.x1;
  rect.y1 = region.y1;
  rect.x2 = region.x2 + 1;
  rect.y2 = region.y2 + 1;
  return rect;
}

/** 
 * @author Jagarl
 *
 * Loads a file from disk into a Surface object. The file loaded
 * should be a type 0 or type 1 g00 bitmap. We don't handle PDT files
 * (yet).
 * 
 * @param filename File to load (a full filename, not the basename)
 * @return A Surface object with the data from this file
 *
 * @todo Considering that I really don't understand this whole section
 * of the code, I'll probably *NEED* to come back and rewrite this,
 * simply because I'll find out that this doesn't do what I think it does.
 *
 * @warning This function probably isn't basic exception safe.
 */
Surface* SDLGraphicsSystem::loadSurfaceFromFile(const std::string& filename)
{
  // Glue code to allow my stuff to work with Jagarl's loader
  FILE* file = fopen(filename.c_str(), "rb");
  if(!file)
  {
    ostringstream oss;
    oss << "Could not open file: " << filename;
    throw Error(oss.str());
  }

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  scoped_array<char> d(new char[size]);
  fseek(file, 0, SEEK_SET);
  fread(d.get(), size, 1, file);
  fclose(file);

  // For the time being, and against my better judgement, we simply
  // call the image loading methods stolen from xclannad. The
  // following code is stolen verbatim from picture.cc in xclannad.
  scoped_ptr<GRPCONV> conv(GRPCONV::AssignConverter(d.get(), size, "???"));
  if (conv == 0) { return 0;}
  char* mem = (char*)malloc(conv->Width() * conv->Height() * 4 + 1024);
  SDL_Surface* s = 0;
  if (conv->Read(mem)) {
    MaskType is_mask = conv->IsMask() ? ALPHA_MASK : NO_MASK;
    if (is_mask == ALPHA_MASK) { // alpha がすべて 0xff ならマスク無しとする
//      cerr << "Loading alpha mask..." << endl;
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

//  cerr << "Converter table size: " << conv->region_table.size() << endl;

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
    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = conv->Width();
    rect.y2 = conv->Height();
    region_table.push_back(rect);
  }

  return new SDLSurface(s, region_table);
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLGraphicsSystem::getDC(int dc)
{
  verifySurfaceExists(dc, "SDLGraphicsSystem::getDC");
  return m_displayContexts[dc];
}
                                
int SDLSurface::width() const { return m_surface->w; }
int SDLSurface::height() const { return m_surface->h; }

// -----------------------------------------------------------------------

class SDLGraphicsObjectOfFile : public GraphicsObjectData
{
private:
  scoped_ptr<SDLSurface> surface;

public:
  SDLGraphicsObjectOfFile(SDLGraphicsSystem& graphics, 
                          const std::string& filename)
    : surface(static_cast<SDLSurface*>(graphics.loadSurfaceFromFile(filename)))
  {  }

  SDLGraphicsObjectOfFile(SDLSurface* inSurface)
    : surface(inSurface)
  {  }

  virtual void render(RLMachine& machine, const GraphicsObject& rp)
  {
//    cerr << "Rendering object!" << endl;
//    surface->dump();
    surface->renderToScreenAsObject(rp);
  }

  virtual int pixelWidth(const GraphicsObject& rp)
  {
    const SDLSurface::GrpRect& rect = surface->getPattern(rp.pattNo());
    int width = rect.x2 - rect.x1;
    return int((rp.width() / 100.0f) * width);
  }

  virtual int pixelHeight(const GraphicsObject& rp)
  {
    const SDLSurface::GrpRect& rect = surface->getPattern(rp.pattNo());
    int height = rect.y2 - rect.y1;
    return int((rp.height() / 100.0f) * height);
  }

  GraphicsObjectData* clone() const 
  {
    return new SDLGraphicsObjectOfFile(static_cast<SDLSurface*>(surface->clone()));
  }
};

// -----------------------------------------------------------------------

void SDLGraphicsSystem::promoteObjects()
{
  for(int i = 0; i < 256; ++i) 
  {
    GraphicsObject& bgObj = getObject(OBJ_BG_LAYER, i);
    GraphicsObject& fgObj = getObject(OBJ_FG_LAYER, i);
    bool copyBgToFg = true;

    if(!bgObj.hasObjectData() && fgObj.wipeCopy())
      copyBgToFg = false;

    if(copyBgToFg)
    {
      fgObj = bgObj;
      bgObj.deleteObject();
    }
  }
}

// -----------------------------------------------------------------------

GraphicsObjectData* SDLGraphicsSystem::buildObjOfFile(RLMachine& machine, 
                                                      const std::string& filename)
{
  return new SDLGraphicsObjectOfFile(*this, findFile(machine, filename));
}
