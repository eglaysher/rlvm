
#include "Module_Sys_Frame.hpp"

#include "Modules/Module_Sys.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"

template<typename FRAMECLASS>
struct Sys_InitFrame 
  : public RLOp_Void<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, int counter, int frameMin, int frameMax, 
                  int time)
  {
    EventSystem& es = machine.system().event();
    es.setFrameCounter(counter, new FRAMECLASS(es, frameMin, frameMax, time));
  }
};

// -----------------------------------------------------------------------

struct Sys_ReadFrame : public RLOp_Store<IntConstant_T>
{
  int operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    if(es.frameCounterExists(counter))
      return es.getFrameCounter(counter).readFrame(es);
    else
      return 0;
  }
};

// -----------------------------------------------------------------------

struct Sys_FrameActive : public RLOp_Store<IntConstant_T>
{
  int operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    if(es.frameCounterExists(counter))
      return es.getFrameCounter(counter).isActive();
    else
      return 0;
  }
};

// -----------------------------------------------------------------------

struct Sys_AnyFrameActive : public RLOp_Store<IntConstant_T>
{
  int operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    for(int i = 0; i < 255; ++i) 
    {
      if(es.frameCounterExists(counter) && 
         es.getFrameCounter(counter).isActive())
      {
        return 1;
      }
    }

    return 0;
  }
};

// -----------------------------------------------------------------------

void addSysFrameOpcodes(RLModule& m)
{
  m.addOpcode(500, 0, new Sys_InitFrame<SimpleFrameCounter>);
  m.addOpcode(501, 0, new Sys_InitFrame<LoopFrameCounter>);
  m.addOpcode(502, 0, new Sys_InitFrame<TurnFrameCounter>);
  m.addOpcode(503, 0, new Sys_InitFrame<AcceleratingFrameCounter>);
  m.addOpcode(504, 0, new Sys_InitFrame<DeceleratingFrameCounter>);

  m.addOpcode(510, 0, new Sys_ReadFrame);
  m.addOpcode(511, 0, new Sys_FrameActive);
  m.addOpcode(512, 0, new Sys_AnyFrameActive);
}
