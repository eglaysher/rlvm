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

#ifndef SRC_SYSTEMS_BASE_OBJECTMUTATOR_HPP_
#define SRC_SYSTEMS_BASE_OBJECTMUTATOR_HPP_

class GraphicsObject;
class RLMachine;

// An object that changes the value of an object parameter over time.
class ObjectMutator {
 public:
  ObjectMutator(int layer, int object, int child, const char* name,
                int creation_time, int duration_time, int delay,
                int type);
  virtual ~ObjectMutator();

  // Called every tick. Returns true if the command has completed.
  bool operator()(RLMachine& machine);

  // Returns true if this ObjectMutator is operating on parameter |name| on
  // |layer|/|object|/|child|.
  bool OperationMatches(int layer, int object, int child, const char* name);

 protected:
  // Returns the specified GraphicsObject.
  GraphicsObject& GetObject(RLMachine& machine);

  // Returns what value should be set on the object at the current time.
  int GetValueForTime(RLMachine& machine, int start, int end);

  // Template method that actually sets the values.
  virtual void PerformSetting(RLMachine& machine) = 0;

 private:
  // Which object we are.
  int layer_;
  int object_;
  int child_;

  // The name of our operation.
  const char* name_;

  // Clock value at time of creation
  int creation_time_;

  // How long the mutation should go on.
  int duration_time_;

  // An optional duration after |creation_time_| where we don't do anything.
  int delay_;

  // Reallive's linear/accelerating/decelerating flag
  int type_;
};

// -----------------------------------------------------------------------

// An object mutator that takes a single integer.
class OneIntObjectMutator : public ObjectMutator {
 public:
  typedef int(GraphicsObject::*Getter)() const;
  typedef void(GraphicsObject::*Setter)(const int);

  OneIntObjectMutator(RLMachine& machine,
                      int layer, int object, int child, const char* name,
                      int creation_time, int duration_time, int delay,
                      int type, int target_value, Getter getter,
                      Setter setter);
  virtual ~OneIntObjectMutator();

 private:
  virtual void PerformSetting(RLMachine& machine);

  int startval_;
  int endval_;
  Setter setter_;
};

// -----------------------------------------------------------------------

// An object mutator that varies two integers.
class TwoIntObjectMutator : public ObjectMutator {
 public:
  typedef int(GraphicsObject::*Getter)() const;
  typedef void(GraphicsObject::*Setter)(const int);

  TwoIntObjectMutator(RLMachine& machine,
                      int layer, int object, int child, const char* name,
                      int creation_time, int duration_time, int delay,
                      int type,
                      int target_one, Getter getter_one, Setter setter_one,
                      int target_two, Getter getter_two, Setter setter_two);
  virtual ~TwoIntObjectMutator();

 private:
  virtual void PerformSetting(RLMachine& machine);

  int startval_one_;
  int endval_one_;
  Setter setter_one_;

  int startval_two_;
  int endval_two_;
  Setter setter_two_;
};

#endif  // SRC_SYSTEMS_BASE_OBJECTMUTATOR_HPP_
