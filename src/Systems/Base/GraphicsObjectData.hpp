// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_BASE_GRAPHICSOBJECTDATA_HPP_
#define SRC_SYSTEMS_BASE_GRAPHICSOBJECTDATA_HPP_

#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

// -----------------------------------------------------------------------

class GraphicsObject;
class Point;
class Rect;
class Surface;

// -----------------------------------------------------------------------

/**
 * Describes what is rendered in a graphics object; Subclasses will
 * store image or text data that need to be associated with a
 * GraphicsObject.
 */
class GraphicsObjectData
{
public:
  enum AfterAnimation {
    AFTER_NONE,
    AFTER_CLEAR,
    AFTER_LOOP
  };

public:
  GraphicsObjectData();
  GraphicsObjectData(const GraphicsObjectData& obj);
  virtual ~GraphicsObjectData();

  AfterAnimation afterAnimation() const { return after_animation_; }
  void setAfterAction(AfterAnimation after) { after_animation_ = after; }

  void setOwnedBy(GraphicsObject& godata) { owned_by_ = &godata; }
  GraphicsObject* ownedBy() const { return owned_by_; }

  void setCurrentlyPlaying(bool in) { currently_playing_ = in; }
  bool currentlyPlaying() const { return currently_playing_; }

  virtual void render(const GraphicsObject& go, std::ostream* tree);

  virtual int pixelWidth(const GraphicsObject& rendering_properties) = 0;
  virtual int pixelHeight(const GraphicsObject& rendering_properties) = 0;

  virtual GraphicsObjectData* clone() const = 0;

  virtual void execute() = 0;

  virtual bool isAnimation() const;
  virtual void playSet(int set);

  /// Returns when an animation has completed. (This only returns true when
  /// afterAnimation() is set to AFTER_NONE.)
  bool animationFinished() const;

  /// Whether this object data owns another layer of objects.
  virtual bool isParentLayer() const { return false; }

protected:
  /**
   * Function called after animation ends when this object has been
   * set up to loop.
   *
   * Default implementation does nothing.
   */
  virtual void loopAnimation();

  /**
   * Takes the specified action when we've reached the last frame of
   * animation.
   */
  void endAnimation();

  /**
   * Template method used during rendering to get the surface to render.
   * Return a null shared_ptr to disable rendering.
   */
  virtual boost::shared_ptr<Surface> currentSurface(
    const GraphicsObject& rp) = 0;

  /**
   * Returns the rectangle in currentSurface() to draw to the screen. Override
   * to return custom rectangles in the case of a custom animation format.
   */
  virtual Rect srcRect(const GraphicsObject& go);

  /**
   * Returns the offset to the destination, which is set on a per surface
   * basis. This template method can be ignored if you override dstRect().
   */
  virtual Point dstOrigin(const GraphicsObject& go);

  /**
   * Returns the destination rectangle on the screen to draw srcRect()
   * to. Override to return custom rectangles in the case of a custom animation
   * format.
   */
  virtual Rect dstRect(const GraphicsObject& go);

  /**
   * Controls the alpha during rendering. Default implementation just consults
   * the GraphicsObject.
   */
  virtual int getRenderingAlpha(const GraphicsObject& go);

  /**
   * Prints a description of this object for the RenderTree log.
   */
  virtual void objectInfo(std::ostream& tree) = 0;

private:
  /// Policy of what to do after an animation is finished.
  AfterAnimation after_animation_;

  GraphicsObject* owned_by_;

  bool currently_playing_;

  /// Whether we're on the final frame (and are in AFTER_NONE mode).
  bool animation_finished_;

  friend class boost::serialization::access;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive& ar, unsigned int version) {
    // boost::serialization should take care of the swizzling of
    // owned_by_.
    ar & after_animation_ & owned_by_ & currently_playing_;
  }
};

#endif  // SRC_SYSTEMS_BASE_GRAPHICSOBJECTDATA_HPP_
