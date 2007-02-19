#ifndef __Module_ObjManagement_hpp__
#define __Module_ObjManagement_hpp__

#include "MachineBase/RLModule.hpp"

class ObjCopyFgToBg : public RLModule
{
public:
  ObjCopyFgToBg();
};

class ObjFgManagement : public RLModule
{
public:
  ObjFgManagement();
};

class ObjBgManagement : public RLModule
{
public:
  ObjBgManagement();
};

#endif
