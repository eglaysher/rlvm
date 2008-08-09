// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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
 * @file   Module_Obj.cpp
 * @author Elliot Glaysher
 * @date   Sat Feb 10 10:34:12 2007
 *
 * @brief  Reusable function objects for the GraphicsObject system.
 */

#include "Modules/Module_Obj.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"

// -----------------------------------------------------------------------

GraphicsObject& getGraphicsObject(RLMachine& machine, int layer, int obj)
{
  return machine.system().graphics().getObject(layer, obj);
}

// -----------------------------------------------------------------------

void setGraphicsObject(RLMachine& machine, int layer, int obj,
                       GraphicsObject& gobj)
{
  machine.system().graphics().setObject(layer, obj, gobj);
}

// -----------------------------------------------------------------------
// Obj_SetOneIntOnObj
// -----------------------------------------------------------------------

Obj_SetOneIntOnObj::Obj_SetOneIntOnObj(int inlayer, Setter s)
  : setter(s), layer(inlayer)
{}

// -----------------------------------------------------------------------

Obj_SetOneIntOnObj::~Obj_SetOneIntOnObj()
{}

// -----------------------------------------------------------------------

void Obj_SetOneIntOnObj::operator()(RLMachine& machine, int buf, int incoming)
{
  GraphicsObject& obj = getGraphicsObject(machine, layer, buf);
  ((obj).*(setter))(incoming);
}


// -----------------------------------------------------------------------
// Obj_SetTwoIntOnObj
// -----------------------------------------------------------------------

Obj_SetTwoIntOnObj::Obj_SetTwoIntOnObj(int inlayer, Setter one, Setter two)
  : setterOne(one), setterTwo(two), layer(inlayer)
{}

// -----------------------------------------------------------------------

Obj_SetTwoIntOnObj::~Obj_SetTwoIntOnObj()
{}

// -----------------------------------------------------------------------

void Obj_SetTwoIntOnObj::operator()(RLMachine& machine, int buf,
                                    int incomingOne, int incomingTwo)
{
  GraphicsObject& obj = getGraphicsObject(machine, layer, buf);
  ((obj).*(setterOne))(incomingOne);
  ((obj).*(setterTwo))(incomingTwo);
}
