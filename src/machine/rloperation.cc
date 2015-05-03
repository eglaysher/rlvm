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

#include "machine/rloperation.h"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "libreallive/bytecode.h"
#include "machine/rloperation/references.h"
#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "utilities/exception.h"

// -----------------------------------------------------------------------
// RLOperation
// -----------------------------------------------------------------------

RLOperation::RLOperation() : name_() {}

RLOperation::~RLOperation() {}

RLOperation* RLOperation::SetProperty(int property, int value) {
  if (!property_list_) {
    property_list_.reset(new std::vector<std::pair<int, int>>);
  }

  // Modify the property if it already exists
  PropertyList::iterator it = FindProperty(property);
  if (it != property_list_->end()) {
    it->second = value;
  } else {
    property_list_->emplace_back(property, value);
  }

  return this;
}

bool RLOperation::GetProperty(int property, int& value) const {
  if (property_list_) {
    PropertyList::iterator it = FindProperty(property);
    if (it != property_list_->end()) {
      value = it->second;
      return true;
    }
  }

  if (module_) {
    // If we don't have a property, ask our module if it has one.
    return module_->GetProperty(property, value);
  }

  return false;
}

RLOperation::PropertyList::iterator RLOperation::FindProperty(int property)
    const {
  return find_if(property_list_->begin(),
                 property_list_->end(),
                 [&](Property& p) { return p.first == property; });
}

bool RLOperation::AdvanceInstructionPointer() { return true; }

void RLOperation::DispatchFunction(RLMachine& machine,
                                   const libreallive::CommandElement& ff) {
  if (!ff.AreParametersParsed()) {
    std::vector<std::string> unparsed = ff.GetUnparsedParameters();
    libreallive::ExpressionPiecesVector output;
    ParseParameters(unparsed, output);
    ff.SetParsedParameters(std::move(output));
  }

  const libreallive::ExpressionPiecesVector& parameter_pieces =
      ff.GetParsedParameters();

  // Now Dispatch based on these parameters.
  Dispatch(machine, parameter_pieces);

  // By default, we advacne the instruction pointer on any instruction we
  // perform. Weird special cases all derive from RLOp_SpecialCase, which
  // redefines the Dispatcher, so this is ok.
  if (AdvanceInstructionPointer())
    machine.AdvanceInstructionPointer();
}

// Implementation for IntConstant_T
IntConstant_T::type IntConstant_T::getData(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& p,
    unsigned int& position) {
  return p[position++].GetIntegerValue(machine);
}

// Was working to change the verify_type to parse_parameters.
void IntConstant_T::ParseParameters(
    unsigned int& position,
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
  const char* data = input.at(position).c_str();
  libreallive::ExpressionPiece ep(libreallive::GetData(data));

  if (ep.GetExpressionValueType() != libreallive::ValueTypeInteger) {
    std::ostringstream oss;
    oss << "IntConstant_T parse error. Expected type string, but actually "
        << "contained \"" << ep.GetDebugString() << "\"";
    throw rlvm::Exception(oss.str());
  }

  output.push_back(std::move(ep));
  position++;
}

IntReference_T::type IntReference_T::getData(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& p,
    unsigned int& position) {
  return p[position++].GetIntegerReferenceIterator(machine);
}

void IntReference_T::ParseParameters(
    unsigned int& position,
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
  const char* data = input.at(position).c_str();
  libreallive::ExpressionPiece ep(libreallive::GetData(data));

  if (ep.GetExpressionValueType() != libreallive::ValueTypeInteger) {
    std::ostringstream oss;
    oss << "IntReference_T parse error. Expected type string, but actually "
        << "contained \"" << ep.GetDebugString() << "\"";
    throw rlvm::Exception(oss.str());
  }

  output.push_back(std::move(ep));
  position++;
}

StrConstant_T::type StrConstant_T::getData(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& p,
    unsigned int& position) {
  // When I was trying to get P_BRIDE running in rlvm, I noticed that when
  // loading a game, I would often crash with invalid iterators in the LRUCache
  // library, in SDLGraphicsSystem where I cached recently used images. After
  // adding some verification methods to it, it was obvious that it's internal
  // state was entirely screwed up. There were duplicates in the std::list<>
  // (it's a precondition that its elements are unique), there were entries in
  // the std::list<> that didn't exist in the corresponding std::map<>; the
  // data structure was a complete mess. I verified the datastructure before
  // and after each operation...and found that it would almost always be
  // corrupted in the check that happened at the beginning of each
  // function. Printing out the c_str() pointers showed there was no change
  // between the consistent data structure and the corrupted one; the
  // underlying pointers were the same.
  //
  // Setting a few watch points, the internal c_str() buffers were being
  // overwritten by a memcpy in libstdc++ which was called by boost::serialize
  // during the loading of the RLMachine's local memory.  But that makes
  // sense. Think about what happens when we execute the following kepago:
  //
  //   strS[0] = 'SOMEFILE'
  //   grpOpenBg(0, strS[0])
  //
  // We are assigning a value into the RLMachine's string memory. We are then
  // passing a copy-on-write string, backed by one memory buffer that contains
  // 'SOMEFILE' around. LRUCache and string memory now point to the same char*
  // buffer.
  //
  // boost::serialization appears to ignore the COW semantics of std::string
  // and just writes into it during a serialization::load() no matter how many
  // people hold references to the inner buffer. Now we have one screwed up
  // LRUCache data structure, and probably screwed up other places.
  //
  // So to fix this, we break the COW semantics here by forcing a copy. I'd
  // prefer to do this in RLMachine or Memory, but I can't because they return
  // references.
  string tmp = p[position++].GetStringValue(machine);
  return string(tmp.data(), tmp.size());
}

