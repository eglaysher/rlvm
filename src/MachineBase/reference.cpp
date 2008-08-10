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
 * @file   reference.cpp
 * @brief  Defines the iterator interface to integer and string memory
 * @author Elliot Glaysher
 * @date   Sat Oct  7 11:15:37 2006
 *
 */

#include "MachineBase/reference.hpp"
#include "MachineBase/Memory.hpp"
#include "libReallive/intmemref.h"

using libReallive::IntMemRef;

// -----------------------------------------------------------------------
// IntAccessor
// -----------------------------------------------------------------------

IntAccessor::IntAccessor(MemoryReferenceIterator<IntAccessor>* i)
  : it(i)
{}

// -----------------------------------------------------------------------

IntAccessor::~IntAccessor()
{}

// -----------------------------------------------------------------------

/**
 * Read from the memory location, and return the value.
 *
 * @return The integer value of the memory location.
 */
IntAccessor::operator int() const
{
  return it->memory_->getIntValue(IntMemRef(it->type_, it->location_));
}

// -----------------------------------------------------------------------

/**
 * Assign a new value to the memory location.
 *
 * @param new_value New value to set.
 * @return Self
 */
IntAccessor& IntAccessor::operator=(const int new_value) {
  it->memory_->setIntValue(IntMemRef(it->type_, it->location_), new_value);
  return *this;
}

// -----------------------------------------------------------------------

/**
 * Assigns to this accessor from another IntAccessor. This allows us
 * to use the "*dest = *src" mechanic since normally, that would call
 * the default copy operator, which would copy rhs.it onto it.
 *
 * @param rhs IntAccessor to read from
 * @return Self
 */
IntAccessor& IntAccessor::operator=(const IntAccessor& rhs)
{
  return operator=(rhs.operator int());
}

// -----------------------------------------------------------------------
// StringAccessor
// -----------------------------------------------------------------------

StringAccessor::StringAccessor(MemoryReferenceIterator<StringAccessor>* i)
  : it(i)
{}

// -----------------------------------------------------------------------

StringAccessor::~StringAccessor()
{}

// -----------------------------------------------------------------------

/**
 * Read from the memory location, and return the value.
 *
 * @return The string value of the memory location.
 */
StringAccessor::operator std::string() const
{
  return it->memory_->getStringValue(it->type_, it->location_);
}

// -----------------------------------------------------------------------

/**
 * Assign a new value to the memory location.
 *
 * @param new_value New value to set.
 * @return Self
 */
StringAccessor& StringAccessor::operator=(const std::string& new_value) {
  it->memory_->setStringValue(it->type_, it->location_, new_value);
  return *this;
}

// -----------------------------------------------------------------------

bool StringAccessor::operator==(const std::string& rhs) {
  return operator std::string() == rhs;
}

// -----------------------------------------------------------------------

/**
 * Assigns to this accessor from another StringAccessor. This allows
 * us to use the "*dest = *src" mechanic since normally, that would
 * call the default copy operator, which would copy rhs.it onto it.
 *
 * @param rhs StringAccessor to read from
 * @return Self
 */
StringAccessor& StringAccessor::operator=(const StringAccessor& rhs)
{
  return operator=(rhs.operator std::string());
}

