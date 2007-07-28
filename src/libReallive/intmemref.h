#ifndef INTMEMREF_H
#define INTMEMREF_H

#include <iosfwd>

namespace libReallive {

const int INTA_LOCATION = 0;
const int INTB_LOCATION = 1;
const int INTC_LOCATION = 2;
const int INTD_LOCATION = 3;
const int INTE_LOCATION = 4;
const int INTF_LOCATION = 5;
const int INTG_LOCATION = 6;

const int INTZ_LOCATION = 7;
const int INTL_LOCATION = 8;

// -----------------------------------------------------------------------

const int INTZ_LOCATION_IN_BYTECODE = 25;
const int INTL_LOCATION_IN_BYTECODE = 11;

// -----------------------------------------------------------------------

/**
 * References a piece of integer memory.
 */
class IntMemRef
{
private:
  /// Which piece of memory to operate on.
  int memoryBank;

  /// How to access 
  int accessType;

  /// The memory location to 
  int loc;

public:
  IntMemRef(int bytecodeRep, int location);
  IntMemRef(int bank, int type, int location);
  
  IntMemRef(char bankName, int location);
  ~IntMemRef();

  int bank() const { return memoryBank; }
  int type() const { return accessType; }
  int location() const { return loc;    }
};

}

std::ostream& operator<<(std::ostream& oss, const libReallive::IntMemRef& memref);

#endif
