
#include "FadeEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
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

  // Render to the screen
  GraphicsSystem& graphics = machine.system().graphics();
  graphics.beginFrame();
  graphics.getDC(0).
    renderToScreen(x(), y(), x() + width(), y() + height(),
                   dx(), dy(), dx() + width(), dy() + height(),
                   255);
  graphics.getDC(1).
    renderToScreen(x(), y(), x() + width(), y() + height(), 
                   dx(), dy(), dx() + width(), dy() + height(),
                   opacity);
  graphics.endFrame();
}

// -----------------------------------------------------------------------

FadeEffect::FadeEffect(RLMachine& machine, int x, int y, int width, 
                       int height, int dx, int dy, int time)
  : Effect(machine, x, y, width, height, dx, dy, time)
{
}
