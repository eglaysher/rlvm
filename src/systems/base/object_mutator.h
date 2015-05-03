// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_BASE_OBJECT_MUTATOR_H_
#define SRC_SYSTEMS_BASE_OBJECT_MUTATOR_H_

#include <string>

class GraphicsObject;
class RLMachine;

// An object that changes the value of an object parameter over time.
class ObjectMutator {
 public:
  ObjectMutator(int repr,
                const std::string& name,
                int creation_time,
                int duration_time,
                int delay,
                int type);
  virtual ~ObjectMutator();

  int repr() const { return repr_; }
  const std::string& name() const { return name_; }

  // Called every tick. Returns true if the command has completed. Virtual for
  // testing.
  virtual bool operator()(RLMachine& machine, GraphicsObject& object);

  // Returns true if this ObjectMutator is operating on |name|/|repr|.
  bool OperationMatches(int repr, const std::string& name) const;

  // Called to end the mutation prematurely.
  virtual void SetToEnd(RLMachine& machine, GraphicsObject& object) = 0;

  // Builds a copy of the ObjectMutator. Used during object promotion.
  virtual ObjectMutator* Clone() const = 0;

 protected:
  ObjectMutator(const ObjectMutator& mutator);

  // Returns what value should be set on the object at the current time.
  int GetValueForTime(RLMachine& machine, int start, int end);

  // Template method that actually sets the values.
  virtual void PerformSetting(RLMachine& machine, GraphicsObject& object) = 0;

 private:
  // An optional paramater to identify object setters that pass additional
  // arguments.
  int repr_;

  // The name of our operation.
  const std::string name_;

  // Clock value at time of creation
  int creation_time_;

  // How long the mutation should go on.
  int duration_time_;

  // An optional duration after |creation_time_| where we don't do anything.
  int delay_;

  // What sort of interpolation we should do here.
  int type_;
};

// -----------------------------------------------------------------------

// An object mutator that takes a single integer.
class OneIntObjectMutator : public ObjectMutator {
 public:
  typedef void (GraphicsObject::*Setter)(const int);

  OneIntObjectMutator(const std::string& name,
                      int creation_time,
                      int duration_time,
                      int delay,
                      int type,
                      int start_value,
                      int target_value,
                      Setter setter);
  virtual ~OneIntObjectMutator();

 private:
  OneIntObjectMutator(const OneIntObjectMutator& rhs);

  virtual void SetToEnd(RLMachine& machine, GraphicsObject& object) override;
  virtual ObjectMutator* Clone() const override;
  virtual void PerformSetting(RLMachine& machine,
                              GraphicsObject& object) override;

  int startval_;
  int endval_;
  Setter setter_;
};

// -----------------------------------------------------------------------

// An object mutator that takes a repno and an integer.
class RepnoIntObjectMutator : public ObjectMutator {
 public:
  typedef void (GraphicsObject::*Setter)(const int, const int);

  RepnoIntObjectMutator(const std::string& name,
                        int creation_time,
                        int duration_time,
                        int delay,
                        int type,
                        int repno,
                        int start_value,
                        int target_value,
                        Setter setter);
  virtual ~RepnoIntObjectMutator();

 private:
  RepnoIntObjectMutator(const RepnoIntObjectMutator& rhs);

  virtual void SetToEnd(RLMachine& machine, GraphicsObject& object) override;
  virtual ObjectMutator* Clone() const override;
  virtual void PerformSetting(RLMachine& machine,
                              GraphicsObject& object) override;

  int repno_;
  int startval_;
  int endval_;
  Setter setter_;
};

// -----------------------------------------------------------------------

// An object mutator that varies two integers.
class TwoIntObjectMutator : public ObjectMutator {
 public:
  typedef void (GraphicsObject::*Setter)(const int);

  TwoIntObjectMutator(const std::string& name,
                      int creation_time,
                      int duration_time,
                      int delay,
                      int type,
                      int start_one,
                      int target_one,
                      Setter setter_one,
                      int start_two,
                      int target_two,
                      Setter setter_two);
  virtual ~TwoIntObjectMutator();

 private:
  TwoIntObjectMutator(const TwoIntObjectMutator& rhs);

  virtual void SetToEnd(RLMachine& machine, GraphicsObject& object) override;
  virtual ObjectMutator* Clone() const override;
  virtual void PerformSetting(RLMachine& machine,
                              GraphicsObject& object) override;

  int startval_one_;
  int endval_one_;
  Setter setter_one_;

  int startval_two_;
  int endval_two_;
  Setter setter_two_;
};

#endif  // SRC_SYSTEMS_BASE_OBJECT_MUTATOR_H_
