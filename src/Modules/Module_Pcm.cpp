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

#include "Modules/Module_Pcm.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "MachineBase/GeneralOperations.hpp"

// -----------------------------------------------------------------------

struct Pcm_wavPlay : public RLOp_Void_1<StrConstant_T> {
  void operator()(RLMachine& machine, std::string fileName) {
    machine.system().sound().wavPlay(machine, fileName);
  }
};

// -----------------------------------------------------------------------

PcmModule::PcmModule()
  : RLModule("Pcm", 1, 21)
{
  addOpcode(0, 0, "wavPlay", new Pcm_wavPlay);
  addUnsupportedOpcode(0, 1, "wavPlay");
  addUnsupportedOpcode(0, 2, "wavPlay");

  addUnsupportedOpcode(1, 0, "wavPlayEx");
  addUnsupportedOpcode(1, 1, "wavPlayEx");

  addUnsupportedOpcode(2, 0, "wavLoop");
  addUnsupportedOpcode(2, 1, "wavLoop");

  addUnsupportedOpcode(3, 0, "wavWait");
  addUnsupportedOpcode(4, 0, "wavPlaying");

  addUnsupportedOpcode(5, 0, "wavStop");
  addUnsupportedOpcode(5, 1, "wavStop");

  addUnsupportedOpcode(7, 0, "wavPlaying2");
  addUnsupportedOpcode(8, 0, "wavRewind");
  addUnsupportedOpcode(9, 0, "wavStop3");
  addUnsupportedOpcode(10, 0, "wavStop4");
  addUnsupportedOpcode(11, 0, "wavVolume");

  addUnsupportedOpcode(12, 0, "wavSetVolume");
  addUnsupportedOpcode(12, 1, "wavSetVolume");

  addUnsupportedOpcode(13, 0, "wavUnMute");
  addUnsupportedOpcode(13, 1, "wavUnMute");

  addUnsupportedOpcode(14, 0, "wavMute");
  addUnsupportedOpcode(14, 1, "wavMute");

  addUnsupportedOpcode(20, 0, "wavStopAll");

  addUnsupportedOpcode(105, 0, "wavFadeOut");
  addUnsupportedOpcode(106, 0, "wavFadeOut2");
  addUnsupportedOpcode(106, 1, "wavFadeOut2");

  // Unknown/Undocumented function in this module
//  fun <1:Pcm:00040, 0> (<intC, (strC, intC, intC)+)
}
