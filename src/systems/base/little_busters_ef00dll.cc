// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2010 Elliot Glaysher
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

#include "systems/base/little_busters_ef00dll.h"

#include <iostream>
#include <string>

#include "machine/rlmachine.h"
#include "utilities/exception.h"
#include "libreallive/intmemref.h"

using libreallive::IntMemRef;

namespace {

static int random_dirtable[] = {0, 2, 1, 3, 0, 2, 1, 3, 1, 3, 2, 0, 1, 3, 2, 0,
                                0, 0, 0, 0, 3, 1, 2, 0, 3, 1, 3, 1, 0, 2, 3, 1};

}  // namespace

int LittleBustersEF00DLL::CallDLL(RLMachine& machine,
                                  int func,
                                  int arg1,
                                  int arg2,
                                  int arg3,
                                  int arg4) {
  switch (func) {
    case 0:
      ConfigureEffect(arg1, arg2, arg3, arg4);
      break;
    case 1:
      PerformCalculations(machine, arg1);
      break;
    default:
      std::cerr << "LittleBustersEF00DLL::callDLL(): Invalid function: " << func
                << std::endl;
      return -1;
  }
  return 0;
}

void LittleBustersEF00DLL::ConfigureEffect(int arg1,
                                           int arg2,
                                           int arg3,
                                           int arg4) {
  if (!lb_ef_param)
    lb_ef_param.reset(new double[sizeof(double) * 0x60 * 8]);

  int param_top, param_size;
  if (arg1 == 1) {
    param_top = 0;
    param_size = 0x20;
  } else {
    param_top = arg2;
    param_size = arg3;
    if (param_top < 0)
      param_top = 0;
    if (param_top > 0x20)
      param_top = 0x20;
    if (param_size + param_top > 0x20)
      param_size = 0x20 - param_top;
  }
  for (int i = 0; i < 8; i++) {
    double* param = lb_ef_param.get() + i * 0x60 + param_top * 3;
    for (int j = 0; j < param_size; j++) {
      *param++ = random() % 800 - 400;
      *param++ = random() % 600 - 300;
      *param++ = random() % 700 - 350;
    }
  }
  if (arg4 != 1)
    return;
  int* dir = &random_dirtable[(random() & 3) * 8];
  for (int i = 0; i < 8; i++) {
    double* param = lb_ef_param.get() + i * 0x60;
    double x = random() % 600 - 300;
    double y = random() % 480 - 240;
    if (x < 0)
      x -= 80;
    else
      x += 80;
    if (y < 0)
      y -= 80;
    else
      y += 80;

    switch (*dir++) {
      case 0:
        if (x < 0)
          x = -x;
        if (y < 0)
          y = -y;
        break;
      case 1:
        if (x > 0)
          x = -x;
        if (y < 0)
          y = -y;
        break;
      case 2:
        if (x < 0)
          x = -x;
        if (y > 0)
          y = -y;
        break;
      case 4:
        if (x > 0)
          x = -x;
        if (y > 0)
          y = -y;
        break;
    }
    param[9] = x * 1.2;
    param[10] = y * 1.2;
    param[11] *= 1.2;
    param[12] *= -0.08;
    param[13] *= -0.08;
    param[14] *= -0.08;
    param[15] = -param[9];
    param[16] = -param[10];
    param[17] = -param[11];
  }
}

void LittleBustersEF00DLL::PerformCalculations(RLMachine& machine, int index) {
  if (!lb_ef_param) {
    throw rlvm::Exception("Effect calculation was called before setting");
  }

  int v5_1154 =
      machine.GetIntValue(IntMemRef(libreallive::INTF_LOCATION, 1154 + index));
  int j = ((v5_1154) & 0x1f) + index * 0x20;
  int k = ((v5_1154 + 1) & 0x1f) + index * 0x20;
  int l = ((v5_1154 + 2) & 0x1f) + index * 0x20;
  int m = ((v5_1154 + 3) & 0x1f) + index * 0x20;
  j *= 3;
  k *= 3;
  l *= 3;
  m *= 3;

  // 0 < x < 1
  // va - vd は 0-1 の範囲で対称性を持つ３次関数
  double x = double(machine.GetIntValue(
                 IntMemRef(libreallive::INTF_LOCATION, 1162 + index))) *
             0.001;
  double va = (x * x * x) / 6;
  double vb = (-x * x * x + 3 * x * x - 3 * x + 1) / 6;
  double vc = (3 * x * x * x - 6 * x * x + 4) / 6;
  double vd = (-3 * x * x * x + 3 * x * x + 3 * x + 1) / 6;

  double r1 = va * lb_ef_param[m + 3] + vd * lb_ef_param[l + 3] +
              vc * lb_ef_param[k + 3] + vb * lb_ef_param[j + 3];
  double r2 = va * lb_ef_param[m + 2] + vd * lb_ef_param[l + 2] +
              vc * lb_ef_param[k + 2] + vb * lb_ef_param[j + 2];
  double r3 = va * lb_ef_param[m + 1] + vd * lb_ef_param[l + 1] +
              vc * lb_ef_param[k + 1] + vb * lb_ef_param[j + 1];
  if (r1 != 400) {
    r2 = r2 * 800 / (400 - r1);
    r3 = r3 * 700 / (400 - r1);
  }

  machine.SetIntValue(IntMemRef(libreallive::INTF_LOCATION, 1151), r2);
  machine.SetIntValue(IntMemRef(libreallive::INTF_LOCATION, 1152), r3);
  machine.SetIntValue(IntMemRef(libreallive::INTF_LOCATION, 1153), r1);
}

const std::string& LittleBustersEF00DLL::GetDLLName() const {
  static std::string n("EF00");
  return n;
}
