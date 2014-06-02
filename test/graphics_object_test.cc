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

#include <boost/scoped_ptr.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "libreallive/archive.h"
#include "libreallive/intmemref.h"
#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "modules/module_obj_fg_bg.h"
#include "modules/module_obj_management.h"
#include "modules/module_str.h"
#include "systems/base/colour_filter_object_data.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_object_of_file.h"
#include "systems/base/object_mutator.h"
#include "systems/base/parent_graphics_object_data.h"
#include "test_system/mock_colour_filter.h"
#include "test_system/test_graphics_system.h"
#include "test_system/test_system.h"
#include "utilities/exception.h"

#include "test_utils.h"
#include <iostream>

using namespace boost;
using namespace std;
using namespace libreallive;
using namespace Serialization;

using ::testing::_;
using ::testing::Ref;
using ::testing::Return;

const char* FILE_NAME = "doesntmatter";

class GraphicsObjectTest : public FullSystemTest {};

// Test the serialization of an individual GraphicsObjectOfFile object.
TEST_F(GraphicsObjectTest, SerializeObjectData) {
  stringstream ss;
  Serialization::g_current_machine = &rlmachine;
  {
    const scoped_ptr<GraphicsObjectData> inputObjOfFile(
        new GraphicsObjectOfFile(system, FILE_NAME));
    boost::archive::text_oarchive oa(ss);
    oa << inputObjOfFile;
  }
  {
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
  Serialization::g_current_machine = &rlmachine;
  {
    const scoped_ptr<GraphicsObject> obj(new GraphicsObject());
    obj->SetObjectData(new GraphicsObjectOfFile(system, FILE_NAME));

    boost::archive::text_oarchive oa(ss);
    oa << obj;
  }
  {
    scoped_ptr<GraphicsObject> dst;
    boost::archive::text_iarchive ia(ss);
    ia >> dst;

    GraphicsObjectOfFile& obj =
        dynamic_cast<GraphicsObjectOfFile&>(dst->GetObjectData());

    EXPECT_EQ(FILE_NAME, obj.filename()) << "Preserved file name";
  }

  Serialization::g_current_machine = NULL;
}

// -----------------------------------------------------------------------

// Automated tests for accessors that take one int.
typedef std::tuple<std::function<void(GraphicsObject&, const int)>,
                   std::function<int(const GraphicsObject&)>> TupleT;

class AccessorTest : public ::testing::TestWithParam<TupleT> {
  // Empty.
};

TEST_P(AccessorTest, TestReferenceCount) {
  TupleT accessors = GetParam();

  GraphicsObject obj;
  GraphicsObject objCopy(obj);

  // At this step, it is equal to three because they all share the
  // empty object
  EXPECT_EQ(3, obj.reference_count())
      << "Both objects have the same internal object";
  EXPECT_EQ(3, objCopy.reference_count())
      << "Both objects have the same internal object";

  // Call the getter method (ignoring the result). We expect that
  // this won't force a copy-on-write.
  (get<1>(accessors))(objCopy);

  EXPECT_EQ(3, obj.reference_count())
      << "Both objects have the same internal object";
  EXPECT_EQ(3, objCopy.reference_count())
      << "Both objects have the same internal object";

  // Call this setter function. This should force the copy-on-write
  // code to trigger.
  (get<0>(accessors))(objCopy, 1);

  EXPECT_EQ(2, obj.reference_count())
      << "Untouched object still points to empty";
  EXPECT_EQ(1, objCopy.reference_count()) << "Modified object has its own impl";
}

typedef vector<TupleT> SetterVec;
SetterVec graphics_object_setters = {
    std::make_tuple(&GraphicsObject::SetVisible, &GraphicsObject::visible),
    std::make_tuple(&GraphicsObject::SetX, &GraphicsObject::x),
    std::make_tuple(&GraphicsObject::SetY, &GraphicsObject::y),
    std::make_tuple(&GraphicsObject::SetVert, &GraphicsObject::vert),
    std::make_tuple(&GraphicsObject::SetOriginX, &GraphicsObject::origin_x),
    std::make_tuple(&GraphicsObject::SetOriginY, &GraphicsObject::origin_y),
    std::make_tuple(&GraphicsObject::SetWidth, &GraphicsObject::width),
    std::make_tuple(&GraphicsObject::SetHeight, &GraphicsObject::height),
    std::make_tuple(&GraphicsObject::SetHqWidth, &GraphicsObject::hq_width),
    std::make_tuple(&GraphicsObject::SetHqHeight, &GraphicsObject::hq_height),
    std::make_tuple(&GraphicsObject::SetRotation, &GraphicsObject::rotation),
    std::make_tuple(&GraphicsObject::SetPattNo, &GraphicsObject::GetPattNo),
    std::make_tuple(&GraphicsObject::SetMono, &GraphicsObject::mono),
    std::make_tuple(&GraphicsObject::SetInvert, &GraphicsObject::invert),
    std::make_tuple(&GraphicsObject::SetLight, &GraphicsObject::light),
    std::make_tuple(&GraphicsObject::SetCompositeMode,
                    &GraphicsObject::composite_mode),
    std::make_tuple(&GraphicsObject::SetScrollRateX,
                    &GraphicsObject::scroll_rate_x),
    std::make_tuple(&GraphicsObject::SetScrollRateY,
                    &GraphicsObject::scroll_rate_y),
    std::make_tuple(&GraphicsObject::SetZOrder, &GraphicsObject::z_order),
    std::make_tuple(&GraphicsObject::SetZLayer, &GraphicsObject::z_layer),
    std::make_tuple(&GraphicsObject::SetZDepth, &GraphicsObject::z_depth),
    std::make_tuple(&GraphicsObject::SetAlpha, &GraphicsObject::raw_alpha),
    std::make_tuple(&GraphicsObject::SetWipeCopy, &GraphicsObject::wipe_copy)};

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
    std::make_tuple(&GraphicsObject::SetTintRed, getTintR),
    std::make_tuple(&GraphicsObject::SetTintGreen, getTintG),
    std::make_tuple(&GraphicsObject::SetTintBlue, getTintB),
    std::make_tuple(&GraphicsObject::SetColourRed, getColourR),
    std::make_tuple(&GraphicsObject::SetColourGreen, getColourG),
    std::make_tuple(&GraphicsObject::SetColourBlue, getColourB),
    std::make_tuple(&GraphicsObject::SetColourLevel, getColourLevel)};

