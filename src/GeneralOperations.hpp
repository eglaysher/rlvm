#ifndef __GeneralOperations_hpp__
#define __GeneralOperations_hpp__

#include "RLOperation.hpp"

/** 
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script. 
 */
template<typename OBJTYPE, typename RETTYPE>
class Op_SetToIncoming : public RLOp_Void_1< IntConstant_T > {
  /// The object we are going to operate on when called.
  OBJTYPE& reference;

  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(RETTYPE);

  /// The setter function to call on Op_SetToIncoming::reference when
  /// called.
  Setter setter;

  Op_SetToIncoming(OBJTYPE& ref, Setter s)
    : RLOp_Void_1(), reference(ref), setter(s) 
  {}

  void operator()(RLMachine& machine, int incoming) 
  {
    ((reference).*(setter)(incoming));
  }
};

// -----------------------------------------------------------------------

/** 
 * Sets an internal variable to a specific value set at compile time,
 * and exposes this as an operation to Reallive scripts.
 */
template<typename OBJTYPE, typename RETTYPE, typename VALTYPE>
class Op_SetToConstant : public RLOp_Void_Void {
  OBJTYPE& reference;

  typedef void(OBJTYPE::*Setter)(RETTYPE);
  Setter setter;

  VALTYPE value;

  Op_SetToIncoming(OBJTYPE& ref, Setter s, VALTYPE inVal)
    : RLOp_Void_Void(), reference(ref), setter(s), 
      value(inVal)
  {}

  void operator()(RLMachine& machine) 
  {
    ((reference).*(setter)(value));
  }
};

// -----------------------------------------------------------------------

/** 
 * Reads the value of an internal variable in a generic way using an
 * arbitrary getter function and places it in the store register.
 */
template<typename OBJTYPE, typename RETTYPE>
class Op_ReturnValue : public RLOp_Store_Void {
  TYPE& reference;

  typedef void(OBJTYPE::*Getter)(RETTYPE);
  Getter getter;

  Op_ReturnValue(OBJTYPE& ref, Getter g) 
    : RLOp_Store_Void(), reference(ref), getter(g) 
  {}

  int operator()(RLMachine& machine) 
  {
    return ((reference).*(getter)());
  }
};

#endif
