// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "gtest/gtest.h"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"
#include "MachineBase/RLOperation/References.hpp"
#include "TestSystem/TestSystem.hpp"
#include "libReallive/archive.h"
#include "libReallive/expression.h"
#include "libReallive/intmemref.h"

#include "testUtils.hpp"

#include <boost/ptr_container/ptr_vector.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <iostream>

using boost::assign::list_of;

using namespace boost;
using namespace std;
using namespace libReallive;

// -----------------------------------------------------------------------

template<class T>
void runDataTest(T& t, RLMachine& machine, const vector<string>& input) {
  ExpressionPiecesVector expression_pieces;
  vector<string> binary_strings;
  transform(input.begin(), input.end(), back_inserter(binary_strings),
            bind(&printableToParsableString, _1));

  t.parseParameters(binary_strings, expression_pieces);
  t.dispatch(machine, expression_pieces);
}

// -----------------------------------------------------------------------

class RLOperationTest : public ::testing::Test {
 protected:
  RLOperationTest()
      : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
        system(),
        rlmachine(system, arc) {
  }

  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  TestSystem system;
  RLMachine rlmachine;
};

// -----------------------------------------------------------------------

// Tests that we can parse an IntConstant_T.
struct IntcIntcCapturer : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  int& one_;
  int& two_;

  IntcIntcCapturer(int& one, int& two)
      : one_(one), two_(two) {
  }

  virtual void operator()(RLMachine& machine, int in_one, int in_two) {
    one_ = in_one;
    two_ = in_two;
  }
};

TEST_F(RLOperationTest, TestIntConstant_T) {
  int one = -1;
  int two = -1;
  IntcIntcCapturer capturer(one, two);

  vector<string> unparsed =
      list_of("$ FF 01 00 00 00")
      ("$ FF 02 00 00 00");
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ(1, one);
  EXPECT_EQ(2, two);
}

// -----------------------------------------------------------------------

// Tests that we can parse an IntReference_T.
struct IntRefIntRefCapturer
    : public RLOp_Void_2<IntReference_T, IntReference_T> {
  int& one_;
  int& two_;

  IntRefIntRefCapturer(int& one, int& two)
      : one_(one), two_(two) {
  }

  virtual void operator()(RLMachine& machine, IntReferenceIterator in_one,
                          IntReferenceIterator in_two) {
    one_ = *in_one;
    two_ = *in_two;
  }
};

TEST_F(RLOperationTest, TestIntReference_T) {
  rlmachine.setIntValue(IntMemRef('A', 0), 1);
  rlmachine.setIntValue(IntMemRef('B', 5), 2);

  int one = -1;
  int two = -1;
  IntRefIntRefCapturer capturer(one, two);

  vector<string> unparsed =
      list_of("$ 00 [ $ FF 00 00 00 00 ]")
      ("$ 01 [ $ FF 05 00 00 00 ]");
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ(1, one);
  EXPECT_EQ(2, two);
}

// -----------------------------------------------------------------------

struct StringcStringcCapturer
    : public RLOp_Void_2<StrConstant_T, StrConstant_T> {
  std::string& one_;
  std::string& two_;

  StringcStringcCapturer(std::string& one, std::string& two)
      : one_(one), two_(two) {
  }

  virtual void operator()(RLMachine& machine, std::string in_one,
                          std::string in_two) {
    one_ = in_one;
    two_ = in_two;
  }
};

TEST_F(RLOperationTest, TestStringConstant_T) {
  std::string one = "empty";
  std::string two = "empty";
  StringcStringcCapturer capturer(one, two);

  vector<string> unparsed =
      list_of("\"string one\"")
      ("\"string two\"");
  ExpressionPiecesVector expression_pieces;
  capturer.parseParameters(unparsed, expression_pieces);
  capturer.dispatch(rlmachine, expression_pieces);

  EXPECT_EQ("string one", one);
  EXPECT_EQ("string two", two);
}

// -----------------------------------------------------------------------

