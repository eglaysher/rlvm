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
// the Free Software Foundation; either version 2 of the License, or
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

};

// -----------------------------------------------------------------------

/** 
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script. 
 */
template<typename OBJTYPE, typename RETTYPE>
class Op_SetToIncomingInt : public RLOp_Void_1< IntConstant_T > {
private:
  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(RETTYPE);

  /// The setter function to call on Op_SetToIncoming::reference when
  /// called.
  Setter setter;

public:
  Op_SetToIncomingInt(Setter s)
    : setter(s) 
  {}

  void operator()(RLMachine& machine, int incoming) 
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(incoming);
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE, typename RETTYPE>
RLOperation* setToIncomingInt(void(OBJTYPE::*s)(RETTYPE))
{
  return new Op_SetToIncomingInt<OBJTYPE, RETTYPE>(s);
}


// -----------------------------------------------------------------------

/** 
 * Binds setting an internal variable to a passed in value in from a
 * running Reallive script. 
 */
template<typename OBJTYPE>
class Op_SetToIncomingString : public RLOp_Void_1< StrConstant_T > {
private:
  /// The function signature for the setter function
  typedef void(OBJTYPE::*Setter)(const std::string&);

  /// The setter function to call on Op_SetToIncomingString::reference
  /// when called.
  Setter setter;

public:
  Op_SetToIncomingString(Setter s)
    : setter(s) 
  {}

  void operator()(RLMachine& machine, std::string incoming) 
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*setter)(incoming);
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* setToIncomingString(void(OBJTYPE::*s)(const std::string&))
{
  return new Op_SetToIncomingString<OBJTYPE>(s);
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
  Op_SetToConstant(Setter s, VALTYPE inVal)
    : setter(s), value(inVal)
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
  Getter getter;

public:
  Op_ReturnIntValue(Getter g) 
    : getter(g) 
  {}

  int operator()(RLMachine& machine) 
  {
    return (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter)();
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE, typename RETTYPE>
RLOperation* returnIntValue(RETTYPE(OBJTYPE::*s)() const)
{
  return new Op_ReturnIntValue<OBJTYPE, RETTYPE>(s);
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
  Getter getter;

public:
  Op_ReturnStringValue(Getter g) 
    : getter(g) 
  {}

  void operator()(RLMachine& machine, StringReferenceIterator dest) 
  {
    *dest = (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*getter)();
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* returnStringValue(const std::string&(OBJTYPE::*s)() const)
{
  return new Op_ReturnStringValue<OBJTYPE>(s);
}

// -----------------------------------------------------------------------

template<typename OBJTYPE>
class Op_CallFunction : public RLOp_Void_Void {
private:
  /// The string getter function to call
  typedef void(OBJTYPE::*FUNCTYPE)();
  FUNCTYPE func;

public:
  Op_CallFunction(FUNCTYPE f) 
    : func(f) 
  {}

  void operator()(RLMachine& machine) 
  {
    (getSystemObjImpl::getSystemObj<OBJTYPE>(machine).*func)();
  }
};

// -----------------------------------------------------------------------

template<typename OBJTYPE>
RLOperation* callFunction(void(OBJTYPE::*s)())
{
  return new Op_CallFunction<OBJTYPE>(s);
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
  boost::scoped_ptr<RLOperation> handler;

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
  std::string fullKeyName;
  int entry;

public:
  ReturnGameexeInt(const std::string& fullKey, int en);

  int operator()(RLMachine& machine);
};

// -----------------------------------------------------------------------

/**
 * Class that exists simply to print out a prettier output message on
 * unimplemented functions.
 */
class UndefinedFunction : public RLOp_SpecialCase {
private:
  std::string m_name;
  int m_modtype;
  int m_module;
  int m_opcode;
  int m_overload;

public:
  UndefinedFunction(const std::string& name,
                    int modtype, int module, int opcode, int overload);

  virtual void operator()(RLMachine&, const libReallive::CommandElement&);
};

#endif
