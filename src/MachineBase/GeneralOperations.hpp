// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------


#ifndef __GeneralOperations_hpp__
#define __GeneralOperations_hpp__

#include "RLOperation.hpp"
#include "RLOperation/RLOp_Store.hpp"
#include "RLOperation/References.hpp"

#include <boost/scoped_ptr.hpp>

#include <string>

// -----------------------------------------------------------------------

class System;
class EventSystem;
class GraphicsSystem;
class TextSystem;
class SoundSystem;
class CGMTable;

// -----------------------------------------------------------------------

/**
 * Contains details on how to get a certain object from an input
 * RLMachine. Templates are specialized in the cpp file.
 */
namespace getSystemObjImpl
{

template<typename RETTYPE>
RETTYPE& getSystemObj(RLMachine& machine);

template<>
inline RLMachine& getSystemObj(RLMachine& machine)
{
  return machine;
}

template<>
System& getSystemObj(RLMachine& machine);

template<>
EventSystem& getSystemObj(RLMachine& machine);

template<>
GraphicsSystem& getSystemObj(RLMachine& machine);

template<>
TextSystem& getSystemObj(RLMachine& machine);

template<>
SoundSystem& getSystemObj(RLMachine& machine);

template<>
CGMTable& getSystemObj(RLMachine& machine);

};

// -----------------------------------------------------------------------

/**
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script.
 */
template<typename OBJTYPE>
class Op_CallWithInt : public RLOp_Void_1< IntConstant_T > {
private:
  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(const int);

  /// The setter function to call on Op_SetToIncoming::reference when
  /// called.
  Setter setter;

public:
  Op_CallWithInt(Setter s)
    : setter(s)
  {}

  void operator()(RLMachine& machine, int incoming)
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(incoming);
  }
};

// -----------------------------------------------------------------------

/**
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script.
 */
template<typename OBJTYPE>
class Op_CallWithMachineInt : public RLOp_Void_1< IntConstant_T > {
private:
  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(RLMachine&, const int);

  /// The setter function to call on Op_SetToIncoming::reference when
  /// called.
  Setter setter;

public:
  Op_CallWithMachineInt(Setter s)
    : setter(s)
  {}

  void operator()(RLMachine& machine, int incoming)
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(machine, incoming);
  }
};

// -----------------------------------------------------------------------

/**
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script.
 */
template<typename OBJTYPE>
class Op_CallWithIntInt : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
private:
  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(const int, const int);

  /// The setter function to call on Op_SetToIncoming::reference when
  /// called.
  Setter setter;

public:
  Op_CallWithIntInt(Setter s)
    : setter(s)
  {}

  void operator()(RLMachine& machine, int incoming1, int incoming2)
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(incoming1, incoming2);
  }
};

// -----------------------------------------------------------------------

/**
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script.
 */
template<typename OBJTYPE>
class Op_CallWithString : public RLOp_Void_1< StrConstant_T > {
private:
  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(const std::string&);

  /// The setter function to call on Op_CallWithString::reference
  /// when called.
  Setter setter;

public:
  Op_CallWithString(Setter s)
    : setter(s)
  {}

  void operator()(RLMachine& machine, std::string incoming)
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(incoming);
  }
};
// -----------------------------------------------------------------------

template<typename OBJTYPE>
class Op_CallMethod : public RLOp_Void_Void {
private:
  /// The string getter function to call
  typedef void(OBJTYPE::*FUNCTYPE)();
  FUNCTYPE func;

public:
  Op_CallMethod(FUNCTYPE f)
    : func(f)
  {}

  void operator()(RLMachine& machine)
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*func)();
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* callFunction(void(OBJTYPE::*s)(const int))
{
  return new Op_CallWithInt<OBJTYPE>(s);
}

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* callFunction(void(OBJTYPE::*s)(RLMachine&, const int))
{
  return new Op_CallWithMachineInt<OBJTYPE>(s);
}

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* callFunction(void(OBJTYPE::*s)(const int, const int))
{
  return new Op_CallWithIntInt<OBJTYPE>(s);
}

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* callFunction(void(OBJTYPE::*s)(const std::string&))
{
  return new Op_CallWithString<OBJTYPE>(s);
}

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* callFunction(void(OBJTYPE::*s)())
{
  return new Op_CallMethod<OBJTYPE>(s);
}

