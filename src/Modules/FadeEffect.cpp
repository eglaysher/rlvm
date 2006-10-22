
#include "FadeEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include <iostream>

using namespace std;

// -----------------------------------------------------------------------

bool FadeEffect::blitOriginalImage() const
{
  return true; 
}

// -----------------------------------------------------------------------

void FadeEffect::performEffectForTime(RLMachine& machine,
                                      int currentTime)
{
  // Blit the source image to the screen with the opacity
  int opacity = (float(currentTime) / duration()) * 255;

  machine.system().graphics().
    blitSurfaceToDC(dc1(), 0,
                    x(), y(), width(), height(), 
                    dx(), dy(), width(), height(),
                    opacity);
}

// -----------------------------------------------------------------------

FadeEffect::FadeEffect(RLMachine& machine, int x, int y, int width, 
                       int height, int dx, int dy, int time)
  : Effect(machine, x, y, width, height, dx, dy, time)
{
}
