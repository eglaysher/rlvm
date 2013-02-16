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

#ifndef SRC_SYSTEMS_BASE_GRAPHICSSYSTEM_HPP_
#define SRC_SYSTEMS_BASE_GRAPHICSSYSTEM_HPP_

#include <boost/filesystem/path.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
#include <boost/shared_ptr.hpp>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <queue>

#include "Systems/Base/CGMTable.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/Rect.hpp"
#include "Systems/Base/ToneCurve.hpp"

#include "Utilities/LazyArray.hpp"
#include "lru_cache.hpp"

class Gameexe;
class GraphicsObject;
class GraphicsObjectData;
class GraphicsStackFrame;
class HIKRenderer;
class HIKScript;
class MouseCursor;
class Renderable;
class RGBAColour;
class RLMachine;
class Size;
class Surface;
class System;
struct ObjectSettings;

template<typename T> class LazyArray;

// Variables and configuration data that are global across all save
// game files in a game.
struct GraphicsSystemGlobals {
  GraphicsSystemGlobals();
  explicit GraphicsSystemGlobals(Gameexe& gameexe);

  // ShowObject flags
  int show_object_1, show_object_2;

  int show_weather;

  // Whether we should skip animations (such as those made by Effect)
  int skip_animations;

  // screen mode. 1 is windowed (default), 0 is full-screen.
  int screen_mode;

  // CG Table
  CGMTable cg_table;
  
  // tone curve table
  ToneCurve tone_curves;

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

// When marking the screen as dirty, we need to know what kind of
// operation was done
enum GraphicsUpdateType {
  GUT_DRAW_DC0,
  GUT_DRAW_HIK,
  GUT_DISPLAY_OBJ,
  GUT_TEXTSYS,
  GUT_MOUSE_MOTION
};

// Which type of mutually exclusive background should we display?
enum GraphicsBackgroundType {
  BACKGROUND_DC0,
  BACKGROUND_HIK
};

// Abstract interface to a graphics system. Specialize this class for
// each system you plan on running RLVM on. For now, there's only one
// derived class; SDLGraphicsSystem.
//
// Two device contexts must be allocated during initialization; DC 0,
// which should refer to a surface that is (usually) blitted onto the
// screen immediatly after it is written to, and DC 1, which is simply
// guarenteed to be allocated, and is guarenteed to not be smaller
// then the screen. (Many {rec,grp} functions will load data onto DC1
// and then copy it onto DC0 with some sort of fancy transition
// effect.)
class GraphicsSystem : public EventListener {
 public:
  // The current display context drawing mode. The Reallive system
  // will update the screen after certain events in user code
  // regarding DCs.
  //
  // Note that these are not the only times when the screen will be
  // updated. Most functions that deal with text windows will trigger
  // screen updates. (Object manipulation functions *don't*.) Having
  // this fine level of control is why DCs are often used for smooth
  // animation...
  enum DCScreenUpdateMode {
    // The screen will be redrawn after every load or blit to DC 0.
    SCREENUPDATEMODE_AUTOMATIC,

    // We currently don't understand how this differs from automatic
    // mode. We declare it anyway for compatibility and the hope that
    // someday we will.
    SCREENUPDATEMODE_SEMIAUTOMATIC,

    // The screen is updated after refresh() is called
    SCREENUPDATEMODE_MANUAL
  };

  GraphicsSystem(System& system, Gameexe& gameexe);
  virtual ~GraphicsSystem();

  bool isResponsibleForUpdate() const { return is_responsible_for_update_; }
  void setIsResponsibleForUpdate(bool in);

  // Sets the default name for certain classes of commands. (Certain graphics
  // commands set either of these and most graphics commands can pass in '?',
  // which is substituted with the default name.)
  void setDefaultGrpName(const std::string& name) { default_grp_name_ = name; }
  const std::string& defaultGrpName() const { return default_grp_name_; }
  void setDefaultBgrName(const std::string& name) { default_bgr_name_ = name; }
  const std::string& defaultBgrName() const { return default_bgr_name_; }

  // Define who is responsible for screen updates.
  DCScreenUpdateMode screenUpdateMode() const { return screen_update_mode_; }
  virtual void setScreenUpdateMode(DCScreenUpdateMode u);

  void setGraphicsBackground(GraphicsBackgroundType t) { background_type_ = t; }

  System& system() { return system_; }

  // Screen Shaking

  // Reads #SHAKE.spec and loads the offsets into the screen shaking queue.
  void QueueShakeSpec(int spec);

