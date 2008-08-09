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
//
// This is our precompiled header. Note that we include a lot of
// boost, something that every C++ program should do.

#ifndef __Header_hpp__
#define __Header_hpp__

// C++ Standard Template Library
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Boost.org Template Libraries
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

// At least for the time being, I'm putting these files in the
// precompiled header because they're large template messes and are
// probably eating up a lot of parsing time.
//
// REMEMBER TO COMMENT THESE LINES OUT BEFORE YOU GO OFF AND FIDDLE
// WITH THEM!
// #include "libReallive/gameexe.h"
// #include "libReallive/defs.h"
// #include "libReallive/bytecode.h"
// #include "libReallive/bytecode_fwd.h"

// #include "MachineBase/reference.hpp"
// #include "MachineBase/RLOperation.hpp"
// #include "MachineBase/RLOperation/Argc_T.hpp"
// #include "MachineBase/RLOperation/Complex_T.hpp"
// #include "MachineBase/RLOperation/DefaultValue.hpp"
// #include "MachineBase/RLOperation/RLOp_Store.hpp"
// #include "MachineBase/RLOperation/References.hpp"
// #include "MachineBase/RLOperation/Special_T.hpp"

#endif
