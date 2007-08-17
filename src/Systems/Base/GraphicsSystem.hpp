// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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
 * @file   GraphicsSystem.hpp
 * @author Elliot Glaysher
 * @date   Sun Oct 15 15:07:23 2006
 * 
 * @brief  Defines the abstract interface to the Graphics system.
 */

#ifndef __GraphicsSystem_hpp__
#define __GraphicsSystem_hpp__

#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

class Surface;
class RLMachine;
class GraphicsObject;
class GraphicsObjectData;
class Gameexe;
class ObjectSettings;

namespace Json {
class Value;
}

/** 
 * Abstract interface to a graphics system. Specialize this class for
 * each system you plan on running RLVM on. For now, there's only one
 * derived class; SDLGraphicsSystem.
 *
 * Two device contexts must be allocated during initialization; DC 0,
 * which should refer to a surface that is (usually) blitted onto the
 * screen immediatly after it is written to, and DC 1, which is simply
 * guarenteed to be allocated, and is guarenteed to not be smaller
 * then the screen. (Many {rec,grp} functions will load data onto DC1
 * and then copy it onto DC0 with some sort of fancy transition
 * effect.)
 */
class GraphicsSystem
{
public:
  /** 
   * The current display context drawing mode. The Reallive system
   * will update the screen after certain events in user code
   * regarding DCs.
   * 
   * Note that these are not the only times when the screen will be
   * updated. Most functions that deal with text windows will trigger
   * screen updates. (Object manipulation functions *don't*.) Having
   * this fine level of control is why DCs are often used for smooth
   * animation...
   */
  enum DCScreenUpdateMode {
    /// The screen will be redrawn after every load or blit to DC 0.
    SCREENUPDATEMODE_AUTOMATIC,   
    /// We currently don't understand how this differs from automatic
    /// mode. We declare it anyway for compatibility and the hope that
    /// someday we will.
    SCREENUPDATEMODE_SEMIAUTOMATIC,
    /// The screen is updated after refresh() is called
    SCREENUPDATEMODE_MANUAL
  };

private:
  /// Default grp name (used in grp* and rec* functions where filename
  /// is '???')
  std::string m_defaultGrpName;

  /// Default bgr name (used in bgr* functions where filename is
  /// '???')
  std::string m_defaultBgrName;

  /// Current screen update mode
  DCScreenUpdateMode m_screenUpdateMode;

  /// Whether we should try to append m_subtitle in the window
  /// titlebar
  bool m_displaySubtitle;

  /// utf8 encoded subtitle string
  std::string m_subtitle;

  /// ShowObject flags
  int m_showObject1, m_showObject2;

  struct GraphicsObjectSettings;
  boost::scoped_ptr<GraphicsObjectSettings> m_graphicsObjectSettings;

public:
  GraphicsSystem(Gameexe& gameexe);
  virtual ~GraphicsSystem();

  void setDefaultGrpName(const std::string& name) { m_defaultGrpName = name; }
  const std::string& defaultGrpName() const { return m_defaultGrpName; }
  void setDefaultBgrName(const std::string& name) { m_defaultBgrName = name; }
  const std::string& defaultBgrName() const { return m_defaultBgrName; }

  DCScreenUpdateMode screenUpdateMode() const { return m_screenUpdateMode; }
  void setScreenUpdateMode(DCScreenUpdateMode u) { m_screenUpdateMode = u; }

  /**
   * @name Title management
   * 
   * We put this here since there's no good place to put it in
   * general. (And it was here before I figured out what I was doing)
   *
   * @{
   */
  void setWindowSubtitle(const std::string& utf8encoded);
  const std::string& windowSubtitle() const;
  bool displaySubtitle() const { return m_displaySubtitle; }
  /// @}

  /**
   * @name Saving Global Values
   * 
   * @{
   */
  virtual void saveGlobals(Json::Value& system);
  virtual void loadGlobals(const Json::Value& system);
  /// @}

  /**
   * @name Show Object flags
   * 
   * The `show object' flags are used to provide a way of enabling or
   * disabling interface elements from the menu. If an object's
   * `ObjectOnOff' property is set to 1 or 2, it will be shown or
   * hidden depending on the corresponding `show object' flag. This is
   * one of the properties controlled by the #OBJECT variables in
   * gameexe.ini.
   *
   * In Clannad, ShowObject1 is used to control display of the date
   * marker at the top left of the screen (object 84).
   * 
   * @note { @b A value of 0 from these functions means that data is
   *       visible by default and a value of 1 means thaey are invisible. }
   *
   * @{
   */
  void setShowObject1(const int in);
  int showObject1() const { return m_showObject1; }

  void setShowObject2(const int in);
  int showObject2() const { return m_showObject1; }

  ObjectSettings getObjectSettings(const int objNum);
  /// @}


  // Marks the screen as dirty; something is done about this if we are
  // in automatic mode.
  virtual void markScreenAsDirty();

  // Marks the screen for refresh; we refresh the screen the next time
  // the graphics system is executed.
  virtual void markScreenForRefresh();

  virtual void beginFrame();
  virtual void refresh(RLMachine& machine) = 0;
  virtual void endFrame();

  virtual boost::shared_ptr<Surface> renderToSurfaceWithBg(
    RLMachine& machine, boost::shared_ptr<Surface> bg);


  /** 
   * Called from the game loop; Does everything that's needed to keep
   * things up.
   */
  virtual void executeGraphicsSystem(RLMachine& machine) = 0;

  virtual int screenWidth() const = 0;
  virtual int screenHeight() const = 0;

  virtual void allocateDC(int dc, int width, int height) = 0;
  virtual void freeDC(int dc) = 0;

/*
  virtual void getDCPixel(int x, int y, int dc, int& r, int& g, int& b) = 0;
*/

  // ----------------------------------------- [ Surface loading functions ]
  virtual boost::shared_ptr<Surface> loadSurfaceFromFile(
    const std::string& filename) = 0;

  virtual boost::shared_ptr<Surface> getDC(int dc) = 0;


  // ----------------------------------- [ Object getter/factory functions ]
  virtual void promoteObjects() = 0;
  virtual void clearAndPromoteObjects() = 0;

  virtual GraphicsObjectData* buildObjOfFile(RLMachine& machine, 
                                             const std::string& filename) = 0;

  /// Object getters
  /// layer == 0 for fg, layer == 1 for bg.
  virtual GraphicsObject& getObject(int layer, int objNumber) = 0;
  virtual void setObject(int layer, int objNumber, GraphicsObject& object) {}
};

const static int OBJ_FG_LAYER = 0;
const static int OBJ_BG_LAYER = 1;

const int OBJECTS_IN_A_LAYER = 256;

#endif
