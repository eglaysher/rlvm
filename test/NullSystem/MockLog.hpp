// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
//
// -----------------------------------------------------------------------

#ifndef __MockLog_hpp__
#define __MockLog_hpp__

#include <string>
#include <sstream>
#include <vector>

/**
 * Keeps track of methods called and asserts that certain events happened.
 */
class MockLog
{
public:
  struct Record {
    Record(const std::string& name, const std::string& arg);
    std::string function_name_;
    std::string argument_str_;

    bool operator==(const Record& rhs) const;
  };

public:
  MockLog(const std::string& object_name);
  ~MockLog();

  /// Records a function with no arguments.
  void recordFunction(const std::string& name);

  template< typename A >
  void recordFunction(const std::string& name, const A& one);

  template< typename A, typename B >
  void recordFunction(const std::string& name, const A& one, const B& two);

  template< typename A, typename B, typename C >
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three);

  template< typename A, typename B, typename C, typename D >
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four);

  template< typename A, typename B, typename C, typename D, typename E >
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F>
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five,
                      const F& six);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G>
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five,
                      const F& six, const G& seven);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H>
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five,
                      const F& six, const G& seven, const H& eight);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I>
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five,
                      const F& six, const G& seven, const H& eight,
                      const I& nine);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I, typename J>
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five,
                      const F& six, const G& seven, const H& eight,
                      const I& nine, const J& ten);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I, typename J,
            typename K>
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five,
                      const F& six, const G& seven, const H& eight,
                      const I& nine, const J& ten, const K& eleven);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I, typename J,
            typename K, typename L>
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five,
                      const F& six, const G& seven, const H& eight,
                      const I& nine, const J& ten, const K& eleven,
                      const L& twelve);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I, typename J,
            typename K, typename L, typename M>
  void recordFunction(const std::string& name, const A& one, const B& two,
                      const C& three, const D& four, const E& five,
                      const F& six, const G& seven, const H& eight,
                      const I& nine, const J& ten, const K& eleven,
                      const L& twelve, const M& thirteen);

  /// Make sure a function was called
  void ensure(const std::string& name);

  template< typename A >
  void ensure(const std::string& name, const A& one);

  template< typename A, typename B >
  void ensure(const std::string& name, const A& one, const B& two);

  template< typename A, typename B, typename C >
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three);

  template< typename A, typename B, typename C, typename D >
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four);

  template< typename A, typename B, typename C, typename D, typename E >
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F>
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five,
              const F& six);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G>
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five,
              const F& six, const G& seven);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H>
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five,
              const F& six, const G& seven, const H& eight);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I>
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five,
              const F& six, const G& seven, const H& eight,
              const I& nine);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I, typename J>
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five,
              const F& six, const G& seven, const H& eight,
              const I& nine, const J& ten);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I, typename J,
            typename K>
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five,
              const F& six, const G& seven, const H& eight,
              const I& nine, const J& ten, const K& eleven);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I, typename J,
            typename K, typename L>
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five,
              const F& six, const G& seven, const H& eight,
              const I& nine, const J& ten, const K& eleven,
              const L& twelve);

  template< typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H, typename I, typename J,
            typename K, typename L, typename M>
  void ensure(const std::string& name, const A& one, const B& two,
              const C& three, const D& four, const E& five,
              const F& six, const G& seven, const H& eight,
              const I& nine, const J& ten, const K& eleven,
              const L& twelve, const M& thirteen);

  /// Clears out all function call records.
  void clear() { records_.clear(); }

private:
  // Checks whether a function was called. Throws whatever TUT throws
  // when it fails an ensure().
  void ensureInternal(const std::string& name,
                      const std::string& arguments) const;

  // Records the
  void recordFunctionInternal(const std::string& name,
                              const std::string& arguments);

  std::string object_name_;

  std::vector<Record> records_;

  friend std::ostream& operator<<(std::ostream& os, const MockLog& log);
};  // end of class MockLog

// -----------------------------------------------------------------------

template< typename A >
void MockLog::recordFunction(const std::string& name, const A& one) {
  std::ostringstream oss;
  oss << one;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B >
void MockLog::recordFunction(const std::string& name, const A& one,
                             const B& two) {
  std::ostringstream oss;
  oss << one << ", " << two;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C >
void MockLog::recordFunction(const std::string& name, const A& one,
                             const B& two, const C& three) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D >
void MockLog::recordFunction(const std::string& name, const A& one,
                             const B& two, const C& three, const D& four) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E >
void MockLog::recordFunction(const std::string& name, const A& one,
                             const B& two, const C& three, const D& four,
                             const E& five) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F>
void MockLog::recordFunction(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G>
void MockLog::recordFunction(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H>
void MockLog::recordFunction(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I>
void MockLog::recordFunction(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I, typename J>
void MockLog::recordFunction(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine, const J& ten) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine << ", " << ten;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I, typename J,
          typename K>
void MockLog::recordFunction(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine, const J& ten, const K& eleven) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine << ", " << ten << ", " << eleven;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I, typename J,
          typename K, typename L>
void MockLog::recordFunction(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine, const J& ten, const K& eleven,
                             const L& twelve) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine << ", " << ten << ", " << eleven << ", " << twelve;
  recordFunctionInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I, typename J,
          typename K, typename L, typename M>
void MockLog::recordFunction(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine, const J& ten, const K& eleven,
                             const L& twelve, const M& thirteen) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine << ", " << ten << ", " << eleven << ", " << twelve << ", "
      << thirteen;
  recordFunctionInternal(name, oss.str());
}

// -----------------------------------------------------------------------

template< typename A >
void MockLog::ensure(const std::string& name, const A& one) {
  std::ostringstream oss;
  oss << one;
  ensureInternal(name, oss.str());
}

template< typename A, typename B >
void MockLog::ensure(const std::string& name, const A& one,
                             const B& two) {
  std::ostringstream oss;
  oss << one << ", " << two;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C >
void MockLog::ensure(const std::string& name, const A& one,
                             const B& two, const C& three) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D >
void MockLog::ensure(const std::string& name, const A& one,
                             const B& two, const C& three, const D& four) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E >
void MockLog::ensure(const std::string& name, const A& one,
                             const B& two, const C& three, const D& four,
                             const E& five) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F>
void MockLog::ensure(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G>
void MockLog::ensure(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H>
void MockLog::ensure(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I>
void MockLog::ensure(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I, typename J>
void MockLog::ensure(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine, const J& ten) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine << ", " << ten;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I, typename J,
          typename K>
void MockLog::ensure(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine, const J& ten, const K& eleven) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine << ", " << ten << ", " << eleven;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I, typename J,
          typename K, typename L>
void MockLog::ensure(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine, const J& ten, const K& eleven,
                             const L& twelve) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine << ", " << ten << ", " << eleven << ", " << twelve;
  ensureInternal(name, oss.str());
}

template< typename A, typename B, typename C, typename D, typename E,
          typename F, typename G, typename H, typename I, typename J,
          typename K, typename L, typename M>
void MockLog::ensure(const std::string& name, const A& one, const B& two,
                             const C& three, const D& four, const E& five,
                             const F& six, const G& seven, const H& eight,
                             const I& nine, const J& ten, const K& eleven,
                             const L& twelve, const M& thirteen) {
  std::ostringstream oss;
  oss << one << ", " << two << ", " << three << ", " << four << ", "
      << five << ", " << six << ", " << seven << ", " << eight << ", "
      << nine << ", " << ten << ", " << eleven << ", " << twelve << ", "
      << thirteen;
  ensureInternal(name, oss.str());
}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const MockLog& log);

#endif
