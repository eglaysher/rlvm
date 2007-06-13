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

#include "GraphicsObject.hpp"

#include <vector>
#include <string>
#include <iosfwd>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

class Surface;
class RLMachine;

class GanGraphicsObjectData : public AnimatedObjectData
{
private:

  bool m_currentlyPlaying;

  void testFileMagic(const std::string& fileName,
                     boost::scoped_array<char>& ganData, int fileSize);
  void readData(
    const std::string& fileName,
    boost::scoped_array<char>& ganData, int fileSize);

  void throwBadFormat(
    const std::string& filename, const std::string& error);


public:
  GanGraphicsObjectData(RLMachine& machine, const std::string& file);
  ~GanGraphicsObjectData();

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
