// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#ifndef __CGMTable_hpp__
#define __CGMTable_hpp__

#include <map>
#include <set>

class RLMachine;

/**
 * Manages whether a certain CG was viewed.
 *
 * The cgm file is a mapping between file names and an index into the intZ[]
 * global memory array, where intZ[index] is 1 when a cg has been viewed. The
 * CGMTable class is responsible for loading the cgm data and providing an
 * interface to querrying whether a CG was viewed.
 *
 * The location of the CGM table is defined in \#CGTABLE_FILENAME gameexe key.
 */
class CGMTable
{
public: 
  CGMTable(Gameexe& gameexe);
  ~CGMTable();

  /// Returns the total number of images designated as CGs.
  int getTotal() const;

  /// Returns the percentage of CG images that have been viewed.
  int getPercent() const;

  /** 
   * Returns the cg index for filename, or -1 if filename is not a CG image.
   */
  int getFlag(const std::string& filename) const;

  /**
   * Returns a value indicating whether filename is a CG that has been viewed:
   *   1   CG has been viewed
   *   0   CG has not been viewed
   *  -1   filename is not a CG image
   */
  int getStatus(const std::string& filename) const;

  /** 
   * Mark a cg as viewed. Sets intZ[getFlag()] to 1.
   */
  void setViewed(RLMachine& machine, const std::string& filename);

private:
  typedef std::map<std::string, int> CGMMap;

  /// Mapping between a graphics file name and the file's cg index.
  CGMMap cgm_info_;

  /// When a CG is viewed, its index is added to this set. This data is
  /// considered global and persists through interpreter invocations.
  std::set<int> cgm_data_;

  // TODO: Add boost::serialization support

};  // end of class CGMTable


#endif
