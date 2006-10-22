
#ifndef __FadeEffect_hpp__
#define __FadeEffect_hpp__

#include "Modules/Effect.hpp"

class FadeEffect : public Effect
{
private:
  virtual bool blitOriginalImage() const;

protected:
  virtual void performEffectForTime(RLMachine& machine,
                                    int currentTime);

public:
  FadeEffect(RLMachine& machine, int x, int y, int width, 
         int height, int dx, int dy, int time);
};

#endif
