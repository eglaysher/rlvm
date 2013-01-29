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

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/References.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "Utilities/Exception.hpp"
#include "libReallive/bytecode.h"

#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <boost/bind.hpp>

using namespace std;
using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------
// RLOperation
// -----------------------------------------------------------------------

RLOperation::RLOperation()
    : property_list_(NULL),
      name_(NULL) {
}

RLOperation::~RLOperation() {
  if (property_list_)
    delete property_list_;
}

RLOperation* RLOperation::setProperty(int property, int value) {
  if (!property_list_) {
    property_list_ = new std::vector< std::pair<int, int> >;
  }

  // Modify the property if it already exists
  PropertyList::iterator it = findProperty(property);
  if (it != property_list_->end()) {
    it->second = value;
  } else {
    property_list_->push_back(std::make_pair(property, value));
  }

  return this;
}

bool RLOperation::getProperty(int property, int& value) const {
  if (property_list_) {
    PropertyList::iterator it = findProperty(property);
    if (it != property_list_->end()) {
      value = it->second;
      return true;
    }
  }

  if (module_) {
    // If we don't have a property, ask our module if it has one.
    return module_->getProperty(property, value);
  }

  return false;
}

RLOperation::PropertyList::iterator RLOperation::findProperty(
    int property) const {
  return find_if (property_list_->begin(), property_list_->end(),
                 bind(&Property::first, _1) == property);
}

bool RLOperation::advanceInstructionPointer() {
  return true;
}

void RLOperation::dispatchFunction(RLMachine& machine,
                                   const CommandElement& ff) {
  if (!ff.areParametersParsed()) {
    vector<string> unparsed = ff.getUnparsedParameters();
    ptr_vector<ExpressionPiece> output;
    parseParameters(unparsed, output);
    ff.setParsedParameters(output);
  }

  const ptr_vector<ExpressionPiece>& parameter_pieces = ff.getParameters();

  // Now dispatch based on these parameters.
  dispatch(machine, parameter_pieces);

  // By default, we advacne the instruction pointer on any instruction we
  // perform. Weird special cases all derive from RLOp_SpecialCase, which
  // redefines the dispatcher, so this is ok.
  if (advanceInstructionPointer())
    machine.advanceInstructionPointer();
}

void RLOperation::throw_unimplemented() {
  throw rlvm::Exception("Unimplemented function");
}

// Implementation for IntConstant_T
IntConstant_T::type IntConstant_T::getData(
    RLMachine& machine,
    const boost::ptr_vector<libReallive::ExpressionPiece>& p,
    unsigned int& position) {
  return p[position++].integerValue(machine);
}

// Was working to change the verify_type to parse_parameters.
void IntConstant_T::parseParameters(
  unsigned int& position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output) {
  const char* data = input.at(position).c_str();
  auto_ptr<ExpressionPiece> ep(get_data(data));

  if (ep->expressionValueType() != libReallive::ValueTypeInteger) {
    throw rlvm::Exception("IntConstant_T parse err.");
  }

  output.push_back(ep.release());
  position++;
}

IntReference_T::type IntReference_T::getData(
    RLMachine& machine,
    const boost::ptr_vector<libReallive::ExpressionPiece>& p,
    unsigned int& position) {
  return static_cast<const libReallive::MemoryReference&>(p[position++]).
    getIntegerReferenceIterator(machine);
}

void IntReference_T::parseParameters(
  unsigned int& position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output) {
  const char* data = input.at(position).c_str();
  auto_ptr<ExpressionPiece> ep(get_data(data));

  if (ep->expressionValueType() != libReallive::ValueTypeInteger) {
    throw rlvm::Exception("IntReference_T parse err.");
  }

  output.push_back(ep.release());
  position++;
}

StrConstant_T::type StrConstant_T::getData(
    RLMachine& machine,
    const boost::ptr_vector<libReallive::ExpressionPiece>& p,
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
  string tmp = p[position++].getStringValue(machine);
  return string(tmp.data(), tmp.size());
}

void StrConstant_T::parseParameters(
  unsigned int& position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output) {
  const char* data = input.at(position).c_str();
  auto_ptr<ExpressionPiece> ep(get_data(data));

  if (ep->expressionValueType() != libReallive::ValueTypeString) {
    throw rlvm::Exception("StrConstant_T parse err.");
  }

  output.push_back(ep.release());
  position++;
}

StrReference_T::type StrReference_T::getData(
    RLMachine& machine,
    const boost::ptr_vector<libReallive::ExpressionPiece>& p,
    unsigned int& position) {
  return static_cast<const libReallive::MemoryReference&>(p[position++]).
    getStringReferenceIterator(machine);
}

void StrReference_T::parseParameters(
  unsigned int& position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output) {
  const char* data = input.at(position).c_str();
  auto_ptr<ExpressionPiece> ep(get_data(data));

  if (ep->expressionValueType() != libReallive::ValueTypeString) {
    throw rlvm::Exception("StrReference_T parse err.");
  }

  output.push_back(ep.release());
  position++;
}

void RLOp_SpecialCase::dispatch(
  RLMachine& machine,
  const boost::ptr_vector<libReallive::ExpressionPiece>& parameters) {
  throw rlvm::Exception(
      "Tried to call empty RLOp_SpecialCase::dispatch().");
}

void RLOp_SpecialCase::parseParameters(
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output) {
  for (vector<string>::const_iterator it = input.begin(); it != input.end();
      ++it) {
    const char* src = it->c_str();
    output.push_back(get_data(src));
  }
}

void RLOp_SpecialCase::dispatchFunction(RLMachine& machine,
                                        const libReallive::CommandElement& ff) {
  // First try to run the default parse_parameters if we can.
  if (!ff.areParametersParsed()) {
    vector<string> unparsed = ff.getUnparsedParameters();
    ptr_vector<ExpressionPiece> output;
    parseParameters(unparsed, output);
    ff.setParsedParameters(output);
  }

  // Pass this on to the implementation of this functor.
  operator()(machine, ff);
}

void RLOp_Void_Void::dispatch(RLMachine& machine,
                              const ExpressionPiecesVector& parameters) {
  operator()(machine);
}
