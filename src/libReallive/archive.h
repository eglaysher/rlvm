
#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "defs.h"
#include "scenario.h"
#include "filemap.h"

namespace libReallive {
  
/** 
 * Interface to a loaded SEEN.TXT file.
 * 
 */
  class Archive {
  typedef std::map<int, FilePos> scenarios_t;
  typedef std::map<int, Scenario*> accessed_t;
  scenarios_t scenarios;
  accessed_t accessed;
  string name;
  Mapping info;
public:
  Archive(string filename);
  ~Archive();
  
  typedef std::map<int, FilePos>::const_iterator const_iterator;
  const_iterator begin() { return scenarios.begin(); }
  const_iterator end()   { return scenarios.end(); }
  
  /** 
   * Returns a specific scenario 
   * 
   * @param index The SEEN number to return
   * @return The coresponding Scenario if index exists, or NULL if it doesn't.
   */
  Scenario* scenario(int index);
  
  void reset();
  void commit();
  
  /// Rewrites an optimized form of 
  void write_to(string filename);
};

}

#endif