  // Returns the current screen origin. This is used for simple #SHAKE.* based
  // screen shaking. While the screen is not shaking, this returns (0,0).
  Point GetScreenOrigin();

  // Whether we are currently shaking.
  bool IsShaking() const;

  // How long the current frame in the shaking should last. 10ms if there are
  // no frames.
  int CurrentShakingFrameTime() const;

  // Mouse Cursor Management

  // Whether we are using a custom cursor. Verifies that there was a
  // \#MOUSE_CURSOR entry in the Gameexe.ini file, and that the currently
  // selected cursor exists.
  int useCustomCursor();

  // Sets the cursor to the incoming cursor index.
  virtual void setCursor(int cursor);

  // Returns the current index.
  int cursor() const { return cursor_; }

  // Whether we display a cursor at all.
  void setShowCursorFromBytecode(const int in) {
    show_cursor_from_bytecode_ = in;
  }

  // Graphics stack implementation
  //
  // The RealLive virtual machine keeps track of recent graphics commands so
  // that when the game is restored, these graphics commands can be replayed to
  // recreate the screen state.

  // Adds |command|, the serialized form of a bytecode used by calling the
  // BytecodeElement::data().
  void addGraphicsStackCommand(const std::string& command);

  // Returns the number of entries in the stack.
  int stackSize() const;

  // Clears the graphics stack.
  void clearStack();

  // Removes (up to) |num_items| from the stack. (Stops when the stack is
  // empty).
  void stackPop(int num_items);

  // Replays the graphics stack. This is called after we've reloaded
  // a saved game and deals with both old style and the new stack system.
  void replayGraphicsStack(RLMachine& machine);

  // Sets the current hik script. GraphicsSystem takes ownership, freeing the
  // current HIKScript if applicable. |script| can be NULL.
  void setHikRenderer(HIKRenderer* script);
  HIKRenderer* getHikRenderer() const { return hik_renderer_.get(); }

  // -----------------------------------------------------------------------

  // Individual LongOperations can also hook into the end of the rendering
  // pipeline by injecting Renderables. There should only really be one
  // Renderable on screen at a time, but the interface allows for multiple
  // ones.
  void addRenderable(Renderable* renderable);
  void removeRenderable(Renderable* renderable);

  // Subtitle management

  // Sets the current value of the subtitle, as set with title(). This
  // is virtual so that UTF8 or other charset systems can convert for
  // their own internal copy.
  virtual void setWindowSubtitle(const std::string& cp932str,
                                 int text_encoding);

  // Returns the current window subtitle, in native encoding.
  const std::string& windowSubtitle() const;

  // Wether we should display the subtitle.
  bool displaySubtitle() const { return display_subtitle_; }

  // Access to the GrapihcsSystem global variables.
  GraphicsSystemGlobals& globals() { return globals_; }

  // The `show object' flags are used to provide a way of enabling or
  // disabling interface elements from the menu. If an object's
  // `ObjectOnOff' property is set to 1 or 2, it will be shown or
  // hidden depending on the corresponding `show object' flag. This is
  // one of the properties controlled by the \#OBJECT variables in
  // gameexe.ini.
  void setShowObject1(const int in);
  int showObject1() const { return globals_.show_object_1; }
  void setShowObject2(const int in);
  int showObject2() const { return globals_.show_object_2; }
  void setShowWeather(const int in);
  int showWeather() const { return globals_.show_weather; }

  // Sets whether we're in fullscreen mode. setScreenMode() is virtual so we
  // can tell SDL to switch the screen mode.
  virtual void setScreenMode(const int in);
  int screenMode() const { return globals_.screen_mode; }
  void toggleFullscreen();

  // Toggles whether the interface is shown. Called by
  // PauseLongOperation and related functors.
  void toggleInterfaceHidden();
  bool interfaceHidden();

  // Whether we should skip animations (such as all the Effect subclasses).
  void setSkipAnimations(const int in) { globals_.skip_animations = in; }
  int skipAnimations() const { return globals_.skip_animations; }

  // Returns the ObjectSettings from the Gameexe for obj_num. The data
  // from this method should be used by all subclasses of
  // GraphicsSystem when deciding whether to render an object or not.
  ObjectSettings getObjectSettings(const int obj_num);

  // Should be called by any of the drawing functions the screen is
  // invalidated.
  //
  // For more information, please see section 5.10.4 of the RLDev
  // manual, which deals with the behaviour of screen updates, and the
  // various modes.
  virtual void markScreenAsDirty(GraphicsUpdateType type);

  // Forces a refresh of the screen the next time the graphics system
  // executes.
  virtual void forceRefresh();

