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

#ifndef __GraphicsTextObject_hpp__
#define __GraphicsTextObject_hpp__

#include <boost/shared_ptr.hpp>

#include "Systems/Base/GraphicsObjectData.hpp"

class GraphicsObject;
class Surface;

/**
 * Represents the textual data made with commands such as objOfText,
 * objSetText, objTextOpts, et cetera.
 */
class GraphicsTextObject : public GraphicsObjectData
{
private:
  std::string m_cachedUtf8str;

  boost::shared_ptr<Surface> m_surface;

  /** 
   * Redraw m_surface.
   */
  void updateSurface(RLMachine& machine,
                     GraphicsObject& rp);

  bool needsUpdate(GraphicsObject& renderingProperties);

public:
  GraphicsTextObject(RLMachine& machine);
  ~GraphicsTextObject();

  // ------------------------------------ [ GraphicsObjectData interface ]
  virtual void render(RLMachine& machine, 
                      GraphicsObject& renderingProperties);
  
  virtual int pixelWidth(RLMachine& machine, 
                         GraphicsObject& renderingProperties);
  virtual int pixelHeight(RLMachine& machine,
                          GraphicsObject& renderingProperties);

  virtual GraphicsObjectData* clone() const;
};

#endif