INSTANTIATE_TEST_CASE_P(GraphicsObjectTintAndColour,
                        AccessorTest,
                        ::testing::ValuesIn(graphics_object_colour_setters));

// -----------------------------------------------------------------------

class MockGraphicsObjectData : public GraphicsObjectData {
 public:
  MOCK_METHOD2(Render, void(const GraphicsObject&, std::ostream*));
  MOCK_METHOD1(PixelWidth, int(const GraphicsObject&));
  MOCK_METHOD1(PixelHeight, int(const GraphicsObject&));
  MOCK_CONST_METHOD0(Clone, GraphicsObjectData*());
  MOCK_METHOD1(Execute, void(RLMachine&));
  MOCK_METHOD0(IsAnimation, bool());
  MOCK_METHOD1(PlaySet, void(int));
  MOCK_METHOD1(CurrentSurface,
               std::shared_ptr<const Surface>(const GraphicsObject&));
  MOCK_METHOD1(ObjectInfo, void(std::ostream&));
};

TEST_F(GraphicsObjectTest, TestPixelHeightWidth) {
  GraphicsObject obj;

  // Check default values when we don't have a GraphicsObjectData.
  EXPECT_EQ(0, obj.PixelWidth());
  EXPECT_EQ(0, obj.PixelHeight());

  MockGraphicsObjectData* data = new MockGraphicsObjectData;
  obj.SetObjectData(data);
  EXPECT_CALL(*data, PixelWidth(Ref(obj))).Times(1).WillOnce(Return(30));
  EXPECT_CALL(*data, PixelHeight(Ref(obj))).Times(1).WillOnce(Return(50));
  EXPECT_EQ(30, obj.PixelWidth());
  EXPECT_EQ(50, obj.PixelHeight());

  ::testing::Mock::VerifyAndClearExpectations(data);
}

