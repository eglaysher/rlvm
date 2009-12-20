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

/**
 * @file   GraphicsSystem.hpp
 * @author Elliot Glaysher
 * @date   Sun Oct 15 15:07:23 2006
 *
 * @brief  Defines the abstract interface to the Graphics system.
 */

#ifndef SRC_SYSTEMS_BASE_GRAPHICSSYSTEM_HPP_
#define SRC_SYSTEMS_BASE_GRAPHICSSYSTEM_HPP_

#include <set>

#include "Systems/Base/CGMTable.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/Rect.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <vector>
#include <iosfwd>

// -----------------------------------------------------------------------

class Gameexe;
class GraphicsObject;
class GraphicsObjectData;
class GraphicsStackFrame;
class MouseCursor;
class Renderable;
class RLMachine;
class Size;
class Surface;
class System;
struct ObjectSettings;

template<typename T> class LazyArray;

// -----------------------------------------------------------------------

// Huh?

/**
 * Variables and configuration data that are global across all save
 * game files in a game.
 */
struct GraphicsSystemGlobals {
  GraphicsSystemGlobals();
  explicit GraphicsSystemGlobals(Gameexe& gameexe);

  /// ShowObject flags
  int show_object_1, show_object_2;

  int show_weather;

  /// Whether we should skip animations (such as those made by Effect)
  int skip_animations;

  /// screen mode. 1 is windowed (default), 0 is full-screen.
  int screen_mode;

  /// CG Table
  CGMTable cg_table;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & show_object_1 & show_object_2 & show_weather;

    if (version > 0)
      ar & cg_table;

    if (version > 1)
      ar & screen_mode;
  }
};

BOOST_CLASS_VERSION(GraphicsSystemGlobals, 2)

// -----------------------------------------------------------------------

/**
 * When marking the screen as dirty, we need to know what kind of
 * operation was done
 */
enum GraphicsUpdateType {
  GUT_DRAW_DC0,
  GUT_DISPLAY_OBJ,
  GUT_TEXTSYS,
  GUT_MOUSE_MOTION
};

// -----------------------------------------------------------------------

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

class GraphicsSystem : public EventListener {
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

 public:
  GraphicsSystem(System& system, Gameexe& gameexe);
  virtual ~GraphicsSystem();

  bool isResponsibleForUpdate() const { return is_responsible_for_update_; }
  void setIsResponsibleForUpdate(bool in);

  void setDefaultGrpName(const std::string& name) { default_grp_name_ = name; }
  const std::string& defaultGrpName() const { return default_grp_name_; }
  void setDefaultBgrName(const std::string& name) { default_bgr_name_ = name; }
  const std::string& defaultBgrName() const { return default_bgr_name_; }

  DCScreenUpdateMode screenUpdateMode() const { return screen_update_mode_; }
  virtual void setScreenUpdateMode(DCScreenUpdateMode u);

  System& system() { return system_; }

  /**
   * @name Mouse Cursor Management
   *
   * @{
   */

  /**
   * Whether we are using a custom cursor. Verifies that there was a
   * \#MOUSE_CURSOR entry in the Gameexe.ini file, and that the currently
   * selected cursor exists.
   */
  int useCustomCursor();

  /// Sets the cursor to the incoming cursor index.
  virtual void setCursor(int cursor);

  /// Returns the current index.
  int cursor() const { return cursor_; }

  /// Whether we display a cursor at all
  void setShowCursor(const int in) { show_curosr_ = in; }
  /// @}

  /**
   * @name Graphics Stack
   *
   * @{
   */
  GraphicsStackFrame& addGraphicsStackFrame(const std::string& name);

  std::vector<GraphicsStackFrame>& graphicsStack();

  int stackSize() const;
  void clearStack();

  void stackPop(int num_items);

  /// Replays the graphics stack. This is called after we've reloaded
  /// a saved game.
  void replayGraphicsStack(RLMachine& machine);
  /// @}


  // -----------------------------------------------------------------------

  /**
   * @name Final Renderers
   *
   * Individual LongOperations can also hook into the end of the rendering
   * pipeline by injecting Renderables. There should only really be one
   * Renderable on screen at a time, but the interface allows for multiple
   * ones.
   *
   * @{
   */
  void addRenderable(Renderable* renderable);
  void removeRenderable(Renderable* renderable);
  /// @}

  // -----------------------------------------------------------------------


  /**
   * @name Title management
   *
   * We put this here since there's no good place to put it in
   * general. (And it was here before I figured out what I was doing)
   *
   * @{
   */

  /**
   * Sets the current value of the subtitle, as set with title(). This
   * is virtual so that UTF8 or other charset systems can convert for
   * their own internal copy.
   *
   * @param cp932str The subtitle (encoded in cp932 or similar)
   * @param text_encoding The encoding type (as passed in to cp932toUTF8)
   */
  virtual void setWindowSubtitle(const std::string& cp932str,
                                 int text_encoding);

