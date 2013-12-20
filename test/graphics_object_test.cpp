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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <string>
#include <vector>

#include "MachineBase/Memory.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Modules/Module_Str.hpp"
#include "TestSystem/MockColourFilter.hpp"
#include "TestSystem/TestGraphicsSystem.hpp"
#include "TestSystem/TestSystem.hpp"
#include "Systems/Base/ColourFilterObjectData.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"
#include "Utilities/Exception.hpp"
#include "libReallive/archive.h"
#include "libReallive/intmemref.h"

#include "testUtils.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <iostream>

using namespace boost;
using namespace std;
using namespace libReallive;
using namespace Serialization;

using ::testing::_;
using ::testing::Ref;
using ::testing::Return;

const char* FILE_NAME = "doesntmatter";

class GraphicsObjectTest : public FullSystemTest { };

// Test the serialization of an individual GraphicsObjectOfFile object.
TEST_F(GraphicsObjectTest, SerializeObjectData) {
  stringstream ss;
  Serialization::g_current_machine = &rlmachine; {
    const scoped_ptr<GraphicsObjectData> inputObjOfFile(
      new GraphicsObjectOfFile(system, FILE_NAME));
    boost::archive::text_oarchive oa(ss);
    oa << inputObjOfFile;
  } {
    scoped_ptr<GraphicsObjectData> dst;
    boost::archive::text_iarchive ia(ss);
    ia >> dst;

    GraphicsObjectOfFile& obj = dynamic_cast<GraphicsObjectOfFile&>(*dst);
    EXPECT_EQ(FILE_NAME, obj.filename()) << "Preserved file name";
  }

  Serialization::g_current_machine = NULL;
}

// -----------------------------------------------------------------------

// Try it again, this time wrapped in the GraphicsObject
TEST_F(GraphicsObjectTest, SerializeObject) {
  stringstream ss;
  Serialization::g_current_machine = &rlmachine; {
    const scoped_ptr<GraphicsObject> obj(new GraphicsObject());
    obj->setObjectData(new GraphicsObjectOfFile(system, FILE_NAME));

    boost::archive::text_oarchive oa(ss);
    oa << obj;
  } {
    scoped_ptr<GraphicsObject> dst;
    boost::archive::text_iarchive ia(ss);
    ia >> dst;

    GraphicsObjectOfFile& obj =
      dynamic_cast<GraphicsObjectOfFile&>(dst->objectData());

    EXPECT_EQ(FILE_NAME, obj.filename()) << "Preserved file name";
  }

  Serialization::g_current_machine = NULL;
}

// -----------------------------------------------------------------------

// Automated tests for accessors that take one int.
typedef boost::tuple<
  boost::function<void(GraphicsObject&, const int)>,
  boost::function<int(const GraphicsObject&)> > TupleT;

class AccessorTest : public ::testing::TestWithParam<TupleT> {
  // Empty.
};

TEST_P(AccessorTest, TestReferenceCount) {
  TupleT accessors = GetParam();

  GraphicsObject obj;
  GraphicsObject objCopy(obj);

  // At this step, it is equal to three because they all share the
  // empty object
  EXPECT_EQ(3, obj.referenceCount())
      << "Both objects have the same internal object";
  EXPECT_EQ(3, objCopy.referenceCount())
      << "Both objects have the same internal object";

  // Call the getter method (ignoring the result). We expect that
  // this won't force a copy-on-write.
  (accessors.get<1>())(objCopy);

  EXPECT_EQ(3, obj.referenceCount())
      << "Both objects have the same internal object";
  EXPECT_EQ(3, objCopy.referenceCount())
      << "Both objects have the same internal object";

  // Call this setter function. This should force the copy-on-write
  // code to trigger.
  (accessors.get<0>())(objCopy, 1);

  EXPECT_EQ(2, obj.referenceCount())
      << "Untouched object still points to empty";
  EXPECT_EQ(1, objCopy.referenceCount())
      << "Modified object has its own impl";
}

typedef vector<TupleT> SetterVec;
SetterVec graphics_object_setters = {
  { &GraphicsObject::setVisible, &GraphicsObject::visible },
  { &GraphicsObject::setX, &GraphicsObject::x },
  { &GraphicsObject::setY, &GraphicsObject::y },
  { &GraphicsObject::setVert, &GraphicsObject::vert },
  { &GraphicsObject::setXOrigin, &GraphicsObject::xOrigin },
  { &GraphicsObject::setYOrigin, &GraphicsObject::yOrigin },
  { &GraphicsObject::setWidth, &GraphicsObject::width },
  { &GraphicsObject::setHeight, &GraphicsObject::height },
  { &GraphicsObject::setRotation, &GraphicsObject::rotation },
  { &GraphicsObject::setPattNo, &GraphicsObject::pattNo },
  { &GraphicsObject::setMono, &GraphicsObject::mono },
  { &GraphicsObject::setInvert, &GraphicsObject::invert },
  { &GraphicsObject::setLight, &GraphicsObject::light },
  { &GraphicsObject::setCompositeMode, &GraphicsObject::compositeMode },
  { &GraphicsObject::setScrollRateX, &GraphicsObject::scrollRateX },
  { &GraphicsObject::setScrollRateY, &GraphicsObject::scrollRateY },
  { &GraphicsObject::setAlpha, &GraphicsObject::rawAlpha },
  { &GraphicsObject::setWipeCopy, &GraphicsObject::wipeCopy }
};

