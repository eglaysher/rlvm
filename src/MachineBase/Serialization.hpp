// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------


#ifndef __Serialization_hpp__
#define __Serialization_hpp__

#include "MachineBase/SaveGameHeader.hpp"
#include <boost/filesystem/path.hpp>

// -----------------------------------------------------------------------

class RLMachine;
class Memory;

// -----------------------------------------------------------------------

namespace Serialization {

/**
 * Pointer to the machine that is having its data
 * serialized. boost::serialization doesn't allow passing something
 * like a closure around, which is frustrating because many pieces of
 * data rely on looking things up on the machine.
 *
 * @warning We're using what is essentially a piece of static data
 *          here; this is a likely location for errors
 */
extern RLMachine* g_currentMachine;

// -----------------------------------------------------------------------

void saveGlobalMemory(RLMachine& machine);
void saveGlobalMemoryTo(std::ostream& oss, RLMachine& machine);

void loadGlobalMemory(RLMachine& machine);
void loadGlobalMemoryFrom(std::istream& iss, RLMachine& machine);

boost::filesystem::path buildSaveGameFilename(RLMachine& machine, int slot);

void saveGameForSlot(RLMachine& machine, int slot);
void saveGameTo(std::ostream& oss, RLMachine& machine);

SaveGameHeader loadHeaderForSlot(RLMachine& machine, int slot);
SaveGameHeader loadHeaderFrom(std::istream& iss);

void loadLocalMemoryForSlot(RLMachine& machine, int slot, Memory& memory);
void loadLocalMemoryFrom(std::istream& iss, Memory& memory);

void loadGameForSlot(RLMachine& machine, int slot);
void loadGameFrom(std::istream& iss, RLMachine& machine);

}


#endif
