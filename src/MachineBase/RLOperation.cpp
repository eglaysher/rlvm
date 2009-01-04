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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file
 * @ingroup RLOperationGroup
 * @brief Defines all the base RLOperations and their type checking structs.
 */

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/References.hpp"
#include "MachineBase/RLMachine.hpp"

#include "libReallive/bytecode.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;
using namespace libReallive;

RLOperation::RLOperation()
{}

// -----------------------------------------------------------------------

RLOperation::~RLOperation()
{}

// -----------------------------------------------------------------------

bool RLOperation::advanceInstructionPointer()
{
  return true;
}

// -----------------------------------------------------------------------

void RLOperation::dispatchFunction(RLMachine& machine, const CommandElement& ff)
{
  if(!ff.areParametersParsed())
  {
    const vector<string>& unparsed = ff.getUnparsedParameters();
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
  if(advanceInstructionPointer())
    machine.advanceInstructionPointer();
}

// -----------------------------------------------------------------------

void RLOperation::throw_unimplemented()
{
  throw rlvm::Exception("Unimplemented function");
}

// -----------------------------------------------------------------------

// Implementation for IntConstant_T
IntConstant_T::type IntConstant_T::getData(RLMachine& machine,
                                           const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                                           unsigned int position)
{
  return p[position].integerValue(machine);
}

// -----------------------------------------------------------------------

// Was working to change the verify_type to parse_parameters.
void IntConstant_T::parseParameters(
  unsigned int position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output)
{
  const char* data = input.at(position).c_str();
  auto_ptr<ExpressionPiece> ep(get_data(data));

  if(ep->expressionValueType() != libReallive::ValueTypeInteger)
  {
    throw rlvm::Exception("IntConstant_T parse err.");
  }

  output.push_back(ep.release());
}

// -----------------------------------------------------------------------

IntReference_T::type IntReference_T::getData(RLMachine& machine,
                                             const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int position) {
  return p[position].getIntegerReferenceIterator(machine);
}

// -----------------------------------------------------------------------

void IntReference_T::parseParameters(
  unsigned int position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output)
{
  const char* data = input.at(position).c_str();
  auto_ptr<ExpressionPiece> ep(get_data(data));

  if(ep->expressionValueType() != libReallive::ValueTypeInteger)
  {
    throw rlvm::Exception("IntReference_T parse err.");
  }

  output.push_back(ep.release());
}

// -----------------------------------------------------------------------

StrConstant_T::type StrConstant_T::getData(RLMachine& machine,
                                           const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int position)
{
  return p[position].getStringValue(machine);
}

// -----------------------------------------------------------------------

void StrConstant_T::parseParameters(
  unsigned int position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output)
{
  const char* data = input.at(position).c_str();
  auto_ptr<ExpressionPiece> ep(get_data(data));

  if(ep->expressionValueType() != libReallive::ValueTypeString)
  {
    throw rlvm::Exception("StrConstant_T parse err.");
  }

  output.push_back(ep.release());
}

// -----------------------------------------------------------------------

StrReference_T::type StrReference_T::getData(RLMachine& machine,
                                             const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int position)
{
  return static_cast<const libReallive::MemoryReference&>(p[position]).
    getStringReferenceIterator(machine);
}

// -----------------------------------------------------------------------

void StrReference_T::parseParameters(
  unsigned int position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output)
{
  const char* data = input.at(position).c_str();
  auto_ptr<ExpressionPiece> ep(get_data(data));

  if(ep->expressionValueType() != libReallive::ValueTypeString)
  {
    throw rlvm::Exception("StrReference_T parse err.");
  }

  output.push_back(ep.release());
}

// -----------------------------------------------------------------------
// Empty_T
// -----------------------------------------------------------------------

Empty_T::type Empty_T::getData(RLMachine& machine,
                               const boost::ptr_vector<libReallive::ExpressionPiece>& p,
                      unsigned int position)
{
  return empty_struct();
}

// -----------------------------------------------------------------------

void Empty_T::parseParameters(
  unsigned int position,
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output)
{
}

// -----------------------------------------------------------------------

void RLOp_SpecialCase::dispatch(
  RLMachine& machine,
  const boost::ptr_vector<libReallive::ExpressionPiece>& parameters)
{}

// -----------------------------------------------------------------------

void RLOp_SpecialCase::parseParameters(
  const std::vector<std::string>& input,
  boost::ptr_vector<libReallive::ExpressionPiece>& output)
{
  for(vector<string>::const_iterator it = input.begin(); it != input.end();
      ++it)
  {
    const char* src = it->c_str();
    output.push_back(get_data(src));
  }
}

// -----------------------------------------------------------------------

void RLOp_SpecialCase::dispatchFunction(RLMachine& machine,
                                        const libReallive::CommandElement& ff)
{
  // First try to run the default parse_parameters if we can.
  if(!ff.areParametersParsed())
  {
    const vector<string>& unparsed = ff.getUnparsedParameters();
    ptr_vector<ExpressionPiece> output;
    parseParameters(unparsed, output);
    ff.setParsedParameters(output);
  }

  // Pass this on to the implementation of this functor.
  operator()(machine, ff);
}
