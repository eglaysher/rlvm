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

#ifndef TEST_TEST_SYSTEM_TEST_MACHINE_H_
#define TEST_TEST_SYSTEM_TEST_MACHINE_H_

#include "machine/rlmachine.h"

#include <boost/variant.hpp>

#include <map>
#include <string>
#include <vector>

class System;
class RLModule;
class RLOperation;
namespace libreallive {
class Archive;
}

// A class that wraps an RLMachine, listens for module attachments, and then
// records the names of the opcodes in a structure for fast lookup by module
// name/opcode number.
class TestMachine : public RLMachine {
 public:
  // Arguments for the exe() function. A pair of the argc and the processed
  // string form to put between the () in the argument list.
  typedef std::pair<int, std::string> ExeArgument;

  // Constructs an argument string from C++ types.
  template <typename A>
  static ExeArgument Arg(const A& a) {
    std::string output;
    addEntity(output, a);
    return ExeArgument(1, output);
  }

  template <typename A, typename B>
  static ExeArgument Arg(const A& a, const B& b) {
    std::string output;
    addEntity(output, a);
    addEntity(output, b);
    return ExeArgument(2, output);
  }

  template <typename A, typename B, typename C>
  static ExeArgument Arg(const A& a, const B& b, const C& c) {
    std::string output;
    addEntity(output, a);
    addEntity(output, b);
    addEntity(output, c);
    return ExeArgument(3, output);
  }

  template <typename A, typename B, typename C, typename D>
  static ExeArgument Arg(const A& a, const B& b, const C& c, const D& d) {
    std::string output;
    addEntity(output, a);
    addEntity(output, b);
    addEntity(output, c);
    addEntity(output, d);
    return ExeArgument(4, output);
  }

  template <typename A, typename B, typename C, typename D, typename E>
  static ExeArgument Arg(const A& a,
                         const B& b,
                         const C& c,
                         const D& d,
                         const E& e) {
    std::string output;
    addEntity(output, a);
    addEntity(output, b);
    addEntity(output, c);
    addEntity(output, d);
    addEntity(output, e);
    return ExeArgument(5, output);
  }

  template <typename A,
            typename B,
            typename C,
            typename D,
            typename E,
            typename F>
  static ExeArgument Arg(const A& a,
                         const B& b,
                         const C& c,
                         const D& d,
                         const E& e,
                         const F& f) {
    std::string output;
    addEntity(output, a);
    addEntity(output, b);
    addEntity(output, c);
    addEntity(output, d);
    addEntity(output, e);
    addEntity(output, f);
    return ExeArgument(6, output);
  }

  template <typename A,
            typename B,
            typename C,
            typename D,
            typename E,
            typename F,
            typename G>
  static ExeArgument Arg(const A& a,
                         const B& b,
                         const C& c,
                         const D& d,
                         const E& e,
                         const F& f,
                         const G& g) {
    std::string output;
    addEntity(output, a);
    addEntity(output, b);
    addEntity(output, c);
    addEntity(output, d);
    addEntity(output, e);
    addEntity(output, f);
    addEntity(output, g);
    return ExeArgument(7, output);
  }

  template <typename A,
            typename B,
            typename C,
            typename D,
            typename E,
            typename F,
            typename G,
            typename H>
  static ExeArgument Arg(const A& a,
                         const B& b,
                         const C& c,
                         const D& d,
                         const E& e,
                         const F& f,
                         const G& g,
                         const H& h) {
    std::string output;
    addEntity(output, a);
    addEntity(output, b);
    addEntity(output, c);
    addEntity(output, d);
    addEntity(output, e);
    addEntity(output, f);
    addEntity(output, g);
    addEntity(output, h);
    return ExeArgument(8, output);
  }

  template <typename A,
            typename B,
            typename C,
            typename D,
            typename E,
            typename F,
            typename G,
            typename H,
            typename I>
  static ExeArgument Arg(const A& a,
                         const B& b,
                         const C& c,
                         const D& d,
                         const E& e,
                         const F& f,
                         const G& g,
                         const H& h,
                         const I& i) {
    std::string output;
    addEntity(output, a);
    addEntity(output, b);
    addEntity(output, c);
    addEntity(output, d);
    addEntity(output, e);
    addEntity(output, f);
    addEntity(output, g);
    addEntity(output, h);
    addEntity(output, i);
    return ExeArgument(9, output);
  }

  TestMachine(System& in_system, libreallive::Archive& in_archive);

  // Index all the RLOperations before passing to parent.
  virtual void attachModule(RLModule* module);

  // Invokes a named opcode (with no arguments)
  void exe(const std::string& name, unsigned char overload);

  // Invokes a named opcode (with arguments)
  void exe(const std::string& name,
           unsigned char overload,
           const ExeArgument& arguments);

 private:
  // Adds a RealLive bytecode version of |arg| to |output|.
  static void addEntity(std::string& output, const std::string& arg);
  static void addEntity(std::string& output, const int arg);

  // Implementation of the two exe() methods.
  void runOpcode(const std::string& name,
                 unsigned char overload,
                 int argc,
                 const std::string& argument_string);

  typedef std::map<std::pair<std::string, unsigned char>, RLOperation*>
      OpcodeRegistry;
  OpcodeRegistry registry_;
};

#endif  // TEST_TEST_SYSTEM_TEST_MACHINE_H_