  bool screenNeedsRefresh() const { return screen_needs_refresh_; }
  void screenRefreshed() {
    screen_needs_refresh_ = false;
    object_state_dirty_ = false;
  }

  // We keep a separate state about whether object state has been modified. We
  // do this so that background object mutation in automatic mode plays nicely
  // with LongOperations.
  void markObjectStateAsDirty() { object_state_dirty_ = true; }
  bool objectStateDirty() const { return object_state_dirty_; }

  virtual void beginFrame() = 0;
  virtual void endFrame() = 0;
  virtual boost::shared_ptr<Surface> endFrameToSurface() = 0;

  // Performs a full redraw of the screen.
  void refresh(std::ostream* tree);

  // Draws the screen (as if refresh() was called), but draw to the returned
  // surface instead of the screen.
  boost::shared_ptr<Surface> renderToSurface();

  // Called from the game loop; Does everything that's needed to keep
  // things up.
  virtual void executeGraphicsSystem(RLMachine& machine);

  // Returns the size of the window in pixels.
  const Size& screenSize() const { return screen_size_; }

  // Returns a rectangle with an origin of (0,0) and a size returned by
  // screenSize().
  const Rect& screenRect() const { return screen_rect_; }

  virtual void allocateDC(int dc, Size size) = 0;
  virtual void setMinimumSizeForDC(int dc, Size size) = 0;
  virtual void freeDC(int dc) = 0;

  // Loads an image, optionally marking that this image has been loaded (if it
  // is in the game's CGM table).
  boost::shared_ptr<const Surface> getSurfaceNamedAndMarkViewed(
      RLMachine& machine, const std::string& short_filename);

  // Just loads an image. This shouldn't be used for images that are destined
  // for one of the DCs, since those can be CGs.
  boost::shared_ptr<const Surface> getSurfaceNamed(
      const std::string& short_filename);

  virtual boost::shared_ptr<Surface> getHaikei() = 0;

  virtual boost::shared_ptr<Surface> getDC(int dc) = 0;

  virtual boost::shared_ptr<Surface> buildSurface(const Size& size) = 0;

  virtual void fillScreenArea(const Rect& rect, const RGBAColour& colour) = 0;

  // Clears and promotes objects.
  void clearAndPromoteObjects();

  // Calls render() on all foreground objects that need to be
  // rendered.
  void renderObjects(std::ostream* tree);

  // Creates rendering data for a graphics object from a G00, PDT or ANM file.
  // Does not deal with GAN files. Those are built with a separate function.
  GraphicsObjectData* buildObjOfFile(const std::string& filename);

  // Object getters
  // layer == 0 for fg, layer == 1 for bg.
  GraphicsObject& getObject(int layer, int obj_number);
  void setObject(int layer, int obj_number, GraphicsObject& object);

  void clearObject(int obj_number);

  // Deallocates all graphics objects.
  void clearAllObjects();

  // Resets the object properties for all graphics objects.
  void resetAllObjectsProperties();

  // The number of objects in a layer for this game. Defaults to 256 and can be
  // overridden with #OBJECT_MAX.
  int objectLayerSize();

  LazyArray<GraphicsObject>& backgroundObjects();
  LazyArray<GraphicsObject>& foregroundObjects();

  // Returns true if there's a currently playing animation.
  bool animationsPlaying() const;

  // Takes a snapshot of the current object state. This snapshot is saved
  // instead of the current state of the graphics, since RealLive is a savepoint
  // based system.
  //
  // (This operation isn't exceptionally expensive; internally GraphicsObject
  // has multiple copy-on-write data structs to make this and object promotion a
  // relativly cheap operation.)
  void takeSavepointSnapshot();

  // Sets DC0 to black and frees up DCs 1 through 16.
  void clearAllDCs();

  // Implementation of MouseMotionListener:
  virtual void mouseMotion(const Point& new_location);

  // Reset the system. Should clear all state for when a user loads a game.
  virtual void reset();

  // Access to the cgtable for the cg* functions.
  CGMTable& cgTable() { return globals_.cg_table; }

  // Access to the tone curve effects file
  ToneCurve& toneCurve() { return globals_.tone_curves; }
  
  // We have a cache of HIK scripts. This is done so we can load HIKScripts
  // outside of loops.
  void PreloadHIKScript(System& system,
                        int slot,
                        const std::string& name,
                        const boost::filesystem::path& file);
  void ClearPreloadedHIKScript(int slot);
  void ClearAllPreloadedHIKScripts();
  boost::shared_ptr<HIKScript> GetHIKScript(
      System& system,
      const std::string& name,
      const boost::filesystem::path& file);