  /**
   * Returns the current window subtitle.
   *
   * @return The current window subtitle in cp932 encoding.
   */
  const std::string& windowSubtitle() const;

  bool displaySubtitle() const { return display_subtitle_; }
  /// @}

  /**
   * @name Saving Global Values
   *
   * @{
   */
  GraphicsSystemGlobals& globals() { return globals_; }
  /// @}

  /**
   * @name Show Object flags
   *
   * The `show object' flags are used to provide a way of enabling or
   * disabling interface elements from the menu. If an object's
   * `ObjectOnOff' property is set to 1 or 2, it will be shown or
   * hidden depending on the corresponding `show object' flag. This is
   * one of the properties controlled by the \#OBJECT variables in
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
  int showObject1() const { return globals_.show_object_1; }

  void setShowObject2(const int in);
  int showObject2() const { return globals_.show_object_2; }
  /// @}

  /**
   * @name Other object display settings
   *
   * @{
   */
  void setShowWeather(const int in);
  int showWeather() const { return globals_.show_weather; }

  // Sets whether we're in fullscreen mode. setScreenMode() is virtual so we
  // can tell SDL to switch the screen mode.
  virtual void setScreenMode(const int in);
  int screenMode() const { return globals_.screen_mode; }
  void toggleFullscreen();

  /**
   * Toggles whether the interface is shown. Called by
   * PauseLongOperation and related functors.
   */
  void toggleInterfaceHidden();
  bool interfaceHidden();

  /**
   * Whether we should skip animations (such as all the Effect subclasses).
   */
  void setSkipAnimations(const int in) { globals_.skip_animations = in; }
  int skipAnimations() const { return globals_.skip_animations; }

  /**
   * Returns the ObjectSettings from the Gameexe for obj_num. The data
   * from this method should be used by all subclasses of
   * GraphicsSystem when deciding whether to render an object or not.
   */
  ObjectSettings getObjectSettings(const int obj_num);
  /// @}

  /**
   * @name Screen refreshing
   *
   * @{
   */

  /**
   * Should be called by any of the drawing functions the screen is
   * invalidated.
   *
   * For more information, please see section 5.10.4 of the RLDev
   * manual, which deals with the behaviour of screen updates, and the
   * various modes.
   */
  virtual void markScreenAsDirty(GraphicsUpdateType type);

  /**
   * Forces a refresh of the screen the next time the graphics system
   * executes.
   */
  virtual void forceRefresh();

  bool screenNeedsRefresh() const { return screen_needs_refresh_; }

  void screenRefreshed() { screen_needs_refresh_ = false; }
  /// @}

  virtual void beginFrame();
  virtual void endFrame();

  /**
   * Performs a full redraw of the screen; blitting dc0 to the screen,
   * then blitting all the visible objects, and then blitting all the
   * visible text windows.
   *
   * The Reallive function refresh() simply calls this method, but
   * this method is also used internally to perform the same task.
   *
   * @param tree Optional parameter that will dump out a render tree to
   *             the passed in ostream. Usually NULL.
   */
  void refresh(std::ostream* tree);

  virtual boost::shared_ptr<Surface> renderToSurfaceWithBg(
    boost::shared_ptr<Surface> bg);


  /**
   * Called from the game loop; Does everything that's needed to keep
   * things up.
   */
  virtual void executeGraphicsSystem(RLMachine& machine) = 0;

  /**
   * Returns the size of the window in pixels.
   */
  const Size& screenSize() const { return screen_size_; }

  /**
   * Returns a rectangle with an origin of (0,0) and a size returned by
   * screenSize().
   */
  const Rect& screenRect() const { return screen_rect_; }

  virtual void allocateDC(int dc, Size size) = 0;
  virtual void setMinimumSizeForDC(int dc, Size size) = 0;
  virtual void freeDC(int dc) = 0;

  // ----------------------------------------- [ Surface loading functions ]

  /**
   * Loads an image, optionally marking that this image has been loaded (if it
   * is in the game's CGM table).
   */
  boost::shared_ptr<Surface> loadSurfaceFromFile(
      RLMachine& machine, const std::string& short_filename);

  /**
   * Just loads an image. This shouldn't be used for images that are destined
   * for one of the DCs, since those can be CGs.
   */
  virtual boost::shared_ptr<Surface> loadNonCGSurfaceFromFile(
      const std::string& short_filename) = 0;

  virtual boost::shared_ptr<Surface> getHaikei() = 0;

  virtual boost::shared_ptr<Surface> getDC(int dc) = 0;

  virtual boost::shared_ptr<Surface> buildSurface(const Size& size) = 0;

  // ----------------------------------- [ Object getter/factory functions ]
  /**
   * @name Graphics Object operations
   *
   * @{
   */

  /**
   * Clears and promotes objects.
   */
  void clearAndPromoteObjects();

  /**
   * Calls render() on all foreground objects that need to be
   * rendered.
   *
   * @param machine RLMachine context.
   * @param tree Optional stream to write this piece of the render tree to.
   */
  void renderObjects(std::ostream* tree);

