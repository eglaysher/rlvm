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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "ZoomLongOperation.hpp"


// -----------------------------------------------------------------------
// ZoomLongOperation
// -----------------------------------------------------------------------

ZoomLongOperation::ZoomLongOperation(
  const int fx, const int fy, const int fwidth, const int fheight, 
  const int tx, const int ty, const int twidth, const int theight,
  const int srcDC, const int dx, const int dy, const int dwidth, 
  const int dheight,  const int time)
  : m_fx(fx), m_fy(fy), m_fwidth(fwidth), m_fheight(fheight), 
    m_tx(tx), m_ty(ty), m_twidth(twidth), m_theight(theight),
    m_srcDC(srcDC), m_dx(dx), m_dy(dy), m_dwidth(dwidth), 
    m_dheight(dheight),  m_time(time)
{}

// -----------------------------------------------------------------------

ZoomLongOperation::~ZoomLongOperation()
{}

// -----------------------------------------------------------------------

bool ZoomLongOperation::operator()(RLMachine& machine)
{}



