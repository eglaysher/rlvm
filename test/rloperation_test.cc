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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#include "gtest/gtest.h"

#include <algorithm>
#include <string>
#include <vector>

#include "libreallive/archive.h"
#include "libreallive/expression.h"
#include "libreallive/intmemref.h"
#include "machine/rlmachine.h"
#include "machine/rloperation.h"
#include "machine/rloperation/argc_t.h"
#include "machine/rloperation/complex_t.h"
#include "machine/rloperation/default_value.h"
#include "machine/rloperation/references.h"
#include "test_system/test_system.h"

#include "test_utils.h"

using namespace std;
using namespace std::placeholders;
using namespace libreallive;

// -----------------------------------------------------------------------

template <class T>
void runDataTest(T& t, RLMachine& machine, const vector<string>& input) {
  ExpressionPiecesVector expression_pieces;
  vector<string> binary_strings;
  transform(input.begin(),
            input.end(),
            back_inserter(binary_strings),
            bind(&PrintableToParsableString, _1));

  t.ParseParameters(binary_strings, expression_pieces);
  t.Dispatch(machine, expression_pieces);
}

// -----------------------------------------------------------------------

class RLOperationTest : public FullSystemTest {};

// -----------------------------------------------------------------------

// Tests that we can parse an IntConstant_T.
struct IntcIntcCapturer : public RLOpcode<IntConstant_T, IntConstant_T> {
  int& one_;
  int& two_;

  IntcIntcCapturer(int& one, int& two) : one_(one), two_(two) {}

  virtual void operator()(RLMachine& machine, int in_one, int in_two) {
    one_ = in_one;
    two_ = in_two;
  }
};

TEST_F(RLOperationTest, TestIntConstant_T) {
  int one = -1;
  int two = -1;
  IntcIntcCapturer capturer(one, two);

  vector<string> unparsed = {"$ FF 01 00 00 00", "$ FF 02 00 00 00"};
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ(1, one);
  EXPECT_EQ(2, two);
}

// -----------------------------------------------------------------------

// Tests that we can parse an IntReference_T.
struct IntRefIntRefCapturer
    : public RLOpcode<IntReference_T, IntReference_T> {
  int& one_;
  int& two_;

  IntRefIntRefCapturer(int& one, int& two) : one_(one), two_(two) {}

  virtual void operator()(RLMachine& machine,
                          IntReferenceIterator in_one,
                          IntReferenceIterator in_two) {
    one_ = *in_one;
    two_ = *in_two;
  }
};

TEST_F(RLOperationTest, TestIntReference_T) {
  rlmachine.SetIntValue(IntMemRef('A', 0), 1);
  rlmachine.SetIntValue(IntMemRef('B', 5), 2);

  int one = -1;
  int two = -1;
  IntRefIntRefCapturer capturer(one, two);

  vector<string> unparsed = {"$ 00 [ $ FF 00 00 00 00 ]",
                             "$ 01 [ $ FF 05 00 00 00 ]"};
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ(1, one);
  EXPECT_EQ(2, two);
}

// -----------------------------------------------------------------------

struct StringcStringcCapturer
    : public RLOpcode<StrConstant_T, StrConstant_T> {
  std::string& one_;
  std::string& two_;

  StringcStringcCapturer(std::string& one, std::string& two)
      : one_(one), two_(two) {}

  virtual void operator()(RLMachine& machine,
                          std::string in_one,
                          std::string in_two) {
    one_ = in_one;
    two_ = in_two;
  }
};

TEST_F(RLOperationTest, TestStringConstant_T) {
  std::string one = "empty";
  std::string two = "empty";
  StringcStringcCapturer capturer(one, two);

  vector<string> unparsed = {"\"string one\"", "\"string two\""};
  ExpressionPiecesVector expression_pieces;
  capturer.ParseParameters(unparsed, expression_pieces);
  capturer.Dispatch(rlmachine, expression_pieces);

  EXPECT_EQ("string one", one);
  EXPECT_EQ("string two", two);
}

// -----------------------------------------------------------------------

struct IntcStringcCapturer
    : public RLOpcode<IntConstant_T, StrConstant_T> {
  int& one_;
  std::string& two_;

  IntcStringcCapturer(int& one, std::string& two)
      : one_(one), two_(two) {}

  virtual void operator()(RLMachine& machine,
                          int in_one,
                          std::string in_two) {
    one_ = in_one;
    two_ = in_two;
  }
};

