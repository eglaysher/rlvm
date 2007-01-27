
#include "Module_Sys_Frame.hpp"

#include "Modules/Module_Sys.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"

struct Sys_InitFrame 
  : public RLOp_Void<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, int counter, int frameMin, int frameMax, 
                  int time)
  {
    EventSystem& es = machine.system().event();
    es.setFrameCounter(counter, new FrameCounter(es, frameMin, frameMax, time));
  }
};

// -----------------------------------------------------------------------

struct Sys_ReadFrame : public RLOp_Store<IntConstant_T>
{
  int operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    return es.getFrameCounter(counter).readFrame(es);
  }
};

// -----------------------------------------------------------------------

void addSysFrameOpcodes(RLModule& m)
{
  m.addOpcode(500, 0, new Sys_InitFrame);
  m.addOpcode(510, 0, new Sys_ReadFrame);
}