// Tests that we can parse an StrReference_T.
struct StrRefStrRefCapturer
    : public RLOp_Void_2<StrReference_T, StrReference_T> {
  std::string& one_;
  std::string& two_;

  StrRefStrRefCapturer(std::string& one, std::string& two)
      : one_(one), two_(two) {
  }

  virtual void operator()(RLMachine& machine, StringReferenceIterator in_one,
                          StringReferenceIterator in_two) {
    one_ = *in_one;
    two_ = *in_two;
  }
};

TEST_F(RLOperationTest, TestStringReference_T) {
  rlmachine.setStringValue(STRM_LOCATION, 0, "string one");
  rlmachine.setStringValue(STRS_LOCATION, 5, "string two");

  std::string one = "empty";
  std::string two = "empty";
  StrRefStrRefCapturer capturer(one, two);

  vector<string> unparsed =
      list_of("$ 0C [ $ FF 00 00 00 00 ]")
      ("$ 12 [ $ FF 05 00 00 00 ]");
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ("string one", one);
  EXPECT_EQ("string two", two);
}

// -----------------------------------------------------------------------

struct ArgcCapturer : public RLOp_Void_1<Argc_T<IntConstant_T> > {
  std::vector<int>& out_;
  explicit ArgcCapturer(std::vector<int>& out) : out_(out) {}

  virtual void operator()(RLMachine& machine, std::vector<int> inputs) {
    copy(inputs.begin(), inputs.end(), back_inserter(out_));
  }
};

TEST_F(RLOperationTest, TestArgc_T) {
  vector<int> output;
  ArgcCapturer capturer(output);

  vector<string> unparsed =
      list_of("$ FF 09 00 00 00")
      ("$ FF 03 00 00 00")
      ("$ FF 07 00 00 00")
      ("$ FF 00 00 00 00");
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ(4, output.size());
  EXPECT_EQ(9, output[0]);
  EXPECT_EQ(3, output[1]);
  EXPECT_EQ(7, output[2]);
  EXPECT_EQ(0, output[3]);
}

// -----------------------------------------------------------------------

struct DefaultValueCapturer
    : public RLOp_Void_1<DefaultIntValue_T<18> > {
  int& out_;
  explicit DefaultValueCapturer(int& out) : out_(out) {}

  virtual void operator()(RLMachine& machine, int in) {
    out_ = in;
  }
};

TEST_F(RLOperationTest, TestDefaultIntValue_T) {
  int output = -1;
  DefaultValueCapturer capturer(output);
  vector<string> unparsed;
  runDataTest(capturer, rlmachine, unparsed);
  EXPECT_EQ(18, output) << "Uses default value with no arguments";

  unparsed.push_back("$ FF 04 00 00 00");
  runDataTest(capturer, rlmachine, unparsed);
  EXPECT_EQ(4, output) << "Returns argument";
}

// -----------------------------------------------------------------------

struct ComplexCapturer
    : public RLOp_Void_2<Complex2_T<IntConstant_T, IntConstant_T>,
                         Complex2_T<IntConstant_T, IntConstant_T> > {
  int& one_;
  int& two_;
  int& three_;
  int& four_;

  ComplexCapturer(int& one, int& two, int& three, int& four)
      : one_(one), two_(two), three_(three), four_(four) {
  }

  virtual void operator()(RLMachine& machine,
                          Complex2_T<IntConstant_T, IntConstant_T>::type one,
                          Complex2_T<IntConstant_T, IntConstant_T>::type two) {
    one_ = one.get<0>();
    two_ = one.get<1>();
    three_ = two.get<0>();
    four_ = two.get<1>();
  }
};

TEST_F(RLOperationTest, TestComplex2_T) {
  int one = -1;
  int two = -1;
  int three = -1;
  int four = -1;
  ComplexCapturer capturer(one, two, three, four);

  vector<string> unparsed =
      list_of("( $ FF 01 00 00 00 $ FF 02 00 00 00 )")
      ("( $ FF 03 00 00 00 $ FF 04 00 00 00 )");
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ(1, one);
  EXPECT_EQ(2, two);

  EXPECT_EQ(3, three);
  EXPECT_EQ(4, four);
}
