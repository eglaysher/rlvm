
#include "Modules/Module_Sys.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"

#include "Systems/Base/GraphicsSystem.hpp"

#include <cmath>
#include <iostream>

const float PI = 3.14159265;

using namespace std;

struct Sys_rnd_0 : public RLOp_Store< IntConstant_T > {
  int operator()(RLMachine& machine, int var1) {
    return rand() % var1;
  }
};

struct Sys_rnd_1 : public RLOp_Store< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return rand() % (var2 - var1) + var1;
  }
};

struct Sys_pcnt : public RLOp_Store< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int numenator, int denominator) {
    return int( ((float)numenator / (float)denominator) * 100 );
  }
};

struct Sys_abs : public RLOp_Store< IntConstant_T > {
  int operator()(RLMachine& machine, int var) {
    return abs(var);
  }
};

struct Sys_power_0 : public RLOp_Store< IntConstant_T > {
  int operator()(RLMachine& machine, int var) {
    return var * var;
  }
};

struct Sys_power_1 : public RLOp_Store< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return (int)std::pow((float)var1, var2);
  }
};

struct Sys_sin_0 : public RLOp_Store< IntConstant_T > {
  int operator()(RLMachine& machine, int var1) {
    return std::sin(var1 * (PI/180)) * 32640;
  }
};

struct Sys_sin_1 : public RLOp_Store< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::sin(var1 * (PI/180)) * 32640 / var2;
  }
};

struct Sys_modulus : public RLOp_Store< IntConstant_T, IntConstant_T,
                                          IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2, 
                 int var3, int var4) {
    return int( float(var1 - var3) / float(var2 - var4) );
  }
};

struct Sys_angle : public RLOp_Store< IntConstant_T, IntConstant_T,
                                          IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2, 
                 int var3, int var4) {
    return int( float(var1 - var3) / float(var2 - var4) );
  }
};

struct Sys_min : public RLOp_Store< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::min(var1, var2);
  }
};

struct Sys_max : public RLOp_Store< IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2) {
    return std::max(var1, var2);
  }
};

struct Sys_constrain : public RLOp_Store< IntConstant_T, IntConstant_T, IntConstant_T > {
  int operator()(RLMachine& machine, int var1, int var2, int var3) {
    if(var2 < var1)
      return var1;
    else if(var2 > var3)
      return var3;
    else 
      return var2;
  }
};

// ----------------------------------------------------------- Date & Time Functions
struct Sys_GetYear : public RLOp_Store<> {
  int operator()(RLMachine& machine) {
    
  }
};

SysModule::SysModule(GraphicsSystem& system)
  : RLModule("Sys", 1, 004)
{
  addOpcode(1000, 0, new Sys_rnd_0);
  addOpcode(1000, 1, new Sys_rnd_1);
  addOpcode(1001, 0, new Sys_pcnt);
  addOpcode(1002, 0, new Sys_abs);
  addOpcode(1003, 0, new Sys_power_0);
  addOpcode(1003, 1, new Sys_power_1);
  addOpcode(1004, 0, new Sys_sin_0);
  addOpcode(1004, 1, new Sys_sin_1);
  addOpcode(1005, 0, new Sys_modulus);
  addOpcode(1006, 0, new Sys_angle);
  addOpcode(1007, 0, new Sys_min);
  addOpcode(1008, 0, new Sys_max);
  addOpcode(1009, 0, new Sys_constrain);
  // cos 01010
  // sign 01011
  // (unknown) 01012
  // (unknown) 01013

  addOpcode(1130, 0, new Op_ReturnStringValue<GraphicsSystem>(
              system, &GraphicsSystem::defaultGrpName));
  addOpcode(1131, 0, new Op_SetToIncomingString<GraphicsSystem>(
              system, &GraphicsSystem::setDefaultGrpName));
  addOpcode(1132, 0, new Op_ReturnStringValue<GraphicsSystem>(
              system, &GraphicsSystem::defaultBgrName));
  addOpcode(1133, 0, new Op_SetToIncomingString<GraphicsSystem>(
              system, &GraphicsSystem::setDefaultBgrName));

  addOpcode(1100, 0, new Sys_GetYear);
}