void StrConstant_T::ParseParameters(
    unsigned int& position,
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
  const char* data = input.at(position).c_str();
  libreallive::ExpressionPiece ep(libreallive::GetData(data));

  if (ep.GetExpressionValueType() != libreallive::ValueTypeString) {
    std::ostringstream oss;
    oss << "StrConstant_T parse error. Expected type string, but actually "
        << "contained \"" << ep.GetDebugString() << "\"";
    throw rlvm::Exception(oss.str());
  }

  output.push_back(std::move(ep));
  position++;
}

StrReference_T::type StrReference_T::getData(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& p,
    unsigned int& position) {
  return p[position++].GetStringReferenceIterator(machine);
}

void StrReference_T::ParseParameters(
    unsigned int& position,
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
  const char* data = input.at(position).c_str();
  libreallive::ExpressionPiece ep(libreallive::GetData(data));

  if (ep.GetExpressionValueType() != libreallive::ValueTypeString) {
    std::ostringstream oss;
    oss << "StrReference_T parse error. Expected type string, but actually "
        << "contained \"" << ep.GetDebugString() << "\"";
    throw rlvm::Exception(oss.str());
  }

  output.push_back(std::move(ep));
  position++;
}

void RLOp_SpecialCase::Dispatch(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& parameters) {
  throw rlvm::Exception("Tried to call empty RLOp_SpecialCase::Dispatch().");
}

void RLOp_SpecialCase::ParseParameters(
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
  for (auto const& parameter : input) {
    const char* src = parameter.c_str();
    output.push_back(libreallive::GetData(src));
  }
}

void RLOp_SpecialCase::DispatchFunction(RLMachine& machine,
                                        const libreallive::CommandElement& ff) {
  // First try to run the default parse_parameters if we can.
  if (!ff.AreParametersParsed()) {
    std::vector<std::string> unparsed = ff.GetUnparsedParameters();
    libreallive::ExpressionPiecesVector output;
    ParseParameters(unparsed, output);
    ff.SetParsedParameters(std::move(output));
  }

  // Pass this on to the implementation of this functor.
  operator()(machine, ff);
}

template <>
void RLNormalOpcode<>::ParseParameters(
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
}

template <>
void RLOpcode<>::Dispatch(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& parameters) {
  operator()(machine);
}

// Default instantiations.
template class RLNormalOpcode<>;
template class RLNormalOpcode<IntConstant_T>;
template class RLNormalOpcode<IntConstant_T, IntConstant_T>;
template class RLNormalOpcode<IntConstant_T, StrConstant_T>;
template class RLNormalOpcode<IntConstant_T, IntConstant_T, IntConstant_T>;
template class RLNormalOpcode<IntConstant_T, IntConstant_T, IntConstant_T,
                              IntConstant_T>;
template class RLNormalOpcode<IntReference_T>;
template class RLNormalOpcode<IntReference_T, IntReference_T>;
template class RLNormalOpcode<StrConstant_T>;
template class RLNormalOpcode<StrConstant_T, IntConstant_T>;
template class RLNormalOpcode<StrConstant_T, StrConstant_T>;
template class RLNormalOpcode<StrReference_T>;

template class RLOpcode<>;
template class RLOpcode<IntConstant_T>;
template class RLOpcode<IntConstant_T, IntConstant_T>;
template class RLOpcode<IntConstant_T, StrConstant_T>;
template class RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T>;
template class RLOpcode<IntConstant_T, IntConstant_T, IntConstant_T,
                        IntConstant_T>;
template class RLOpcode<IntReference_T>;
template class RLOpcode<IntReference_T, IntReference_T>;
template class RLOpcode<StrConstant_T>;
template class RLOpcode<StrConstant_T, IntConstant_T>;
template class RLOpcode<StrConstant_T, StrConstant_T>;
template class RLOpcode<StrReference_T>;
