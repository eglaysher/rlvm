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

#ifndef __GanGraphicsObjectData_hpp__
#define __GanGraphicsObjectData_hpp__

#include "Systems/Base/GraphicsObjectData.hpp"

#include <vector>
#include <string>
#include <iosfwd>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

class Surface;
class RLMachine;
class GraphicsObject;

// -----------------------------------------------------------------------

/**
 * In-memory representation of a GAN file. Responsible for reading in,
 * storing, and rendering GAN data as a GraphicsObjectData.
 */
class GanGraphicsObjectData : public GraphicsObjectData
{
private:
  struct Frame {
    int pattern;
    int x;
    int y;
    int time;
    int alpha;
    int other; ///< WTF?
  };

  typedef std::vector< std::vector<Frame> > AnimationSets;
  AnimationSets animationSets;

  int m_currentSet;
  int m_currentFrame;
  int m_timeAtLastFrameChange;

  /// The image the above coordinates map into.
  boost::shared_ptr<Surface> image;

  void testFileMagic(const std::string& fileName,
                     boost::scoped_array<char>& ganData, int fileSize);
  void readData(
    RLMachine& machine,
    const std::string& fileName,
    boost::scoped_array<char>& ganData, int fileSize);
  Frame readSetFrame(const std::string& filename, const char*& data);

  void throwBadFormat(
    const std::string& filename, const std::string& error);

protected:

  virtual void loopAnimation();

public:
  GanGraphicsObjectData(RLMachine& machine, const std::string& file,
                        const boost::shared_ptr<Surface>& incomingImage);
  ~GanGraphicsObjectData();

  virtual void render(RLMachine& machine, 
                      const GraphicsObject& renderingProperties);
  
  virtual int pixelWidth(RLMachine& machine, 
						 const GraphicsObject& renderingProperties);
  virtual int pixelHeight(RLMachine& machine, 
						  const GraphicsObject& renderingProperties);

  virtual GraphicsObjectData* clone() const;  
  virtual void execute(RLMachine& machine);

  virtual bool isAnimation() const { return true; }
  virtual void playSet(RLMachine& machine, int set);
};

#endif
