// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef __Utilities_h__
#define __Utilities_h__

#include <string>
#include <vector>
#include <iosfwd>

#include <boost/scoped_array.hpp>

class RLMachine;
class Gameexe;

/**
 * Takes the full path to a file, and adjusts its case so that it
 * points to an existing file if possible.
 *
 * @param fileName The path to correct the case of.
 * @return On platforms with case-insensitive file systems, returns
 *         a copy of the input unchanged. On less tolerant platforms,
 *         returns a copy of the input with correct case, or the empty
 *         string if no solution could be found.
 */
std::string correctPathCase(const std::string& fileName);

// -----------------------------------------------------------------------

/**
 * @name File type constants.
 * 
 * These constant, externed vectors are passed as parameters to
 * findFile to control which file types are searched for. Defaults to
 * all.
 *
 * @{
 */
extern const std::vector<std::string> ALL_FILETYPES;
extern const std::vector<std::string> IMAGE_FILETYPES;
extern const std::vector<std::string> GAN_FILETYPES;
/// @}

/** 
 * Returns the full path to a g00 file for the basename of the file.
 * 
 * @param fileName The filename given in the source code.
 * @return The full path of the file
 */
std::string findFile(RLMachine& machine, const std::string& fileName,
                     const std::vector<std::string>& extensions = ALL_FILETYPES);

// -----------------------------------------------------------------------

/**
 * Attempts to find the file fileName in the home directory
 */
std::string findFontFile(const std::string& fileName);

// -----------------------------------------------------------------------

/** 
 * Changes the coordinate types. All operations internally are done in
 * rec coordinates, (x, y, width, height). The GRP functions pass
 * parameters of the format (x1, y1, x2, y2).
 * 
 * @param x1 X coordinate. Not changed by this function
 * @param y1 Y coordinate. Not changed by this function
 * @param x2 X2. In place changed to width.
 * @param y2 Y2. In place changed to height.
 */
inline void grpToRecCoordinates(int x1, int y1, int& x2, int& y2)
{
  x2 = x2 - x1;
  y2 = y2 - y1;
}

// -----------------------------------------------------------------------

/** 
 * Will search for a #SEL.selNum (and translate from grp to rec
 * coordinates), or #SELR.selNum if a #SEL version isn't found in the
 * gameexe.ini file.
 * 
 * @return #SEL in rec coordinates
 */
std::vector<int> getSELEffect(RLMachine& machine, int selNum);

// -----------------------------------------------------------------------

/**
 * Gets the size of the screen and sets it in width/height.
 */
void getScreenSize(Gameexe& gameexe, int& width, int& height);

// -----------------------------------------------------------------------

/**
 * Clamp var between [min, max].
 */
void clamp(float& var, float min, float max);

// -----------------------------------------------------------------------

bool loadFileData(std::ifstream& ifs, 
                  boost::scoped_array<char>& anmData, 
                  int& fileSize);

// -----------------------------------------------------------------------

namespace rlvm {

class Exception : public std::exception
{
private:
  std::string description;
public:
  virtual const char* what() const throw();
  Exception(std::string what);
  virtual ~Exception() throw();
};

}

#endif
