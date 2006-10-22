



LongOperation* EffectFactory::build(string filename, Gameexe& exe, int effectNum)
{
  // 

  // 
  return build(   );
}

/** 
 * Returns a constructed LongOperation with the following properties
 * to perform a transition.
 * 
 * @param filename      Image file to use
 * @param x             Source x coordinate
 * @param y             Source y coordinate
 * @param width         Source width
 * @param height        Source hight
 * @param dx            Destination x coordinate
 * @param dy            Destination y coordinate
 * @param time          Length of transition, in ms.
 * @param style         The style of transition. This factory does a
 *                      big switch statement on this value.
 * @param direction     For wipes and pans, sets the wipe direction.
 * @param interpolation Smooths certain transitions. For For dithered
 *                      fades, interpolation N adds N intermediate
 *                      steps so that the transition fades between
 *                      patterns rather than stepping between them.
 *                      For wipes, interpolation N replaces the hard
 *                      boundary with a soft edge of thickness roughly
 *                      2^N * 2.5 pixels.
 * @param xsize         X size of pattern in some transitions
 * @param ysize         Y size of pattern in some transitions
 * @param a             Unknown
 * @param b             Unknown
 * @param opacity       Opacity of the new image composited onto the
 *                      old image in DC1. (DC1 is then copied onto DC0
 *                      with this transition object.)
 * @param c             Unknown
 * 
 * @return A LongOperation which will perform the following transition
 *         and then exit.
 */
LongOperation* EffectFactory::build(const std::string& filename)
  int x, int y, int width, int height, int dx, int dy, int time, int style,
  int direction, int interpolation, int xsize, int ysize, int a, int b,
  int opacity, int c)
{
  // There is a completely ridaculous number of transitions here! Damn
  // you, VisualArts, for making something so simple sounding so
  // confusing and hard to implement!
  switch(style)
  {
  case 0: 
    return SimpleFadeEffect(x, y, width, height, dx, dy, time);
    

  }

}
