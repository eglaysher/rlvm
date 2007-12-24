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
// the Free Software Foundation; either version 2 of the License, or
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

#ifndef __ZoomLongOperation_hpp__
#define __ZoomLongOperation_hpp__

#include <boost/shared_ptr.hpp>

#include "MachineBase/LongOperation.hpp"

class RLMachine;
class Surface;

class ZoomLongOperation : public LongOperation
{
private:
  RLMachine& m_machine;

  boost::shared_ptr<Surface> m_origSurface;
  boost::shared_ptr<Surface> m_srcSurface;

  const int m_fx; 
  const int m_fy; 
  const int m_fwidth; 
  const int m_fheight;
  const int m_tx; 
  const int m_ty; 
  const int m_twidth; 
  const int m_theight; 
  const int m_dx;
  const int m_dy; 
  const int m_dwidth; 
  const int m_dheight; 
  const unsigned int m_duration;

  unsigned int m_startTime;

public:
  ZoomLongOperation(
    RLMachine& machine,
    const boost::shared_ptr<Surface>& m_origSurface,
    const boost::shared_ptr<Surface>& m_srcSurface,
    const int fx, const int fy, const int fwidth, const int fheight, 
    const int tx, const int ty, const int twidth, const int theight,
    const int dx, const int dy, const int dwidth, 
    const int dheight,  const int time);
  ~ZoomLongOperation();

  virtual bool operator()(RLMachine& machine);
};

#endif
