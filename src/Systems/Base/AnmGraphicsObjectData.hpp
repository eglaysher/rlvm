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

#ifndef __AnmGraphicsObjectData_hpp__
#define __AnmGraphicsObjectData_hpp__

#include "Systems/Base/GraphicsObjectData.hpp"
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include <vector>
#include <string>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

class Surface;
class RLMachine;

/**
 * Executable, in-memory representation of an ANM file.
 *
 * @note This internal structure is heavily based off of xkanon's
 *       ANM file implementation, but has been changed to be all C++ like.
 */
class AnmGraphicsObjectData : public GraphicsObjectData {
public:
  AnmGraphicsObjectData();
  AnmGraphicsObjectData(RLMachine& machine, const std::string& file);
  ~AnmGraphicsObjectData();

  void loadAnmFile(RLMachine& machine);

  virtual int pixelWidth(const GraphicsObject& rendering_properties);
  virtual int pixelHeight(const GraphicsObject& rendering_properties);

  virtual GraphicsObjectData* clone() const;
  virtual void execute(RLMachine& machine);

  virtual bool isAnimation() const { return true; }
  virtual void playSet(RLMachine& machine, int set);

protected:
  virtual boost::shared_ptr<Surface> currentSurface(const GraphicsObject& go);
  virtual Rect srcRect(const GraphicsObject& go);
  virtual Rect dstRect(const GraphicsObject& go);

private:
  /// Advance the position in the animation.
  void advanceFrame(RLMachine& machine);

  /**
   * @name Data loading functions
   *
   * @{
   */
  struct Frame {
    int src_x1, src_y1;
    int src_x2, src_y2;
    int dest_x, dest_y;
    int time;
  };

  bool testFileMagic(boost::scoped_array<char>& anm_data);
  void readIntegerList(
    const char* start, int offset, int iterations,
    std::vector< std::vector<int> >& dest);
  void loadAnmFileFromData(
    RLMachine& machine, boost::scoped_array<char>& anm_data);
  void fixAxis(Frame& frame, int width, int height);
  /// @}

  /// Raw, short name for the ANM file.
  std::string filename_;

  /**
   * @name Animation Data
   *
   * (This structure was stolen from xkanon.)
   *
   * @{
   */
  std::vector<Frame> frames;
  std::vector< std::vector<int> > framelist;
  std::vector< std::vector<int> > animation_set;

  /// The image the above coordinates map into.
  boost::shared_ptr<Surface> image;

  /// @}

  /**
   * @name Animation state
   *
   * @{
   */
  bool currently_playing_;

  int current_set_;

  std::vector<int>::const_iterator cur_frame_set_;
  std::vector<int>::const_iterator cur_frame_set_end_;

  std::vector<int>::const_iterator cur_frame_;
  std::vector<int>::const_iterator cur_frame_end_;

  int current_frame_;

  unsigned int time_at_last_frame_change_;

  // @}

  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int file_version) const;

  template<class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};


#endif
