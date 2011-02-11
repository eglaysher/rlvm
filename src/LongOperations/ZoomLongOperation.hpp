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

#ifndef SRC_LONGOPERATIONS_ZOOMLONGOPERATION_HPP_
#define SRC_LONGOPERATIONS_ZOOMLONGOPERATION_HPP_

#include <boost/shared_ptr.hpp>

#include "MachineBase/LongOperation.hpp"
#include "Systems/Base/Rect.hpp"

class RLMachine;
class Surface;

class ZoomLongOperation : public LongOperation {
 public:
  ZoomLongOperation(
    RLMachine& machine,
    const boost::shared_ptr<Surface>& orig_surface_,
    const boost::shared_ptr<Surface>& src_surface_,
    const Rect& frect_, const Rect& trect_, const Rect& drect_,
    const int time);
  virtual ~ZoomLongOperation();

  virtual bool operator()(RLMachine& machine);

 private:
  RLMachine& machine_;

  boost::shared_ptr<Surface> orig_surface_;
  boost::shared_ptr<Surface> src_surface_;

  const Rect frect_;
  const Rect trect_;
  const Rect drect_;
  const unsigned int duration_;

  unsigned int start_time_;
};

#endif  // SRC_LONGOPERATIONS_ZOOMLONGOPERATION_HPP_
