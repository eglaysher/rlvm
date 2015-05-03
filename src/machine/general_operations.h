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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_MACHINE_GENERAL_OPERATIONS_H_
#define SRC_MACHINE_GENERAL_OPERATIONS_H_

#include <memory>
#include <string>
#include <vector>

#include "machine/rloperation.h"
#include "machine/rloperation/rlop_store.h"
#include "machine/rloperation/references.h"

class System;
class EventSystem;
class GraphicsSystem;
class TextPage;
class TextSystem;
class SoundSystem;
class CGMTable;

// Templated implementation details:
#include "machine/general_operations_impl.h"

// CallFunction(), the public interface to all the above binder classes.
template <typename OBJTYPE>
RLOperation* CallFunction(void (OBJTYPE::*s)(const int)) {
  return new binderImpl::Op_CallWithInt<OBJTYPE>(s);
}

template <typename OBJTYPE>
RLOperation* CallFunction(void (OBJTYPE::*s)(RLMachine&, const int)) {
  return new binderImpl::Op_CallWithMachineInt<OBJTYPE>(s);
}

template <typename OBJTYPE>
RLOperation* CallFunction(void (OBJTYPE::*s)(RLMachine&,
                                             const int,
                                             const int)) {
  return new binderImpl::Op_CallWithMachineIntInt<OBJTYPE>(s);
}

template <typename OBJTYPE>
RLOperation* CallFunction(void (OBJTYPE::*s)(const int, const int)) {
  return new binderImpl::Op_CallWithIntInt<OBJTYPE>(s);
}

template <typename OBJTYPE>
RLOperation* CallFunction(void (OBJTYPE::*s)(const std::string&)) {
  return new binderImpl::Op_CallWithString<OBJTYPE>(s);
}

template <typename OBJTYPE>
RLOperation* CallFunction(void (OBJTYPE::*s)()) {
  return new binderImpl::Op_CallMethod<OBJTYPE>(s);
}

// Calls the incoming function with value.
template <typename OBJTYPE, typename VALTYPE>
RLOperation* CallFunctionWith(void (OBJTYPE::*s)(VALTYPE), VALTYPE val) {
  return new binderImpl::Op_CallWithConstant<OBJTYPE, VALTYPE>(s, val);
}

template <typename OBJTYPE, typename VALONE, typename VALTWO>
RLOperation* CallFunctionWith(void (OBJTYPE::*s)(VALONE, VALTWO),
                              VALONE one,
                              VALTWO two) {
  return new binderImpl::Op_CallWithConstantConstant<OBJTYPE, VALONE, VALTWO>(
      s, one, two);
}

// Returns the int value of the passed in function as the store register.
template <typename RETTYPE>
RLOperation* ReturnIntValue(RETTYPE (*s)()) {
  return new binderImpl::Op_ReturnFunctionIntValue<RETTYPE>(s);
}

template <typename OBJTYPE, typename RETTYPE>
RLOperation* ReturnIntValue(RETTYPE (OBJTYPE::*s)() const) {
  return new binderImpl::Op_ReturnIntValue<OBJTYPE, RETTYPE>(s);
}

template <typename OBJTYPE, typename RETTYPE>
RLOperation* ReturnIntValue(RETTYPE (OBJTYPE::*s)(int) const) {
  return new binderImpl::Op_ReturnIntValueWithInt<OBJTYPE, RETTYPE>(s);
}

template <typename OBJTYPE, typename RETTYPE>
RLOperation* ReturnIntValue(RETTYPE (OBJTYPE::*s)(const std::string&) const) {
  return new binderImpl::Op_ReturnIntValueWithString<OBJTYPE, RETTYPE>(s);
}

template <typename OBJTYPE>
RLOperation* returnStringValue(const std::string& (OBJTYPE::*s)()
                               const) {  // NOLINT
  return new binderImpl::Op_ReturnStringValue<OBJTYPE>(s);
}

// Special adapter for multiple Dispatch versions of operations. This
// operation structure will take a Argc_T<  >
//
// For example, consider the two functions @c InitFrame and @c
// InitFrames. The following pieces of kepago are equivalent:
//
// @code
// InitFrame(0, 0, 1000, 2500)
// InitFrame(1, 1000, 0, 2500)
// @endcode
//
// @code
// InitFrames({0, 0, 1000, 2500}, {1, 1000, 0, 2500))
// @endcode
class MultiDispatch : public RLOp_SpecialCase {
 public:
  explicit MultiDispatch(RLOperation* op);
  ~MultiDispatch();

  virtual void ParseParameters(
      const std::vector<std::string>& input,
      libreallive::ExpressionPiecesVector& output) override;

  virtual void operator()(RLMachine& machine,
                          const libreallive::CommandElement& ff);

 private:
  std::unique_ptr<RLOperation> handler_;
};

// Returns a Gameexe value to the store register.
class ReturnGameexeInt : public RLStoreOpcode<> {
 public:
  ReturnGameexeInt(const std::string& full_key, int en);

  virtual int operator()(RLMachine& machine) override;

 private:
  std::string full_key_name_;
  int entry_;
};

// Invokes a syscom command.
class InvokeSyscomAsOp : public RLOpcode<> {
 public:
  explicit InvokeSyscomAsOp(const int syscom);

  virtual void operator()(RLMachine& machine) override;

 private:
  int syscom_;
};

// Class that exists simply to print out a prettier output message on
// unimplemented functions.
class UndefinedFunction : public RLOp_SpecialCase {
 public:
  UndefinedFunction(int modtype,
                    int module,
                    int opcode,
                    int overload);

  // A note on UGLY HACKS: We need to override RLOp_SpecialCase::Dispatch()
  // because that's the entry point when using ChildObjAdapter. So we overload
  // all these methods so we error as early as possible when trying to use this
  // invalid opcode.

  // RLOp_SpecialCase:
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override;
  virtual void DispatchFunction(RLMachine& machine,
                                const libreallive::CommandElement& f) override;
  virtual void ParseParameters(
      const std::vector<std::string>& input,
      libreallive::ExpressionPiecesVector& output) override;
  virtual void operator()(RLMachine&,
                          const libreallive::CommandElement&) override;

 private:
  int modtype_;
  int module_;
  int opcode_;
  int overload_;
};

// extern template declarations; manually instantiated in the cc file.
extern template RLOperation* CallFunction<EventSystem>(void (EventSystem::*)(int));
extern template RLOperation* CallFunction<GraphicsSystem>(void (GraphicsSystem::*)(int));
extern template RLOperation* CallFunction<SoundSystem>(void (SoundSystem::*)(int));
extern template RLOperation* CallFunction<System>(void (System::*)(int));
extern template RLOperation* CallFunction<TextSystem>(void (TextSystem::*)(int));
extern template RLOperation* ReturnIntValue<EventSystem, int>(int (EventSystem::*)() const);
extern template RLOperation* ReturnIntValue<GraphicsSystem, int>(int (GraphicsSystem::*)() const);
extern template RLOperation* ReturnIntValue<RLMachine, int>(int (RLMachine::*)() const);
extern template RLOperation* ReturnIntValue<SoundSystem, int>(int (SoundSystem::*)() const);
extern template RLOperation* ReturnIntValue<TextSystem, int>(int (TextSystem::*)() const);

#endif  // SRC_MACHINE_GENERAL_OPERATIONS_H_
