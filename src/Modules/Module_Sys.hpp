#ifndef __Module_Sys_hpp__
#define __Module_Sys_hpp__

#include "MachineBase/RLModule.hpp"

class GraphicsSystem;

class SysModule : public RLModule {
public:
  SysModule(GraphicsSystem& system);
};

#endif