  /**
   * Creates rendering data for a graphics object from a G00, PDT or ANM file.
   *
   * Does not deal with GAN files. Those are built with a separate function.
   */
  GraphicsObjectData* buildObjOfFile(const std::string& filename);

  /// Object getters
  /// layer == 0 for fg, layer == 1 for bg.
  GraphicsObject& getObject(int layer, int obj_number);
  void setObject(int layer, int obj_number, GraphicsObject& object);

  /**
   * Deallocates all graphics objects.
   */
  void clearAllObjects();

  LazyArray<GraphicsObject>& backgroundObjects();
  LazyArray<GraphicsObject>& foregroundObjects();

  /**
   * Takes a snapshot of the current object state. This snapshot is saved
   * instead of the current state of the graphics, since RealLive is a savepoint
   * based system.
   *
   * (This operation isn't exceptionally expensive; internally GraphicsObject
   * has multiple copy-on-write data structs to make this and object promotion a
   * relativly cheap operation.)
   */
  void takeSavepointSnapshot();
  /// @}

  /// Sets DC0 to black and frees up DCs 1 through 16.
  void clearAllDCs();

  /**
   * @name Overridden from MouseMotionListener:
   *
   * @{
   */
  /// Listen to the mouse's location so we can possibly draw a cursor there.
  virtual void mouseMotion(const Point& new_location);
  /// @}

  /**
   * Reset the system. Should clear all state for when a user loads a
   * game.
   */
  virtual void reset();

  /// Access to the cgtable for the cg* functions.
  CGMTable& cgTable() { return globals_.cg_table; }

 protected:
  typedef std::set<Renderable*> FinalRenderers;

  FinalRenderers::iterator renderer_begin() { return final_renderers_.begin(); }
  FinalRenderers::iterator renderer_end() { return final_renderers_.end(); }

  const Point& cursorPos() const { return cursor_pos_; }

  boost::shared_ptr<MouseCursor> currentCursor();

  void setScreenSize(const Size& size) {
    screen_size_ = size;
    screen_rect_ = Rect(Point(0, 0), size);
  }

 private:
  /// Default grp name (used in grp* and rec* functions where filename
  /// is '???')
  std::string default_grp_name_;

  /// Default bgr name (used in bgr* functions where filename is
  /// '???')
  std::string default_bgr_name_;

  /// Current screen update mode
  DCScreenUpdateMode screen_update_mode_;

  /// Flag set to redraw the screen NOW
  bool screen_needs_refresh_;

  /// Whether it is the Graphics system's responsibility to redraw the
  /// screen. Some LongOperations temporarily take this responsibility
  /// to implement pretty fades and wipes
  bool is_responsible_for_update_;

  /// Whether we should try to append subtitle_ in the window
  /// titlebar
  bool display_subtitle_;

  /// cp932 encoded subtitle string
  std::string subtitle_;

  /// Controls whether we render the interface (this can be
  /// temporarily toggled by the user at runtime)
  bool hide_interface_;

  /// Mutable global data to be saved in the globals file
  GraphicsSystemGlobals globals_;

  /// Size of our display window.
  Size screen_size_;

  /// Rectangle of the screen.
  Rect screen_rect_;

  /// Immutable
  struct GraphicsObjectSettings;
  /// Immutable global data that's constructed from the Gameexe.ini file.
  boost::scoped_ptr<GraphicsObjectSettings> graphics_object_settings_;

  struct GraphicsObjectImpl;
  boost::scoped_ptr<GraphicsObjectImpl> graphics_object_impl_;

  /** Whether we should use a custom mouse cursor. Set while parsing
   * the Gameexe file, and then left unchanged. We only use a custom
   * mouse cursor if \#MOUSE_CURSOR is set in the Gameexe
   */
  bool use_custom_mouse_cursor_;

  /// Whether we should render any cursor. Controller by the bytecode.
  bool show_curosr_;

  /** Current cursor id. Initially set to \#MOUSE_CURSOR if the key
   * exists.
   */
  int cursor_;

  /// Location of the cursor's hotspot
  Point cursor_pos_;

  /// Current mouse cursor
  boost::shared_ptr<MouseCursor> mouse_cursor_;

  /// MouseCursor construction is nontrivial so cache everything we
  /// build:
  typedef std::map<int, boost::shared_ptr<MouseCursor> > MouseCursorCache;
  MouseCursorCache cursor_cache_;

  /// A set of renderers
  FinalRenderers final_renderers_;

  /// Our parent system object.
  System& system_;

  /// boost::serialization support
  friend class boost::serialization::access;

  // boost::serialization forward declaration
  template<class Archive>
  void save(Archive & ar, const unsigned int file_version) const;

  // boost::serialization forward declaration
  template<class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

const int OBJECTS_IN_A_LAYER = 256;

#endif  // SRC_SYSTEMS_BASE_GRAPHICSSYSTEM_HPP_

