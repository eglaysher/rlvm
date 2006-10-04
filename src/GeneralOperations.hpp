#ifndef __GeneralOperations_hpp__
#define __GeneralOperations_hpp__

#include "RLOperation.hpp"

/** 
 * Binds setting an internal variable to a passed in value in a
 * generic way.
 */
template<typename TYPE>
class Op_SetToIncoming : public RLOp_Void_1< IntConstant_T > {
  TYPE& reference;
  ScriptModifyable& objToAlert;

  Op_SetToIncoming(TYPE& ref, ScriptModifyable& in) 
    : reference(ref), objToAlert(in) {}

  void operator()(RLMachine& machine, int incoming) {
    reference = incoming;
    objToAlert->alertModified();
  }
};

// -----------------------------------------------------------------------

/** 
 * Binds setting an internal variable to true in a
 * generic way.
 */
template<typename TYPE>
class Op_SetToTrue : public RLOp_Void_Void {
  TYPE& reference;
  ScriptModifyable& objToAlert;

  Op_SetToTrue(TYPE& ref, ScriptModifyable& in) 
    : reference(ref), objToAlert(in) {}

  void operator()(RLMachine& machine) {
    reference = 1;
    objToAlert->alertModified();
  }
};

// -----------------------------------------------------------------------

/** 
 * Binds setting an internal variable to false in a generic way.
 */
template<typename TYPE>
class Op_SetToFalse : public RLOp_Void_Void {
  TYPE& reference;
  ScriptModifyable& objToAlert;

  Op_SetToFalse(TYPE& ref, ScriptModifyable& in)
    : reference(ref), objToAlert(in) {}

  void operator()(RLMachine& machine) {
    reference = 0;
    objToAlert->alertModified();
  }
};

// -----------------------------------------------------------------------

/** 
 * Reads the value of an internal variable in a generic way.
 */
template<typename TYPE>
class Op_ReturnValue : public RLOp_Store_Void {
  TYPE& reference;

  Op_ReturnValue(TYPE& ref) : reference(ref) {}

  int operator()(RLMachine& machine) {
    return reference;
  }
};


#endif
