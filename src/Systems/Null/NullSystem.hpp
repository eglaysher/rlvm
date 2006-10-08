#ifndef __NullSystem_hpp__
#define __NullSystem_hpp__

#include "Systems/Base/System.hpp"
#include "Systems/Null/NullGraphicsSystem.hpp"

/** 
 * The Null system contains absolutely no input/ouput 
 * 
 * 
 * @return 
 */
class NullSystem : public System
{
private:
  NullGraphicsSystem nullGraphicsSystem;

public:
  virtual GraphicsSystem& graphics() { return nullGraphicsSystem; }
};

#endif
