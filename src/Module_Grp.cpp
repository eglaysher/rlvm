
/**
 * @file   Module_Grp.cpp
 * @author Elliot Glaysher
 * @date   Wed Oct  4 16:45:44 2006
 * 
 * @brief  Implements graphical operations.
 * 
 * 
 */


#include "Module_Grp.hpp"
#include "RLOperation.hpp"
//#include "LongOperation.hpp"
#include "RLMachine.hpp"
//#include "GeneralOperations.hpp"

#include <iostream>

using namespace std;

// -----------------------------------------------------------------------

struct Grp_allocDC : public RLOp_Void_3< IntConstant_T, IntConstant_T,
                                         IntConstant_T > {
  void operator()(RLMachine& machine, int dc, int width, int height) {
//    machine.system().graphics().allocDC(dc, width, height);
  }
};

// -----------------------------------------------------------------------

struct Grp_freeDC : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int dc) {
//    machine.system().graphics().freeDC(dc);
  }
};

// -----------------------------------------------------------------------

GrpModule::GrpModule()
  : RLModule("Grp", 1, 033)
{
  addOpcode(15, 0, new Grp_allocDC);
  addOpcode(16, 0, new Grp_freeDC);
}
