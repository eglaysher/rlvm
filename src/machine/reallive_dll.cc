// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#include "machine/reallive_dll.h"

#include <boost/algorithm/string/predicate.hpp>
#include <string>
#include <sstream>

#include "systems/base/little_busters_ef00dll.h"
#include "systems/base/little_busters_pt00dll.h"
#include "systems/base/tomoyo_after_dt00dll.h"
#include "systems/base/rlbabel_dll.h"
#include "utilities/exception.h"

using std::ostringstream;

namespace {

class IgnoredDLL : public RealLiveDLL {
 public:
  IgnoredDLL(const std::string& name)
      : name_(name) {}
  ~IgnoredDLL() override {}

  int CallDLL(RLMachine& machine,
              int one,
              int two,
              int three,
              int four,
              int five) override {
    return 0;
  }

  const std::string& GetDLLName() const override {
    return name_;
  }

 private:
  const std::string& name_;
};

}  // namespace

// -----------------------------------------------------------------------
// RealLiveDLL
// -----------------------------------------------------------------------

RealLiveDLL* RealLiveDLL::BuildDLLNamed(RLMachine& machine,
                                        const std::string& name) {
  if (name == "rlBabel") {
    return new RlBabelDLL(machine);
  } else if (name == "EF00") {
    return new LittleBustersEF00DLL;
  } else if (name == "PT00") {
    return new LittleBustersPT00DLL;
  } else if (name == "DT00") {
    return new TomoyoAfterDT00DLL;
  } else if (boost::starts_with(name, "RealLiveSteam")) {
    return new IgnoredDLL(name);
  } else {
    ostringstream oss;
    oss << "Unsupported DLL interface " << name;
    throw rlvm::Exception(oss.str());
  }
}

RealLiveDLL::~RealLiveDLL() {}