INSTANTIATE_TEST_CASE_P(GraphicsObjectSimple,
                        AccessorTest,
                        ::testing::ValuesIn(graphics_object_setters));

// -----------------------------------------------------------------------

int getTintR(const GraphicsObject& obj) { return obj.tint().r(); }
int getTintG(const GraphicsObject& obj) { return obj.tint().g(); }
int getTintB(const GraphicsObject& obj) { return obj.tint().b(); }
int getColourR(const GraphicsObject& obj) { return obj.colour().r(); }
int getColourG(const GraphicsObject& obj) { return obj.colour().g(); }
int getColourB(const GraphicsObject& obj) { return obj.colour().b(); }
int getColourLevel(const GraphicsObject& obj) { return obj.colour().a(); }

typedef vector<TupleT> SetterVec;
SetterVec graphics_object_colour_setters = {
  { &GraphicsObject::setTintR, getTintR },
  { &GraphicsObject::setTintG, getTintG },
  { &GraphicsObject::setTintB, getTintB },
  { &GraphicsObject::setColourR, getColourR },
  { &GraphicsObject::setColourG, getColourG },
  { &GraphicsObject::setColourB, getColourB },
  { &GraphicsObject::setColourLevel, getColourLevel }
};

INSTANTIATE_TEST_CASE_P(GraphicsObjectTintAndColour,
                        AccessorTest,
                        ::testing::ValuesIn(graphics_object_colour_setters));

// -----------------------------------------------------------------------

class MockGraphicsObjectData : public GraphicsObjectData {
 public:
  MOCK_METHOD2(render, void(const GraphicsObject&, std::ostream*));
  MOCK_METHOD1(pixelWidth, int(const GraphicsObject&));
  MOCK_METHOD1(pixelHeight, int(const GraphicsObject&));
  MOCK_CONST_METHOD0(clone, GraphicsObjectData*());
  MOCK_METHOD1(execute, void(RLMachine&));
  MOCK_METHOD0(isAnimation, bool());
  MOCK_METHOD1(playSet, void(int));
  MOCK_METHOD1(currentSurface,
               boost::shared_ptr<const Surface>(const GraphicsObject&));
  MOCK_METHOD1(objectInfo, void(std::ostream&));
};

TEST_F(GraphicsObjectTest, TestPixelHeightWidth) {
  GraphicsObject obj;

  // Check default values when we don't have a GraphicsObjectData.
  EXPECT_EQ(0, obj.pixelWidth());
  EXPECT_EQ(0, obj.pixelHeight());

  MockGraphicsObjectData* data = new MockGraphicsObjectData;
  obj.setObjectData(data);
  EXPECT_CALL(*data, pixelWidth(Ref(obj))).Times(1).WillOnce(Return(30));
  EXPECT_CALL(*data, pixelHeight(Ref(obj))).Times(1).WillOnce(Return(50));
  EXPECT_EQ(30, obj.pixelWidth());
  EXPECT_EQ(50, obj.pixelHeight());

  ::testing::Mock::VerifyAndClearExpectations(data);
}

TEST_F(GraphicsObjectTest, GetObjectData) {
  GraphicsObject obj;

  // Throws when it doesn't have data.
  EXPECT_THROW({obj.objectData();}, rlvm::Exception);

  MockGraphicsObjectData* data = new MockGraphicsObjectData;
  obj.setObjectData(data);
  EXPECT_EQ(data, &obj.objectData());
}

// TODO: Use the above mock to test more of the insides of GraphicsObject...


TEST_F(GraphicsObjectTest, TestColourFilter) {
  // In the past, we've had regressions because we ignored updated screen_rects
  // in colour filter objects.

  GraphicsObject obj;

  // Two completely different rects.
  const Rect one(10, 12, Size(18, 14));
  const Rect two(43, 81, Size(5, 20));

  ColourFilterObjectData* data =
      new ColourFilterObjectData(system.graphics(), one);
  obj.setObjectData(data);

  MockColourFilter* filter =
      dynamic_cast<MockColourFilter*>(data->GetColourFilter());
  EXPECT_CALL(*filter, Fill(_, one, _));
  EXPECT_CALL(*filter, Fill(_, two, _));

  // Render as is (for the first call).
  data->render(obj, NULL, NULL);

  data->setRect(two);

  // Render with the modified rect (for the second call).
  data->render(obj, NULL, NULL);
}
