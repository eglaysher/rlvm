
#include "intmemref.h"

#include <iostream>
#include <stdexcept>

using namespace std;
using std::ostream;

namespace libReallive {

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(int bytecodeRep, int location)
  : memoryBank(bytecodeRep % 26),  accessType(bytecodeRep / 26),
	loc(location)
{
  if(memoryBank == INTZ_LOCATION_IN_BYTECODE)
	memoryBank = INTZ_LOCATION;
  else if(memoryBank == INTL_LOCATION_IN_BYTECODE)
	memoryBank = INTL_LOCATION;

//  cerr << "#1 Built " << *this << endl;
}

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(int bank, int type, int location)
  : memoryBank(bank), accessType(type), loc(location)
{
//  cerr << "#2 Built " << *this << endl;
}

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(char bankName, int location)
  : accessType(0), loc(location)
{
  if(bankName >= 'A' && bankName <= 'G')
	memoryBank = bankName - 'A';
  else if(bankName == 'Z')
	memoryBank = INTZ_LOCATION;
  else if(bankName == 'L')
	memoryBank = INTL_LOCATION;
  else
	throw std::runtime_error("Invalid memory bank name.");

//  cerr << "#3 Built " << *this << endl;
}

// -----------------------------------------------------------------------

IntMemRef::~IntMemRef()
{}

}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& oss, const libReallive::IntMemRef& memref)
{
  using namespace libReallive;

  oss << "int";

  int bank = memref.bank();
  if(bank >= INTA_LOCATION && bank <= INTG_LOCATION)
	oss << char('A' + memref.bank());
  else if(bank == INTZ_LOCATION)
	oss << 'Z';
  else if(bank == INTL_LOCATION)
	oss << 'L';
  else 
	oss << "{Invalid bank# " << bank << "}";

  if(bank == 0)
	;
  else if(bank == 1)
	oss << "b";
  else if(bank == 2)
	oss << "2b";
  else if(bank == 3)
	oss << "4b";
  else if(bank == 4)
	oss << "8b";

  oss << '[' << memref.location() << ']';

  return oss;
}
