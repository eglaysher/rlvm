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

struct RLOp_Store_Void : public RLOp_NormalOperation<> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    int store = operator()(machine);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&) = 0;
};

template <typename A>
struct RLOp_Store_1 : public RLOp_NormalOperation<A> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    int store = operator()(machine, A::getData(machine, parameters, position));
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&, typename A::type) = 0;
};

template <typename A, typename B>
struct RLOp_Store_2 : public RLOp_NormalOperation<A, B> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    int store = operator()(machine, a, b);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&, typename A::type, typename B::type) = 0;
};

template <typename A, typename B, typename C>
struct RLOp_Store_3 : public RLOp_NormalOperation<A, B, C> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type) = 0;
};

template <typename A, typename B, typename C, typename D>
struct RLOp_Store_4 : public RLOp_NormalOperation<A, B, C, D> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type) = 0;
};

template <typename A, typename B, typename C, typename D, typename E>
struct RLOp_Store_5 : public RLOp_NormalOperation<A, B, C, D, E> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F>
struct RLOp_Store_6 : public RLOp_NormalOperation<A, B, C, D, E, F> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G>
struct RLOp_Store_7 : public RLOp_NormalOperation<A, B, C, D, E, F, G> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f, g);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H>
struct RLOp_Store_8 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f, g, h);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I>
struct RLOp_Store_9 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f, g, h, i);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J>
struct RLOp_Store_10
    : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f, g, h, i, j);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K>
struct RLOp_Store_11
    : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f, g, h, i, j, k);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L>
struct RLOp_Store_12
    : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K, L> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f, g, h, i, j, k, l);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M>
struct RLOp_Store_13
    : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K, L, M> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f, g, h, i, j, k, l, m);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N>
struct RLOp_Store_14
    : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K, L, M, N> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    int store = operator()(machine, a, b, c, d, e, f, g, h, i, j, k, l, m, n);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O>
