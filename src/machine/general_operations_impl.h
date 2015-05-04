// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2010 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef SRC_MACHINE_GENERAL_OPERATIONS_IMPL_H_
#define SRC_MACHINE_GENERAL_OPERATIONS_IMPL_H_

// Implementation details of the GeneralOperations functions. This internal
// interface is hidden and should only be accessed through the functions
// exposed in general_operations.h.

// Contains details on how to get a certain object from an input
// RLMachine. Templates are specialized in the cc file.
namespace getSystemObjImpl {

template <typename RETTYPE>
RETTYPE& GetSystemObj(RLMachine& machine);

template <>
inline RLMachine& GetSystemObj(RLMachine& machine) {
  return machine;
}

// Equivalent to machine.system().
template <>
System& GetSystemObj(RLMachine& machine);

// Equivalent to machine.system().event().
template <>
EventSystem& GetSystemObj(RLMachine& machine);

// Equivalent to machine.system().graphics().
template <>
GraphicsSystem& GetSystemObj(RLMachine& machine);

// Equivalent to machine.system().text().
template <>
TextSystem& GetSystemObj(RLMachine& machine);

// Equivalent to machine.system().sound().
template <>
SoundSystem& GetSystemObj(RLMachine& machine);

// Equivalent to machine.system().graphics().cg_table().
template <>
CGMTable& GetSystemObj(RLMachine& machine);

// Equivalent to machine.system().text().GetCurrentPage().
template <>
TextPage& GetSystemObj(RLMachine& machine);

}  // namespace getSystemObjImpl

namespace binderImpl {

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template <typename OBJTYPE>
class Op_CallWithInt : public RLOpcode<IntConstant_T> {
 public:
  // The function signature for the setter function
  typedef void (OBJTYPE::*Setter)(const int);

  explicit Op_CallWithInt(Setter s) : setter(s) {}

  virtual void operator()(RLMachine& machine, int in) override {
    (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*setter)(in);
  }

 private:
  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;
};

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template <typename OBJTYPE>
class Op_CallWithMachineInt : public RLOpcode<IntConstant_T> {
 public:
  // The function signature for the setter function
  typedef void (OBJTYPE::*Setter)(RLMachine&, const int);

  explicit Op_CallWithMachineInt(Setter s) : setter(s) {}

  virtual void operator()(RLMachine& machine, int in) override {
    (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*setter)(machine, in);
  }

 private:
  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;
};

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template <typename OBJTYPE>
class Op_CallWithMachineIntInt
    : public RLOpcode<IntConstant_T, IntConstant_T> {
 public:
  // The function signature for the setter function
  typedef void (OBJTYPE::*Setter)(RLMachine&, const int, const int);

  explicit Op_CallWithMachineIntInt(Setter s) : setter(s) {}

  virtual void operator()(RLMachine& machine, int one, int two) override {
    (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*setter)(
        machine, one, two);
  }

 private:
  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;
};

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template <typename OBJTYPE>
class Op_CallWithIntInt : public RLOpcode<IntConstant_T, IntConstant_T> {
 public:
  // The function signature for the setter function
  typedef void (OBJTYPE::*Setter)(const int, const int);

  explicit Op_CallWithIntInt(Setter s) : setter(s) {}

  virtual void operator()(RLMachine& machine, int in1, int in2) override {
    (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*setter)(in1, in2);
  }

 private:
  // The setter function to call on Op_SetToIncoming::reference when
  // called.
  Setter setter;
};

// Binds setting an internal variable to a passed in value in from a
// running Reallive script.
template <typename OBJTYPE>
class Op_CallWithString : public RLOpcode<StrConstant_T> {
 public:
  // The function signature for the setter function
  typedef void (OBJTYPE::*Setter)(const std::string&);

  explicit Op_CallWithString(Setter s) : setter(s) {}

  virtual void operator()(RLMachine& machine, std::string incoming) override {
    (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*setter)(incoming);
  }

 private:
  // The setter function to call on Op_CallWithString::reference
  // when called.
  Setter setter;
};

template <typename OBJTYPE>
class Op_CallMethod : public RLOpcode<> {
 public:
  // The string getter function to call
  typedef void (OBJTYPE::*FUNCTYPE)();

