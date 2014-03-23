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

#include "machine/reference.h"

#include <string>

#include "machine/memory.h"
#include "libreallive/intmemref.h"

using libreallive::IntMemRef;

// -----------------------------------------------------------------------
// IntAccessor
// -----------------------------------------------------------------------

IntAccessor::IntAccessor(MemoryReferenceIterator<IntAccessor>* i)
    : it(i), store_register_(i->store_register_) {}

IntAccessor::~IntAccessor() {}

IntAccessor::operator int() const {
  if (store_register_)
    return *store_register_;
  else
    return it->memory_->GetIntValue(IntMemRef(it->type_, it->location_));
}

IntAccessor& IntAccessor::operator=(const int new_value) {
  if (store_register_)
    *store_register_ = new_value;
  else
    it->memory_->SetIntValue(IntMemRef(it->type_, it->location_), new_value);
  return *this;
}

IntAccessor& IntAccessor::operator=(const IntAccessor& rhs) {
  return operator=(rhs.operator int());
}

// -----------------------------------------------------------------------
// StringAccessor
// -----------------------------------------------------------------------

StringAccessor::StringAccessor(MemoryReferenceIterator<StringAccessor>* i)
    : it(i) {}

StringAccessor::~StringAccessor() {}

StringAccessor::operator std::string() const {
  return it->memory_->GetStringValue(it->type_, it->location_);
}

StringAccessor& StringAccessor::operator=(const std::string& new_value) {
  it->memory_->SetStringValue(it->type_, it->location_, new_value);
  return *this;
}

bool StringAccessor::operator==(const std::string& rhs) {
  return operator std::string() == rhs;
}

StringAccessor& StringAccessor::operator=(const StringAccessor& rhs) {
  return operator=(rhs.operator std::string());
}
