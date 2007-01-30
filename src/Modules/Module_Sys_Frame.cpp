
#include "Module_Sys_Frame.hpp"

#include "Modules/Module_Sys.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/GeneralOperations.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/FrameCounter.hpp"

#include <iostream>
using namespace std;

template<typename FRAMECLASS>
struct Sys_InitFrame 
  : public RLOp_Void<IntConstant_T, IntConstant_T, IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, int counter, int frameMin, int frameMax, 
                  int time)
  {
//    cerr << "BUILDING FRAMECOUNTER: (" << counter << ", " << frameMin << ", " 
//         << frameMax << ", " << time << ")" << endl;
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

struct Sys_ClearFrame_0 : public RLOp_Void<IntConstant_T>
{
  void operator()(RLMachine& machine, int counter)
  {
    EventSystem& es = machine.system().event();
    es.setFrameCounter(counter, NULL);
  }
};

// -----------------------------------------------------------------------

struct Sys_ClearFrame_1 : public RLOp_Void<IntConstant_T, IntConstant_T>
{
  void operator()(RLMachine& machine, int counter, int newValue)
  {
    FrameCounter& fc = machine.system().event().getFrameCounter(counter);
    fc.setActive(false);
    fc.setValue(newValue);
  }
};

// -----------------------------------------------------------------------

struct Sys_ClearAllFrames_0 : public RLOp_Void<IntConstant_T>
{
  void operator()(RLMachine& machine, int newValue)
  {
    EventSystem& es = machine.system().event();

    for(int i = 0; i < 255; ++i)
    {
      if(es.frameCounterExists(i))
      {
        FrameCounter& fc = es.getFrameCounter(i);
        fc.setActive(false);
        fc.setValue(newValue);
      }
    }
  }
};

// -----------------------------------------------------------------------

struct Sys_ClearAllFrames_1 : public RLOp_Void<>
{
  void operator()(RLMachine& machine)
  {
    EventSystem& es = machine.system().event();

    for(int i = 0; i < 255; ++i)
    {
      if(es.frameCounterExists(i))
      {
        es.setFrameCounter(i, NULL);
      }
    }
  }
};

// -----------------------------------------------------------------------

void addSysFrameOpcodes(RLModule& m)
{
  // Normal frame counter operations
  m.addOpcode(500, 0, new Sys_InitFrame<SimpleFrameCounter>);
  m.addOpcode(501, 0, new Sys_InitFrame<LoopFrameCounter>);
  m.addOpcode(502, 0, new Sys_InitFrame<TurnFrameCounter>);
  m.addOpcode(503, 0, new Sys_InitFrame<AcceleratingFrameCounter>);
  m.addOpcode(504, 0, new Sys_InitFrame<DeceleratingFrameCounter>);
  m.addOpcode(510, 0, new Sys_ReadFrame);
  m.addOpcode(511, 0, new Sys_FrameActive);
  m.addOpcode(512, 0, new Sys_AnyFrameActive);
  m.addOpcode(513, 0, new Sys_ClearFrame_0);
  m.addOpcode(513, 1, new Sys_ClearFrame_1);
  m.addOpcode(514, 0, new Sys_ClearAllFrames_0);
  m.addOpcode(514, 1, new Sys_ClearAllFrames_1);

  // Extended frame counter operations

  // Multiple dispatch operations on frame counters
  m.addOpcode(600, 0, "InitFrames", new MultiDispatch(new Sys_InitFrame<SimpleFrameCounter>));
  m.addOpcode(601, 0, "InitFramesLoop",
              new MultiDispatch(new Sys_InitFrame<LoopFrameCounter>));
  m.addOpcode(602, 0, "InitFramesTurn",
              new MultiDispatch(new Sys_InitFrame<TurnFrameCounter>));
  m.addOpcode(603, 0, "InitFramesAccel",
              new MultiDispatch(new Sys_InitFrame<AcceleratingFrameCounter>));
  m.addOpcode(604, 0, "InitFramesDecel",
              new MultiDispatch(new Sys_InitFrame<DeceleratingFrameCounter>));
}