  explicit Op_CallMethod(FUNCTYPE f) : func(f) {}

  virtual void operator()(RLMachine& machine) override {
    (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*func)();
  }

 private:
  FUNCTYPE func;
};

// Sets an internal variable to a specific value set at compile time,
// and exposes this as an operation to Reallive scripts.
template <typename OBJTYPE, typename VALTYPE>
class Op_CallWithConstant : public RLOpcode<> {
 public:
  typedef void (OBJTYPE::*Setter)(VALTYPE);

  Op_CallWithConstant(Setter s, VALTYPE in_val) : setter(s), value(in_val) {}

  virtual void operator()(RLMachine& machine) override {
    (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*setter)(value);
  }

 private:
  Setter setter;
  VALTYPE value;
};

template <typename OBJTYPE, typename VALONE, typename VALTWO>
class Op_CallWithConstantConstant : public RLOpcode<> {
 public:
  typedef void (OBJTYPE::*Setter)(VALONE, VALTWO);

  Op_CallWithConstantConstant(Setter s, VALONE in_valone, VALTWO in_valtwo)
      : setter(s), valone(in_valone), valtwo(in_valtwo) {}

  virtual void operator()(RLMachine& machine) override {
    (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*setter)(valone, valtwo);
  }

 private:
  Setter setter;
  VALONE valone;
  VALTWO valtwo;
};

template <typename RETTYPE>
class Op_ReturnFunctionIntValue : public RLStoreOpcode<> {
 public:
  typedef RETTYPE (*Getter)();

  explicit Op_ReturnFunctionIntValue(Getter g) : getter_(g) {}

  virtual int operator()(RLMachine& machine) override {
    return (*getter_)();
  }

 private:
  Getter getter_;
};

// Reads the value of an internal variable in a generic way using an
// arbitrary getter function and places it in the store register.
template <typename OBJTYPE, typename RETTYPE>
class Op_ReturnIntValue : public RLStoreOpcode<> {
 public:
  typedef RETTYPE (OBJTYPE::*Getter)() const;

  explicit Op_ReturnIntValue(Getter g) : getter_(g) {}

  virtual int operator()(RLMachine& machine) override {
    return (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*getter_)();
  }

 private:
  Getter getter_;
};

template <typename OBJTYPE, typename RETTYPE>
class Op_ReturnIntValueWithInt : public RLStoreOpcode<IntConstant_T> {
 public:
  typedef RETTYPE (OBJTYPE::*Getter)(const int) const;

  explicit Op_ReturnIntValueWithInt(Getter g) : getter_(g) {}

  virtual int operator()(RLMachine& machine, int one) override {
    return (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*getter_)(one);
  }

 private:
  Getter getter_;
};

template <typename OBJTYPE, typename RETTYPE>
class Op_ReturnIntValueWithString : public RLStoreOpcode<StrConstant_T> {
 public:
  typedef RETTYPE (OBJTYPE::*Getter)(const std::string&) const;

  explicit Op_ReturnIntValueWithString(Getter g) : getter_(g) {}

  virtual int operator()(RLMachine& machine, string one) override {
    return (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*getter_)(one);
  }

 private:
  Getter getter_;
};

// Reads the value of an internal variable in a generic way using an
// arbitrary getter function and places it in a passed in reference.
template <typename OBJTYPE>
class Op_ReturnStringValue : public RLOpcode<StrReference_T> {
 public:
  // The signature of a string getter function
  typedef const std::string& (OBJTYPE::*Getter)() const;  // NOLINT

  explicit Op_ReturnStringValue(Getter g) : getter_(g) {}

  virtual void operator()(RLMachine& machine,
                          StringReferenceIterator dest) override {
    *dest = (getSystemObjImpl::GetSystemObj<OBJTYPE>(machine).*getter_)();
  }

 private:
  // The string getter function to call
  Getter getter_;
};

}  // namespace binderImpl

#endif  // SRC_MACHINE_GENERAL_OPERATIONS_IMPL_H_
