// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef __AnimationObjectData_hpp__
#define __AnimationObjectData_hpp__

class RLMachine;
class GraphicsObject;

// -----------------------------------------------------------------------

/**
 * Describes what is rendered in a graphics object; Subclasses will
 * store image or text data that need to be associated with a
 * GraphicsObject.
 */
class GraphicsObjectData {
public:
  virtual ~GraphicsObjectData();
  virtual void render(RLMachine& machine, 
                      const GraphicsObject& renderingProperties) = 0;
  
  virtual int pixelWidth(RLMachine& machine,
						 const GraphicsObject& renderingProperties) = 0;
  virtual int pixelHeight(RLMachine& machine,
						  const GraphicsObject& renderingProperties) = 0;

  virtual GraphicsObjectData* clone() const = 0;

  virtual void execute(RLMachine& machine) { }
  virtual void setOwnedBy(GraphicsObject& godata) { }

  virtual bool isAnimation() const;
};

// -----------------------------------------------------------------------

/**
 * Describes additional data and methods needed by objects that
 * represent animations.
 */
class AnimatedObjectData : public GraphicsObjectData 
{
public:
  enum AfterAnimation {
    AFTER_NONE,
    AFTER_CLEAR
  };

private:
  AfterAnimation m_afterAnimation;
  GraphicsObject* m_objectToCleanupOn;

public:
  AnimatedObjectData();
  ~AnimatedObjectData();

  AfterAnimation afterAnimation() const { return m_afterAnimation; }
  void setAfterAction(AfterAnimation after) { m_afterAnimation = after; }

  virtual void setOwnedBy(GraphicsObject& godata) 
  { m_objectToCleanupOn = &godata; }
  GraphicsObject* ownedBy() const { return m_objectToCleanupOn; }

  virtual bool isAnimation() const;

  virtual bool isPlaying() const = 0;

  virtual void playSet(RLMachine& machine, int set) = 0;
};

// -----------------------------------------------------------------------

/**
 * Some graphics objects override the position or other properties
 * from GraphicsObject while rendering. This struct permits the
 * optional passing of this data into Surface::renderToScreenAsObject().
 *
 * @see Surface
 */
struct GraphicsObjectOverride
{
  GraphicsObjectOverride();

  void setOverrideSource(int insrcX1, int insrcY1, int insrcX2, int insrcY2);
  void setDestOffset(int indstX, int indstY);
  void setOverrideDestination(int indstX1, int indstY1, int indstX2, int indstY2);
  void setAlphaOverride(int inalpha);

  bool overrideSource;
  bool overrideDest;
  bool hasDestOffset;
  bool hasAlphaOverride;
  int srcX1, srcY1, srcX2, srcY2;
  int dstX, dstY;
  int dstX1, dstY1, dstX2, dstY2;
  int alpha;
};

#endif
