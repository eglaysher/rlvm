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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Modules/Module_Gan.hpp"

#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "Modules/Module_Obj.hpp"
#include "Systems/Base/GanGraphicsObjectData.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/System.hpp"

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;
using namespace libReallive;

// -----------------------------------------------------------------------

struct Gan_ganPlay : public RLOp_Void_2<IntConstant_T, IntConstant_T>
{
  struct WaitForGanToFinish : public LongOperation
  {
    /**
     * Save the screen update mode and change it to automatic when entering a
     * blocking animation, restoring when we leave.
     *
     * When I actually started paying attention to DrawManual()/DrawAuto(), I
     * stopped forcing a refresh of the screen on each and instead obeyed the
     * DCScreenUpdateMode, I broke the OP of Kanon.
     *
     * I don't know if this is the correct solution. If this breaks things, the
     * solution is that things like AnmGraphicsObjectData force screen
     * refreshes, ignoring whatever the current screen update mode is.
     */
    GraphicsSystem& system_;
    GraphicsSystem::DCScreenUpdateMode mode_;

    int layer_;
    int buf_;
    WaitForGanToFinish(GraphicsSystem& system, int inLayer, int inBuf)
      : system_(system),
        mode_(system_.screenUpdateMode()),
        layer_(inLayer), buf_(inBuf) {
      system_.setScreenUpdateMode(GraphicsSystem::SCREENUPDATEMODE_AUTOMATIC);
    }

    bool operator()(RLMachine& machine)
    {
      GraphicsObject& obj = getGraphicsObject(machine, layer_, buf_);
      bool done = true;

      if(obj.hasObjectData())
      {
        const GraphicsObjectData& data = obj.objectData();
        if(data.isAnimation())
          done = !data.currentlyPlaying();
      }

      if (done) {
        // Restore whatever mode we were in before.
        system_.setScreenUpdateMode(mode_);
      }

      return done;
    }
  };

  bool block_;
  int layer_;
  GraphicsObjectData::AfterAnimation after_effect_;

  Gan_ganPlay(bool block, int layer,
              GraphicsObjectData::AfterAnimation after)
    : block_(block), layer_(layer), after_effect_(after) {}

  void operator()(RLMachine& machine, int buf, int animationSet)
  {
    GraphicsObject& obj = getGraphicsObject(machine, layer_, buf);

    if(obj.hasObjectData())
    {
      GraphicsObjectData& data = obj.objectData();
      if(data.isAnimation())
      {
        data.playSet(machine, animationSet);
        data.setAfterAction(after_effect_);

        if(block_)
          machine.pushLongOperation(new WaitForGanToFinish(
                                      machine.system().graphics(),
                                      layer_, buf));
      }
    }
  }
};

// -----------------------------------------------------------------------

void addGanOperationsTo(RLModule& m, int layer)
{
  m.addUnsupportedOpcode(1000, 0, "objStop");
  m.addUnsupportedOpcode(1000, 1, "objStop");

  m.addOpcode(1001, 0, "ganLoop",
              new Gan_ganPlay(false, layer, GraphicsObjectData::AFTER_LOOP));
  m.addOpcode(1003, 0, "ganPlay",
              new Gan_ganPlay(false, layer, GraphicsObjectData::AFTER_NONE));
  m.addOpcode(1005, 0, "ganPlayOnce",
              new Gan_ganPlay(false, layer, GraphicsObjectData::AFTER_CLEAR));
  m.addOpcode(1006, 0, "ganPlayEx",
              new Gan_ganPlay(true, layer, GraphicsObjectData::AFTER_NONE));
  m.addOpcode(1007, 0, "ganPlayOnceEx",
              new Gan_ganPlay(true, layer, GraphicsObjectData::AFTER_CLEAR));

  m.addOpcode(2001, 0, "objLoop",
              new Gan_ganPlay(false, layer, GraphicsObjectData::AFTER_LOOP));
  m.addUnsupportedOpcode(2003, 0, "objPlay");

  m.addOpcode(3001, 0, "ganLoop2",
              new Gan_ganPlay(false, layer, GraphicsObjectData::AFTER_LOOP));
  m.addOpcode(3003, 0, "ganPlay2",
              new Gan_ganPlay(false, layer, GraphicsObjectData::AFTER_NONE));
  m.addOpcode(3005, 0, "ganPlayOnce2",
              new Gan_ganPlay(false, layer, GraphicsObjectData::AFTER_CLEAR));
  m.addOpcode(3006, 0, "ganPlayEx2",
              new Gan_ganPlay(true, layer, GraphicsObjectData::AFTER_NONE));
  m.addOpcode(3007, 0, "ganPlayOnceEx2",
              new Gan_ganPlay(true, layer, GraphicsObjectData::AFTER_CLEAR));
}

// -----------------------------------------------------------------------

GanFgModule::GanFgModule()
  : RLModule("GanFg", 1, 73)
{
  addGanOperationsTo(*this, OBJ_FG_LAYER);
}

// -----------------------------------------------------------------------

GanBgModule::GanBgModule()
  : RLModule("GanBg", 1, 74)
{
  addGanOperationsTo(*this, OBJ_BG_LAYER);
}
