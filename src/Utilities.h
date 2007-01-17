
#ifndef __Utilities_h__
#define __Utilities_h__


/** 
 * Returns the full path to a g00 file for the basename of the file.
 * 
 * @param fileName The filename given in the source code.
 * @return The full path of the file
 * @todo This function is currently a hack and does not actually
 *       search for the file; it points directly into my copy of the
 *       Kannon all ages DVD.
 */
inline string findFile(const std::string& fileName)
{
  // Hack until I do this correctly
  string file = "/Users/elliot/KANON_SE_ALL/g00/" + fileName;
  file += ".g00";
  return file;
}

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


#endif