struct RLOp_Store_15
    : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K, L, M, N, O> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    int store = operator()(
        machine, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P>
struct RLOp_Store_16 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    int store = operator()(
        machine, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q>
struct RLOp_Store_17 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    int store = operator()(
        machine, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R>
struct RLOp_Store_18 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    int store = operator()(
        machine, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R,
          typename S>
struct RLOp_Store_19 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R,
                                                   S> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    typename S::type s = S::getData(machine, parameters, position);
    int store = operator()(
        machine, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type,
                         typename S::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R,
          typename S,
          typename T>
struct RLOp_Store_20 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R,
                                                   S,
                                                   T> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    typename S::type s = S::getData(machine, parameters, position);
    typename T::type t = T::getData(machine, parameters, position);
    int store = operator()(
        machine, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type,
                         typename S::type,
                         typename T::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R,
          typename S,
          typename T,
          typename U>
struct RLOp_Store_21 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R,
                                                   S,
                                                   T,
                                                   U> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    typename S::type s = S::getData(machine, parameters, position);
    typename T::type t = T::getData(machine, parameters, position);
    typename U::type u = U::getData(machine, parameters, position);
    int store = operator()(
        machine, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type,
                         typename S::type,
                         typename T::type,
                         typename U::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R,
          typename S,
          typename T,
          typename U,
          typename V>
struct RLOp_Store_22 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R,
                                                   S,
                                                   T,
                                                   U,
                                                   V> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    typename S::type s = S::getData(machine, parameters, position);
    typename T::type t = T::getData(machine, parameters, position);
    typename U::type u = U::getData(machine, parameters, position);
    typename V::type v = V::getData(machine, parameters, position);
    int store = operator()(machine,
                           a,
                           b,
                           c,
                           d,
                           e,
                           f,
                           g,
                           h,
                           i,
                           j,
                           k,
                           l,
                           m,
                           n,
                           o,
                           p,
                           q,
                           r,
                           s,
                           t,
                           u,
                           v);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type,
                         typename S::type,
                         typename T::type,
                         typename U::type,
                         typename V::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R,
          typename S,
          typename T,
          typename U,
          typename V,
          typename W>
struct RLOp_Store_23 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R,
                                                   S,
                                                   T,
                                                   U,
                                                   V,
                                                   W> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    typename S::type s = S::getData(machine, parameters, position);
    typename T::type t = T::getData(machine, parameters, position);
    typename U::type u = U::getData(machine, parameters, position);
    typename V::type v = V::getData(machine, parameters, position);
    typename W::type w = W::getData(machine, parameters, position);
    int store = operator()(machine,
                           a,
                           b,
                           c,
                           d,
                           e,
                           f,
                           g,
                           h,
                           i,
                           j,
                           k,
                           l,
                           m,
                           n,
                           o,
                           p,
                           q,
                           r,
                           s,
                           t,
                           u,
                           v,
                           w);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type,
                         typename S::type,
                         typename T::type,
                         typename U::type,
                         typename V::type,
                         typename W::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R,
          typename S,
          typename T,
          typename U,
          typename V,
          typename W,
          typename X>
struct RLOp_Store_24 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R,
                                                   S,
                                                   T,
                                                   U,
                                                   V,
                                                   W,
                                                   X> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    typename S::type s = S::getData(machine, parameters, position);
    typename T::type t = T::getData(machine, parameters, position);
    typename U::type u = U::getData(machine, parameters, position);
    typename V::type v = V::getData(machine, parameters, position);
    typename W::type w = W::getData(machine, parameters, position);
    typename X::type x = X::getData(machine, parameters, position);
    int store = operator()(machine,
                           a,
                           b,
                           c,
                           d,
                           e,
                           f,
                           g,
                           h,
                           i,
                           j,
                           k,
                           l,
                           m,
                           n,
                           o,
                           p,
                           q,
                           r,
                           s,
                           t,
                           u,
                           v,
                           w,
                           x);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type,
                         typename S::type,
                         typename T::type,
                         typename U::type,
                         typename V::type,
                         typename W::type,
                         typename X::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R,
          typename S,
          typename T,
          typename U,
          typename V,
          typename W,
          typename X,
          typename Y>
struct RLOp_Store_25 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R,
                                                   S,
                                                   T,
                                                   U,
                                                   V,
                                                   W,
                                                   X,
                                                   Y> {
  virtual void Dispatch(
      RLMachine& machine,
      const libreallive::ExpressionPiecesVector& parameters) override {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    typename S::type s = S::getData(machine, parameters, position);
    typename T::type t = T::getData(machine, parameters, position);
    typename U::type u = U::getData(machine, parameters, position);
    typename V::type v = V::getData(machine, parameters, position);
    typename W::type w = W::getData(machine, parameters, position);
    typename X::type x = X::getData(machine, parameters, position);
    typename Y::type y = Y::getData(machine, parameters, position);
    int store = operator()(machine,
                           a,
                           b,
                           c,
                           d,
                           e,
                           f,
                           g,
                           h,
                           i,
                           j,
                           k,
                           l,
                           m,
                           n,
                           o,
                           p,
                           q,
                           r,
                           s,
                           t,
                           u,
                           v,
                           w,
                           x,
                           y);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type,
                         typename S::type,
                         typename T::type,
                         typename U::type,
                         typename V::type,
                         typename W::type,
                         typename X::type,
                         typename Y::type) = 0;
};

template <typename A,
          typename B,
          typename C,
          typename D,
          typename E,
          typename F,
          typename G,
          typename H,
          typename I,
          typename J,
          typename K,
          typename L,
          typename M,
          typename N,
          typename O,
          typename P,
          typename Q,
          typename R,
          typename S,
          typename T,
          typename U,
          typename V,
          typename W,
          typename X,
          typename Y,
          typename Z>
struct RLOp_Store_26 : public RLOp_NormalOperation<A,
                                                   B,
                                                   C,
                                                   D,
                                                   E,
                                                   F,
                                                   G,
                                                   H,
                                                   I,
                                                   J,
                                                   K,
                                                   L,
                                                   M,
                                                   N,
                                                   O,
                                                   P,
                                                   Q,
                                                   R,
                                                   S,
                                                   T,
                                                   U,
                                                   V,
                                                   W,
                                                   X,
                                                   Y,
                                                   Z> {
  void Dispatch(RLMachine& machine,
                const libreallive::ExpressionPiecesVector& parameters) {
    unsigned int position = 0;
    typename A::type a = A::getData(machine, parameters, position);
    typename B::type b = B::getData(machine, parameters, position);
    typename C::type c = C::getData(machine, parameters, position);
    typename D::type d = D::getData(machine, parameters, position);
    typename E::type e = E::getData(machine, parameters, position);
    typename F::type f = F::getData(machine, parameters, position);
    typename G::type g = G::getData(machine, parameters, position);
    typename H::type h = H::getData(machine, parameters, position);
    typename I::type i = I::getData(machine, parameters, position);
    typename J::type j = J::getData(machine, parameters, position);
    typename K::type k = K::getData(machine, parameters, position);
    typename L::type l = L::getData(machine, parameters, position);
    typename M::type m = M::getData(machine, parameters, position);
    typename N::type n = N::getData(machine, parameters, position);
    typename O::type o = O::getData(machine, parameters, position);
    typename P::type p = P::getData(machine, parameters, position);
    typename Q::type q = Q::getData(machine, parameters, position);
    typename R::type r = R::getData(machine, parameters, position);
    typename S::type s = S::getData(machine, parameters, position);
    typename T::type t = T::getData(machine, parameters, position);
    typename U::type u = U::getData(machine, parameters, position);
    typename V::type v = V::getData(machine, parameters, position);
    typename W::type w = W::getData(machine, parameters, position);
    typename X::type x = X::getData(machine, parameters, position);
    typename Y::type y = Y::getData(machine, parameters, position);
    typename Z::type z = Z::getData(machine, parameters, position);
    int store = operator()(machine,
                           a,
                           b,
                           c,
                           d,
                           e,
                           f,
                           g,
                           h,
                           i,
                           j,
                           k,
                           l,
                           m,
                           n,
                           o,
                           p,
                           q,
                           r,
                           s,
                           t,
                           u,
                           v,
                           w,
                           x,
                           y,
                           z);
    machine.set_store_register(store);
  }

  virtual int operator()(RLMachine&,
                         typename A::type,
                         typename B::type,
                         typename C::type,
                         typename D::type,
                         typename E::type,
                         typename F::type,
                         typename G::type,
                         typename H::type,
                         typename I::type,
                         typename J::type,
                         typename K::type,
                         typename L::type,
                         typename M::type,
                         typename N::type,
                         typename O::type,
                         typename P::type,
                         typename Q::type,
                         typename R::type,
                         typename S::type,
                         typename T::type,
                         typename U::type,
                         typename V::type,
                         typename W::type,
                         typename X::type,
                         typename Y::type,
                         typename Z::type) = 0;
};

#endif  // SRC_MACHINE_RLOPERATION_RLOP_STORE_H_
