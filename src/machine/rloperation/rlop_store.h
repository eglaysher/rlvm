// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef SRC_MACHINE_RLOPERATION_RLOP_STORE_H_
#define SRC_MACHINE_RLOPERATION_RLOP_STORE_H_

#include "machine/rloperation.h"
#include "machine/rlmachine.h"

template <typename... Args>
class RLStoreOpcode : public RLNormalOpcode<Args...> {
 public:
  RLStoreOpcode();
  virtual ~RLStoreOpcode();

  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters);

  virtual int operator()(RLMachine&, typename Args::type...) = 0;

 private:
  template<int... Indexes>
  void DispatchImpl(RLMachine& machine,
                    const std::tuple<typename Args::type...>& args,
                    internal::index_tuple<Indexes...>) {
    int store = operator()(machine, std::get<Indexes>(args)...);
    machine.set_store_register(store);
  }
};

template <typename... Args>
RLStoreOpcode<Args...>::RLStoreOpcode() {}

template <typename... Args>
RLStoreOpcode<Args...>::~RLStoreOpcode() {}

template <typename... Args>
void RLStoreOpcode<Args...>::Dispatch(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& parameters) {
  // The following does not work in gcc 4.8.2, but it's supposed to!
  // Parameter unpacking inside an initializer-clause is supposed to always
  // be evaluated in the order it appears.
  //
  // http://stackoverflow.com/questions/12048221/c11-variadic-template-function-parameter-pack-expansion-execution-order
  unsigned int position = 0;
  std::tuple<typename Args::type...> tuple =
      std::tuple<typename Args::type...>{
    Args::getData(machine, parameters, position)...
  };
  DispatchImpl(machine, tuple,
               typename internal::make_indexes<Args...>::type());
}

template <>
void RLStoreOpcode<>::Dispatch(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& parameters);

extern template class RLStoreOpcode<>;
extern template class RLStoreOpcode<IntConstant_T>;
extern template class RLStoreOpcode<IntConstant_T, IntConstant_T>;
extern template class RLStoreOpcode<IntReference_T>;
extern template class RLStoreOpcode<IntReference_T, IntReference_T>;

#endif  // SRC_MACHINE_RLOPERATION_RLOP_STORE_H_