// -----------------------------------------------------------------------

/**
 * Sets an internal variable to a specific value set at compile time,
 * and exposes this as an operation to Reallive scripts.
 */
template<typename OBJTYPE, typename VALTYPE>
class Op_SetToConstant : public RLOp_Void_Void {
private:
  typedef void(OBJTYPE::*Setter)(VALTYPE);
  Setter setter;

  VALTYPE value;

public:
  Op_SetToConstant(Setter s, VALTYPE in_val)
    : setter(s), value(in_val)
  {}

  void operator()(RLMachine& machine)
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(value);
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE, typename VALTYPE>
RLOperation* setToConstant(void(OBJTYPE::*s)(VALTYPE), VALTYPE val)
{
  return new Op_SetToConstant<OBJTYPE, VALTYPE>(s, val);
}

// -----------------------------------------------------------------------

/**
 * Reads the value of an internal variable in a generic way using an
 * arbitrary getter function and places it in the store register.
 */
template<typename OBJTYPE, typename RETTYPE>
class Op_ReturnIntValue : public RLOp_Store_Void {
private:
  typedef RETTYPE(OBJTYPE::*Getter)() const;
  Getter getter_;

public:
  Op_ReturnIntValue(Getter g)
    : getter_(g)
  {}

  int operator()(RLMachine& machine)
  {
    return (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter_)();
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE, typename RETTYPE>
class Op_ReturnIntValueWithString : public RLOp_Store_1<StrConstant_T> {
private:
  typedef RETTYPE(OBJTYPE::*Getter)(const std::string&) const;
  Getter getter_;

public:
  Op_ReturnIntValueWithString(Getter g)
    : getter_(g)
  {}

  int operator()(RLMachine& machine, string one)
  {
    return (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter_)(one);
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE, typename RETTYPE>
RLOperation* returnIntValue(RETTYPE(OBJTYPE::*s)() const)
{
  return new Op_ReturnIntValue<OBJTYPE, RETTYPE>(s);
}

// -----------------------------------------------------------------------

template<typename OBJTYPE, typename RETTYPE>
RLOperation* returnIntValue(RETTYPE(OBJTYPE::*s)(const std::string&) const)
{
  return new Op_ReturnIntValueWithString<OBJTYPE, RETTYPE>(s);
}

// -----------------------------------------------------------------------

/**
 * Reads the value of an internal variable in a generic way using an
 * arbitrary getter function and places it in the store register.
 */
template<typename OBJTYPE>
class Op_ReturnStringValue : public RLOp_Void_1< StrReference_T > {
private:
  /// The signature of a string getter function
  typedef const std::string&(OBJTYPE::*Getter)() const;
  /// The string getter function to call
  Getter getter_;

public:
  Op_ReturnStringValue(Getter g)
    : getter_(g)
  {}

  void operator()(RLMachine& machine, StringReferenceIterator dest)
  {
    *dest = (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter_)();
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* returnStringValue(const std::string&(OBJTYPE::*s)() const)
{
  return new Op_ReturnStringValue<OBJTYPE>(s);
}

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
  boost::scoped_ptr<RLOperation> handler_;

public:
  MultiDispatch(RLOperation* op);
  ~MultiDispatch();

  void parseParameters(const std::vector<std::string>& input,
                       boost::ptr_vector<libReallive::ExpressionPiece>& output);

  void operator()(RLMachine& machine, const libReallive::CommandElement& ff);
};

// -----------------------------------------------------------------------

class ReturnGameexeInt : public RLOp_Store_Void {
private:
  std::string full_key_name_;
  int entry_;

public:
  ReturnGameexeInt(const std::string& full_key, int en);

  int operator()(RLMachine& machine);
};

// -----------------------------------------------------------------------

/**
 * Class that exists simply to print out a prettier output message on
 * unimplemented functions.
 */
class UndefinedFunction : public RLOp_SpecialCase {
private:
  std::string name_;
  int modtype_;
  int module_;
  int opcode_;
  int overload_;

public:
  UndefinedFunction(const std::string& name,
                    int modtype, int module, int opcode, int overload);

  virtual void operator()(RLMachine&, const libReallive::CommandElement&);
};

#endif
