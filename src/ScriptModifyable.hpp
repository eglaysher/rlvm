#ifndef __ScriptModifyable_hpp__
#define __ScriptModifyable_hpp__

/** 
 * Alert a class when a value has been modified by one of the
 * GeneralOperations.
 * 
 */
class ScriptModifyable
{
  virtual void alertModified() = 0;
};

#endif
