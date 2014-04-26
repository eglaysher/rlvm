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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_LONG_OPERATIONS_ZOOM_LONG_OPERATION_H_
#define SRC_LONG_OPERATIONS_ZOOM_LONG_OPERATION_H_

#include <memory>

#include "machine/long_operation.h"
#include "systems/base/rect.h"

class RLMachine;
class Surface;

class ZoomLongOperation : public LongOperation {
 public:
  ZoomLongOperation(RLMachine& machine,
                    const std::shared_ptr<Surface>& orig_surface_,
                    const std::shared_ptr<Surface>& src_surface_,
                    const Rect& frect_,
                    const Rect& trect_,
                    const Rect& drect_,
                    const int time);
  virtual ~ZoomLongOperation();

  virtual bool operator()(RLMachine& machine);

 private:
  std::shared_ptr<Surface> orig_surface_;
  std::shared_ptr<Surface> src_surface_;

  const Rect frect_;
  const Rect trect_;
  const Rect drect_;
  const unsigned int duration_;

  unsigned int start_time_;
};

#endif  // SRC_LONG_OPERATIONS_ZOOM_LONG_OPERATION_H_
