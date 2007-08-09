// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef __RLOp_Store_hpp__
#define __RLOp_Store_hpp__

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"

// -----------------------------------------------------------------------

struct RLOp_Store_Void : public RLOp_NormalOperation<>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
    int store = operator()(machine);
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&) = 0;
};

// -----------------------------------------------------------------------

template<typename A>
struct RLOp_Store_1 : public RLOp_NormalOperation<A>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  { 
    int store = operator()(machine, A::getData(machine, parameters, 0));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B>
struct RLOp_Store_2 : public RLOp_NormalOperation<A, B>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
    int store = operator()(machine, A::getData(machine, parameters, 0),
               B::getData(machine, parameters, 1));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type) = 0;  
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C>
struct RLOp_Store_3 : public RLOp_NormalOperation<A, B, C>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D>
struct RLOp_Store_4 : public RLOp_NormalOperation<A, B, C, D>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type) = 0;
};

// -----------------------------------------------------------------------


template<typename A, typename B, typename C, typename D, typename E>
struct RLOp_Store_5 : public RLOp_NormalOperation<A, B, C, D, E>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type) = 0;
};

// -----------------------------------------------------------------------


template<typename A, typename B, typename C, typename D, typename E, 
         typename F>
struct RLOp_Store_6 : public RLOp_NormalOperation<A, B, C, D, E, F>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G>
struct RLOp_Store_7 : public RLOp_NormalOperation<A, B, C, D, E, F, G>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H>
struct RLOp_Store_8 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I>
struct RLOp_Store_9 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J>
struct RLOp_Store_10 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K>
struct RLOp_Store_11 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L>
struct RLOp_Store_12 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K, L>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M>
struct RLOp_Store_13 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K, L, M>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N>
struct RLOp_Store_14 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type) = 0;
};

// -----------------------------------------------------------------------


template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O>
struct RLOp_Store_15 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P>
struct RLOp_Store_16 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type) = 0;
};

// -----------------------------------------------------------------------


template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q>
struct RLOp_Store_17 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R>
struct RLOp_Store_18 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S>
struct RLOp_Store_19 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T>
struct RLOp_Store_20 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U>
struct RLOp_Store_21 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V>
struct RLOp_Store_22 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V, typename W>
struct RLOp_Store_23 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V,
                                                  W>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21),
                 W::getData(machine, parameters, 22));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type, 
                          typename W::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V, typename W, typename X>
struct RLOp_Store_24 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V,
                                                  W, X>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21),
                 W::getData(machine, parameters, 22),
                 X::getData(machine, parameters, 23));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type, 
                          typename W::type, typename X::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V, typename W, typename X, typename Y>
struct RLOp_Store_25 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V,
                                                  W, X, Y>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21),
                 W::getData(machine, parameters, 22),
                 X::getData(machine, parameters, 23),
                 Y::getData(machine, parameters, 24));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type,typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type, 
                          typename W::type, typename X::type,
                          typename Y::type) = 0;
};

// -----------------------------------------------------------------------

template<typename A, typename B, typename C, typename D, typename E, 
         typename F, typename G, typename H, typename I, typename J,
         typename K, typename L, typename M, typename N, typename O,
         typename P, typename Q, typename R, typename S, typename T,
         typename U, typename V, typename W, typename X, typename Y,
         typename Z>
struct RLOp_Store_26 : public RLOp_NormalOperation<A, B, C, D, E, F, G, H, I, J, K,
                                                  L, M, N, O, P, Q, R, S, T, U, V,
                                                  W, X, Y, Z>
{
  void dispatch(RLMachine& machine, 
                const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) 
  {
      int store = operator()(machine, A::getData(machine, parameters, 0),
                 B::getData(machine, parameters, 1),
                 C::getData(machine, parameters, 2),
                 D::getData(machine, parameters, 3),
                 E::getData(machine, parameters, 4),
                 F::getData(machine, parameters, 5),
                 G::getData(machine, parameters, 6),
                 H::getData(machine, parameters, 7),
                 I::getData(machine, parameters, 8),
                 J::getData(machine, parameters, 9),
                 K::getData(machine, parameters, 10),
                 L::getData(machine, parameters, 11),
                 M::getData(machine, parameters, 12),
                 N::getData(machine, parameters, 13),
                 O::getData(machine, parameters, 14),
                 P::getData(machine, parameters, 15),
                 Q::getData(machine, parameters, 16),
                 R::getData(machine, parameters, 17),
                 S::getData(machine, parameters, 18),
                 T::getData(machine, parameters, 19),
                 U::getData(machine, parameters, 20),
                 V::getData(machine, parameters, 21),
                 W::getData(machine, parameters, 22),
                 X::getData(machine, parameters, 23),
                 Y::getData(machine, parameters, 24),
                 Z::getData(machine, parameters, 25));
    machine.setStoreRegister(store);
  }

  virtual int operator()(RLMachine&, typename A::type, typename B::type, typename C::type, 
                          typename D::type, typename E::type, typename F::type, typename G::type,
                          typename H::type, typename I::type, typename J::type, typename K::type,
                          typename L::type, typename M::type, typename N::type,
                          typename O::type, typename P::type, typename Q::type,
                          typename R::type, typename S::type, typename T::type,
                          typename U::type, typename V::type, 
                          typename W::type, typename X::type,
                          typename Y::type, typename Z::type) = 0;
};


#endif