TEST_F(RLOperationTest, TestIntStringConstant_T) {
  int one = -1;
  std::string two = "empty";
  IntcStringcCapturer capturer(one, two);

  vector<string> unparsed = {PrintableToParsableString("$ FF 01 00 00 00"),
                             "\"string two\""};
  ExpressionPiecesVector expression_pieces;
  capturer.ParseParameters(unparsed, expression_pieces);
  capturer.Dispatch(rlmachine, expression_pieces);

  EXPECT_EQ(1, one);
  EXPECT_EQ("string two", two);
}

// -----------------------------------------------------------------------

// Tests that we can parse an StrReference_T.
struct StrRefStrRefCapturer
    : public RLOpcode<StrReference_T, StrReference_T> {
  std::string& one_;
  std::string& two_;

  StrRefStrRefCapturer(std::string& one, std::string& two)
      : one_(one), two_(two) {}

  virtual void operator()(RLMachine& machine,
                          StringReferenceIterator in_one,
                          StringReferenceIterator in_two) {
    one_ = *in_one;
    two_ = *in_two;
  }
};

TEST_F(RLOperationTest, TestStringReference_T) {
  rlmachine.SetStringValue(STRM_LOCATION, 0, "string one");
  rlmachine.SetStringValue(STRS_LOCATION, 5, "string two");

  std::string one = "empty";
  std::string two = "empty";
  StrRefStrRefCapturer capturer(one, two);

  vector<string> unparsed = {"$ 0C [ $ FF 00 00 00 00 ]",
                             "$ 12 [ $ FF 05 00 00 00 ]"};
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ("string one", one);
  EXPECT_EQ("string two", two);
}

// -----------------------------------------------------------------------

struct ArgcCapturer : public RLOpcode<Argc_T<IntConstant_T>> {
  std::vector<int>& out_;
  explicit ArgcCapturer(std::vector<int>& out) : out_(out) {}

  virtual void operator()(RLMachine& machine, std::vector<int> inputs) {
    copy(inputs.begin(), inputs.end(), back_inserter(out_));
  }
};

TEST_F(RLOperationTest, TestArgc_T) {
  vector<int> output;
  ArgcCapturer capturer(output);

  vector<string> unparsed = {"$ FF 09 00 00 00", "$ FF 03 00 00 00",
                             "$ FF 07 00 00 00", "$ FF 00 00 00 00"};
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ(4, output.size());
  EXPECT_EQ(9, output[0]);
  EXPECT_EQ(3, output[1]);
  EXPECT_EQ(7, output[2]);
  EXPECT_EQ(0, output[3]);
}

// -----------------------------------------------------------------------

struct DefaultValueCapturer : public RLOpcode<DefaultIntValue_T<18>> {
  int& out_;
  explicit DefaultValueCapturer(int& out) : out_(out) {}

  virtual void operator()(RLMachine& machine, int in) { out_ = in; }
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
    : public RLOpcode<Complex_T<IntConstant_T, IntConstant_T>,
                      Complex_T<IntConstant_T, IntConstant_T>> {
  int& one_;
  int& two_;
  int& three_;
  int& four_;

  ComplexCapturer(int& one, int& two, int& three, int& four)
      : one_(one), two_(two), three_(three), four_(four) {}

  virtual void operator()(RLMachine& machine,
                          Complex_T<IntConstant_T, IntConstant_T>::type one,
                          Complex_T<IntConstant_T, IntConstant_T>::type two) {
    one_ = get<0>(one);
    two_ = get<1>(one);
    three_ = get<0>(two);
    four_ = get<1>(two);
  }
};

TEST_F(RLOperationTest, TestComplex_T) {
  int one = -1;
  int two = -1;
  int three = -1;
  int four = -1;
  ComplexCapturer capturer(one, two, three, four);

  vector<string> unparsed = {"( $ FF 01 00 00 00 $ FF 02 00 00 00 )",
                             "( $ FF 03 00 00 00 $ FF 04 00 00 00 )"};
  runDataTest(capturer, rlmachine, unparsed);

  EXPECT_EQ(1, one);
  EXPECT_EQ(2, two);

  EXPECT_EQ(3, three);
  EXPECT_EQ(4, four);
}
