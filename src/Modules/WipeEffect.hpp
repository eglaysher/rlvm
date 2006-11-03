
#ifndef __WipeEffect_hpp__
#define __WipeEffect_hpp__

#include "Modules/Effect.hpp"

class GraphicsSystem;

class WipeEffect : public Effect
{
private:
  /// Which direction we wipe in 
  enum Direction {
    WIPE_TOP_TO_BOTTOM = 0, ///< Wipe from the top to the bottom
    WIPE_BOTTOM_TO_TOP = 1, ///< Wipe from the bottom to the top
    WIPE_LEFT_TO_RIGHT = 2, ///< Wipe from left to right
    WIPE_RIGHT_TO_LEFT = 3  ///< Wipe from right to left
  } m_direction;
  int m_interpolation;
  int m_interpolationInPixels;

  virtual bool blitOriginalImage() const;

  void calculateSizes(int currentTime,
                      int& sizeOfInterpolation, 
                      int& sizeOfMainPolygon,
                      int sizeOfScreen);

  void wipeFromTopToBottom(GraphicsSystem& system, int currentTime);
  void wipeFromBottomToTop(GraphicsSystem& system, int currentTime);
  void wipeFromLeftToRight(GraphicsSystem& system, int currentTime);
  void wipeFromRightToLeft(GraphicsSystem& system, int currentTime);

protected:
  virtual void performEffectForTime(RLMachine& machine,
                                    int currentTime);

public:
  WipeEffect(RLMachine& machine, int x, int y, int width, 
             int height, int dx, int dy, int time, int direction,
             int interpolation);
};

#endif