  // We have a cache of preloaded g00 files.
  void PreloadG00(int slot, const std::string& name);
  void ClearPreloadedG00(int slot);
  void ClearAllPreloadedG00();
  boost::shared_ptr<const Surface> GetPreloadedG00(const std::string& name);

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

  void drawFrame(std::ostream* tree);

 private:
  // Gets a platform appropriate surface loaded.
  virtual boost::shared_ptr<const Surface> loadSurfaceFromFile(
      const std::string& short_filename) = 0;

  // Default grp name (used in grp* and rec* functions where filename
  // is '???')
  std::string default_grp_name_;

  // Default bgr name (used in bgr* functions where filename is
  // '???')
  std::string default_bgr_name_;

  // Current screen update mode
  DCScreenUpdateMode screen_update_mode_;

  // Whether we display HIK or DC0.
  GraphicsBackgroundType background_type_;

  // Flag set to redraw the screen NOW
  bool screen_needs_refresh_;

  // Whether object state has been mutated since the last screen refresh.
  bool object_state_dirty_;

  // Whether it is the Graphics system's responsibility to redraw the
  // screen. Some LongOperations temporarily take this responsibility
  // to implement pretty fades and wipes
  bool is_responsible_for_update_;

  // Whether we should try to append subtitle_ in the window
  // titlebar
  bool display_subtitle_;

  // cp932 encoded subtitle string
  std::string subtitle_;

  // Controls whether we render the interface (this can be
  // temporarily toggled by the user at runtime)
  bool hide_interface_;

  // Mutable global data to be saved in the globals file
  GraphicsSystemGlobals globals_;

  // Size of our display window.
  Size screen_size_;

  // Rectangle of the screen.
  Rect screen_rect_;

  // Queued origin/time pairs. The front of the queue shall be the current
  // screen offset.
  std::queue<std::pair<Point, int> > screen_shake_queue_;

  // The last time |screen_shake_queue_| was modified.
  unsigned int time_at_last_queue_change_;

  // Immutable
  struct GraphicsObjectSettings;
  // Immutable global data that's constructed from the Gameexe.ini file.
  boost::scoped_ptr<GraphicsObjectSettings> graphics_object_settings_;

  struct GraphicsObjectImpl;
  boost::scoped_ptr<GraphicsObjectImpl> graphics_object_impl_;

  // Whether we should use a custom mouse cursor. Set while parsing the Gameexe
  // file, and then left unchanged. We only use a custom mouse cursor if
  // \#MOUSE_CURSOR is set in the Gameexe
  bool use_custom_mouse_cursor_;

  // Whether we should render any cursor. Controller by the bytecode.
  bool show_cursor_from_bytecode_;

  // Current cursor id. Initially set to \#MOUSE_CURSOR if the key exists.
  int cursor_;

  // Location of the cursor's hotspot
  Point cursor_pos_;

  // Current mouse cursor
  boost::shared_ptr<MouseCursor> mouse_cursor_;

  // MouseCursor construction is nontrivial so cache everything we
  // build:
  typedef std::map<int, boost::shared_ptr<MouseCursor> > MouseCursorCache;
  MouseCursorCache cursor_cache_;

  // A set of renderers
  FinalRenderers final_renderers_;

  // Our parent system object.
  System& system_;

  // Preloaded HIKScripts.
  typedef std::pair<std::string, boost::shared_ptr<HIKScript> > HIKArrayItem;
  typedef LazyArray<HIKArrayItem> HIKScriptList;
  HIKScriptList preloaded_hik_scripts_;

  // Preloaded G00 images.
  typedef std::pair<std::string, boost::shared_ptr<const Surface> >
      G00ArrayItem;
  typedef LazyArray<G00ArrayItem> G00ScriptList;
  G00ScriptList preloaded_g00_;

  /**
   * LRU cache filled with the last fifteen accessed images.
   *
   * This cache's contents are assumed to be immutable.
   */
  LRUCache<std::string, boost::shared_ptr<const Surface> > image_cache_;

  // Possible background script which drives graphics to the screen.
  boost::scoped_ptr<HIKRenderer> hik_renderer_;

  // boost::serialization support
  friend class boost::serialization::access;

  // boost::serialization forward declaration
  template<class Archive>
  void save(Archive & ar, const unsigned int file_version) const;

  // boost::serialization forward declaration
  template<class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(GraphicsSystem, 1)

#endif  // SRC_SYSTEMS_BASE_GRAPHICSSYSTEM_HPP_

