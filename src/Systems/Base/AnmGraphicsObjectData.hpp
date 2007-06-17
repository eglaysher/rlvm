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

#ifndef __AnmGraphicsObjectData_hpp__
#define __AnmGraphicsObjectData_hpp__

//#include "GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"

#include <vector>
#include <string>
#include <iosfwd>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

class Surface;
class RLMachine;

class AnmGraphicsObjectData : public AnimatedObjectData
{
private:
  /// Note: This internal structure is heavily based off of xkanon's
  /// ANM file implementation, but has been changed to be all C++ like.

  /**
   * @name Animation Data
   * 
   * (This structure was stolen from xkanon.)
   *
   * @{
   */
  struct Frame {
    int src_x1, src_y1;
    int src_x2, src_y2;
    int dest_x, dest_y;
    int time;
  };

  std::vector<Frame> frames;
  std::vector< std::vector<int> > framelist;
  std::vector< std::vector<int> > animationSet;

  /// The image the above coordinates map into.
  boost::shared_ptr<Surface> image;

  /// @}

  /**
   * @name Animation state
   * 
   * @{
   */
  bool m_currentlyPlaying;

  std::vector<int>::iterator m_curFrameSet;
  std::vector<int>::iterator m_curFrameSetEnd;

  std::vector<int>::iterator m_curFrame;
  std::vector<int>::iterator m_curFrameEnd;

  int m_currentFrame;

  unsigned int m_timeAtLastFrameChange;

  // @}

  void advanceFrame(RLMachine& machine);


  /** 
   * @name Data loading functions
   * 
   * @{
   */
  bool testFileMagic(boost::scoped_array<char>& anmData);
  void readIntegerList(
    const char* start, int offset, int iterations, 
    std::vector< std::vector<int> >& dest);
  void loadAnmFileFromData(
    RLMachine& machine, boost::scoped_array<char>& anmData);
  void fixAxis(Frame& frame, int width, int height);
  /// @}

public:
  AnmGraphicsObjectData(RLMachine& machine, const std::string& file);
  ~AnmGraphicsObjectData();

  virtual void render(RLMachine& machine, 
                      const GraphicsObject& renderingProperties);
  
  virtual int pixelWidth(const GraphicsObject& renderingProperties);
  virtual int pixelHeight(const GraphicsObject& renderingProperties);

  virtual GraphicsObjectData* clone() const;  
  virtual void execute(RLMachine& machine);

  // ------------------------------------ [ AnimatedObjectData interface ]
  virtual bool isPlaying() const;
  virtual void playSet(RLMachine& machine, int set);
};


#endif
