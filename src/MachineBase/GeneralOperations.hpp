#ifndef __GeneralOperations_hpp__
#define __GeneralOperations_hpp__

#include "RLOperation.hpp"
#include <boost/scoped_ptr.hpp>

/** 
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script. 
 */
template<typename OBJTYPE, typename RETTYPE>
class Op_SetToIncomingInt : public RLOp_Void< IntConstant_T > {
  /// The object we are going to operate on when called.
  OBJTYPE& reference;

  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(RETTYPE);

  /// The setter function to call on Op_SetToIncoming::reference when
  /// called.
  Setter setter;

  Op_SetToIncomingInt(OBJTYPE& ref, Setter s)
    : reference(ref), setter(s) 
  {}

  void operator()(RLMachine& machine, int incoming) 
  {
    ((reference).*(setter)(incoming));
  }
};

// -----------------------------------------------------------------------

/** 
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script. 
 */
template<typename OBJTYPE>
class Op_SetToIncomingString : public RLOp_Void< StrConstant_T > {
private:
  /// The object we are going to operate on when called.
  OBJTYPE& reference;

  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(const std::string&);

  /// The setter function to call on Op_SetToIncomingString::reference
  /// when called.
  Setter setter;

public:
  Op_SetToIncomingString(OBJTYPE& ref, Setter s)
    : reference(ref), setter(s) 
  {}

  void operator()(RLMachine& machine, string incoming) 
  {
    (reference.*setter)(incoming);
  }
};

// -----------------------------------------------------------------------

/** 
 * Sets an internal variable to a specific value set at compile time,
 * and exposes this as an operation to Reallive scripts.
 */
template<typename OBJTYPE, typename RETTYPE, typename VALTYPE>
class Op_SetToConstant : public RLOp_Void<> {
  OBJTYPE& reference;

  typedef void(OBJTYPE::*Setter)(RETTYPE);
  Setter setter;

  VALTYPE value;

  Op_SetToConstant(OBJTYPE& ref, Setter s, VALTYPE inVal)
    : reference(ref), setter(s), value(inVal)
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
class Op_ReturnIntValue : public RLOp_Store<> {
  OBJTYPE& reference;

  typedef void(OBJTYPE::*Getter)(RETTYPE);
  Getter getter;

  Op_ReturnIntValue(OBJTYPE& ref, Getter g) 
    : reference(ref), getter(g) 
  {}

  int operator()(RLMachine& machine) 
  {
    return ((reference).*(getter)());
  }
};

// -----------------------------------------------------------------------

/** 
 * Reads the value of an internal variable in a generic way using an
 * arbitrary getter function and places it in the store register.
 */
template<typename OBJTYPE>
class Op_ReturnStringValue : public RLOp_Void< StrReference_T > {
private:
  /// The object on which we want to call the getter function
  OBJTYPE& reference;

  /// The signature of a string getter function
  typedef const std::string&(OBJTYPE::*Getter)() const;
  /// The string getter function to call
  Getter getter;

public:
  Op_ReturnStringValue(OBJTYPE& ref, Getter g) 
    : reference(ref), getter(g) 
  {}

  void operator()(RLMachine& machine, StringReferenceIterator dest) 
  {
    *dest = (reference.*getter)();
  }
};

// -----------------------------------------------------------------------

/**
 * Special adapter for multiple dispatch versions of operations. This
 * operation structure will take a Argc_T<  >
 *
 * For example, consider the two functions @c InitFrame and @c
 * InitFrames. The following pieces of kepago are equivalent:
 *
 * @code
 * InitFrame(0, 0, 1000, 2500)
 * InitFrame(1, 1000, 0, 2500)
 * @endcode
 *
 * @code
 * InitFrames({0, 0, 1000, 2500}, {1, 1000, 0, 2500))
 * @endcode
 */
class MultiDispatch : public RLOp_SpecialCase {
private:
  boost::scoped_ptr<RLOperation> handler;

public:
  MultiDispatch(RLOperation* op) : handler(op) {}

  void operator()(RLMachine& machine, const libReallive::CommandElement& ff);
};

#endif