TEST_F(GraphicsObjectTest, GetObjectData) {
  GraphicsObject obj;

  // Throws when it doesn't have data.
  EXPECT_THROW({ obj.GetObjectData(); }, rlvm::Exception);

  MockGraphicsObjectData* data = new MockGraphicsObjectData;
  obj.SetObjectData(data);
  EXPECT_EQ(data, &obj.GetObjectData());
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
  obj.SetObjectData(data);

  MockColourFilter* filter =
      dynamic_cast<MockColourFilter*>(data->GetColourFilter());
  EXPECT_CALL(*filter, Fill(_, one, _));
  EXPECT_CALL(*filter, Fill(_, two, _));

  // Render as is (for the first call).
  data->Render(obj, NULL, NULL);

  data->set_rect(two);

  // Render with the modified rect (for the second call).
  data->Render(obj, NULL, NULL);
}

TEST_F(GraphicsObjectTest, objFgFreeAll) {
  GraphicsObject obj;
  obj.SetX(50);
  obj.SetY(120);
  obj.SetObjectData(new ColourFilterObjectData(
      system.graphics(), Rect(10, 10, Size(80, 70))));

  system.graphics().SetObject(0, 10, obj);
  system.graphics().SetObject(0, 46, obj);
  system.graphics().SetObject(1, 18, obj);

  rlmachine.AttachModule(new ObjFgManagement);
  rlmachine.Exe("objFgFreeAll", 0);

  EXPECT_FALSE(system.graphics().GetObject(0, 10).has_object_data());
  EXPECT_FALSE(system.graphics().GetObject(0, 46).has_object_data());
  EXPECT_TRUE(system.graphics().GetObject(1, 18).has_object_data());
}

TEST_F(GraphicsObjectTest, ObjectMutatorCopy) {
  GraphicsObject obj;
  obj.SetX(50);
  obj.SetY(120);
  obj.SetObjectData(new ColourFilterObjectData(
      system.graphics(), Rect(10, 10, Size(80, 70))));
  system.graphics().SetObject(1, 18, obj);

  // Set a mutator on the object.
  rlmachine.AttachModule(new ObjBgModule);
  rlmachine.Exe("objBgEveColLevel", 1, TestMachine::Arg(18, 128, 0, 0, 0));

  EXPECT_TRUE(system.graphics().GetObject(1, 18).IsMutatorRunningMatching(
      -1, "objEveColLevel"));

  system.graphics().ClearAndPromoteObjects();

  EXPECT_TRUE(system.graphics().GetObject(0, 18).IsMutatorRunningMatching(
      -1, "objEveColLevel"));
}

class MutatorTest : public ObjectMutator {
 public:
  MutatorTest()
      : ObjectMutator(-1, "MutatorTest", 0, 1000, 0, 0),
        called_(false) {}

  bool called() const { return called_; }

  virtual bool operator()(RLMachine& machine, GraphicsObject& object) override {
    called_ = true;
    return false;
  }

  virtual void SetToEnd(RLMachine& machine, GraphicsObject& object) override {}
  virtual ObjectMutator* Clone() const override { return NULL; }
  virtual void PerformSetting(RLMachine& machine, GraphicsObject& object)
      override {}

 private:
  bool called_;
};

TEST_F(GraphicsObjectTest, RunMutatorsOnChildObjects) {
  GraphicsObject parent;
  ParentGraphicsObjectData* parent_data = new ParentGraphicsObjectData(10);
  parent.SetObjectData(parent_data);

  MutatorTest* mutator_test = new MutatorTest;
  parent_data->GetObject(5).AddObjectMutator(
      std::unique_ptr<ObjectMutator>(mutator_test));

  EXPECT_FALSE(mutator_test->called());
  parent.Execute(rlmachine);
  EXPECT_TRUE(mutator_test->called());
}
