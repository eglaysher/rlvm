// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of libReallive, a dependency of RLVM.
//
// -----------------------------------------------------------------------
//
// Copyright (c) 2006, 2007 Peter Jolly
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// -----------------------------------------------------------------------

#ifndef BYTECODE_H
#define BYTECODE_H

#include "defs.h"
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "bytecode_fwd.h"

class RLMachine;

namespace libReallive {

struct ConstructionData {
  std::vector<unsigned long> kidoku_table;
  pointer_t null;
  typedef std::map<unsigned long, pointer_t> offsets_t;
  offsets_t offsets;
private:
  friend class Script;
  ConstructionData(size_t kt, pointer_t pt);
  ~ConstructionData();
};

class Pointers {
  std::vector<unsigned long> target_ids;
  std::vector<pointer_t> targets;
public:
  typedef std::vector<pointer_t> t;
  typedef t::iterator iterator;
  iterator begin() { return targets.begin(); }
  iterator end() { return targets.end(); }

  void reserve(size_t i) { target_ids.reserve(i); }
  void push_id(unsigned long id) { target_ids.push_back(id); }
  pointer_t& operator[] (long idx) { return targets.at(idx); }
  const pointer_t& operator[] (long idx) const { return targets.at(idx); }
  const size_t size() const { return targets.size(); }
  const size_t idSize() const { return target_ids.size(); }

  void set_pointers(ConstructionData& cdata);
};

// Base classes for bytecode elements.

class BytecodeElement {
  friend class Script;
  size_t offset_;
protected:
  static char entrypoint_marker;
  static long id_src;
  BytecodeElement(const BytecodeElement& c);
public:
  const long id;

  virtual const ElementType type() const;

  const size_t offset() const;

  virtual const string data() const;
  virtual const size_t length() const;

  virtual Pointers* get_pointers();
  virtual void set_pointers(ConstructionData& cdata);

  // Note that x.clone() != x, since the copy constructor assigns a new id.
  virtual BytecodeElement* clone() const;

  virtual ~BytecodeElement();

  BytecodeElement();

  /// Execute this bytecode instruction on this virtual machine
  virtual void runOnMachine(RLMachine& machine) const;

  // Needed for MetaElement during reading the script
  virtual const int entrypoint() const;

  // Read the next element from a stream.
  static BytecodeElement* read(const char* stream, ConstructionData& cdata);
};

inline BytecodeElement* new_clone(const BytecodeElement& e)
{
  return e.clone();
}

class DataElement : public BytecodeElement {
protected:
  string repr;
public:
  virtual const ElementType type() const;
  virtual const string data() const;
  virtual const size_t length() const;

  virtual DataElement* clone() const;
  DataElement();
  DataElement(const char* src, const size_t count);
  ~DataElement();
};

} namespace std {
template<> struct less<libReallive::pointer_t> {
  bool operator() (const libReallive::pointer_t& a,
                   const libReallive::pointer_t& b) const {
    return a->id < b->id;
  }
};
} namespace libReallive {

// Metadata elements: source line, kidoku, and entrypoint markers.

class MetaElement : public BytecodeElement {
  enum MetaElementType { Line_ = '\n', Kidoku_ = '@', Entrypoint_ };
  MetaElementType type_;
  int value_;
  int entrypoint_index;
public:
  virtual const ElementType type() const;
  virtual const string data() const;
  virtual const size_t length() const;

  const int value() const { return value_; }
  void set_value(const int value) { value_ = value; }
  const int entrypoint() const;

  /// Execute this bytecode instruction on this virtual machine
  virtual void runOnMachine(RLMachine& machine) const;

  MetaElement(const ConstructionData* cv, const char* src);
  ~MetaElement();

  virtual MetaElement* clone() const;
};

// Display-text elements.

class TextoutElement : public DataElement {
public:
  virtual const ElementType type() const;
  const string text() const;
  void set_text(const char* src);
  void set_text(const string& src) { set_text(src.c_str()); }
  TextoutElement(const char* src);
  TextoutElement();
  TextoutElement* clone() const;

  /// Execute this bytecode instruction on this virtual machine
  virtual void runOnMachine(RLMachine& machine) const;
};

// Expression elements.
// Construct from long to build a representation of an integer constant.

/**
 * A BytecodeElement that represents an expression
 */
class ExpressionElement : public DataElement {
private:
  /// Storage for the parsed expression so we only have to calculate
  /// it once (and so we can return it by const reference)
  mutable boost::scoped_ptr<ExpressionPiece> m_parsedExpression;

public:
  virtual const ElementType type() const;
  ExpressionElement(const long val);
  ExpressionElement(const char* src);
  ExpressionElement(const ExpressionElement& rhs);
  ExpressionElement* clone() const;

  /**
   * Returns an ExpressionPiece representing this expression. This
   * function lazily parses the expression and stores the tree for
   * reuse.
   *
   * @return A parsed expression tree
   * @see expression.cpp
   */
  const ExpressionPiece& parsedExpression() const;

  virtual void runOnMachine(RLMachine& machine) const;
};

// Command elements.

class CommandElement : public DataElement {
protected:
  mutable std::vector<std::string> m_unparsedParameters;
  mutable boost::ptr_vector<libReallive::ExpressionPiece> m_parsedParameters;

public:
  virtual const ElementType type() const;
  const int modtype()  const { return repr[1]; }
  const int module()   const { return repr[2]; }
  const int opcode()   const { return repr[3] | (repr[4] << 8); }
  const int argc()     const { return repr[5] | (repr[6] << 8); }
  const int overload() const { return repr[7]; }

  void set_modtype(unsigned char to)  { repr[1] = to; }
  void set_module(unsigned char to)   { repr[2] = to; }
  void set_opcode(int to)             { repr[3] = to & 0xff; repr[4] = (to >> 8) & 0xff; }
  void set_argc(int to)               { repr[5] = to & 0xff; repr[6] = (to >> 8) & 0xff; }
  void set_overload(unsigned char to) { repr[7] = to; }

  virtual const size_t param_count() const = 0;
  virtual string get_param(int) const = 0;

  const std::vector<string>& getUnparsedParameters() const;
  bool areParametersParsed() const;

  void setParsedParameters(boost::ptr_vector<libReallive::ExpressionPiece>& p) const;
  const boost::ptr_vector<libReallive::ExpressionPiece>& getParameters() const;

  /// Get pointer reference. I consider the fatter interface the lesser of two
  /// evils between this and casting CommandElements to their subclasses.
  virtual const Pointers& get_pointersRef() const {
    static Pointers falseTargets;
    return falseTargets;
  }

  // Fat interface stuff for GotoCase. Prevents casting, etc.
  virtual const size_t case_count() const { return 0; }
  virtual const string get_case(int i) const { return ""; }

  CommandElement(const int type, const int module, const int opcode, const int argc, const int overload);
  CommandElement(const char* src);
  CommandElement(const CommandElement& ce);
  ~CommandElement();

  virtual void runOnMachine(RLMachine& machine) const;
};

class SelectElement : public CommandElement {
public:
  struct Param {
    string cond, text;
    int line;
    Param() : cond(), text(), line(0) {}
    Param(const char* tsrc, const size_t tlen, const int lnum) :
      cond(), text(tsrc, tlen), line(lnum) {}
    Param(const char* csrc, const size_t clen, const char* tsrc, const size_t tlen, const int lnum) :
      cond(csrc, clen), text(tsrc, tlen), line(lnum) {}
  };
  typedef std::vector<Param> params_t;
private:
  params_t params;
  int firstline;
public:
  virtual const ElementType type() const;
  ExpressionElement window();
  const string text(const int index) const;

  const string data() const;
  const size_t length() const;

  const size_t param_count() const;
  string get_param(int i) const;

  const params_t& getRawParams() const { return params; }

  SelectElement(const char* src);
  SelectElement* clone() const;
};

class FunctionElement : public CommandElement {
  std::vector<string> params;
public:
  virtual const ElementType type() const;
  FunctionElement(const char* src);

  const string data() const;
  const size_t length() const;

  const size_t param_count() const;
  string get_param(int i) const;

  virtual FunctionElement* clone() const;
};

class PointerElement : public CommandElement {
protected:
  Pointers targets;
public:
  PointerElement(const char* src);
  ~PointerElement();
  virtual const ElementType type() const = 0;
  virtual PointerElement* clone() const = 0;
  virtual const string data() const = 0;
  virtual const size_t length() const = 0;
  virtual void set_pointers(ConstructionData& cdata);
  virtual Pointers* get_pointers();
  virtual const Pointers& get_pointersRef() const;
};

class GotoElement : public PointerElement {
//	std::vector<string> params;
public:
  virtual const ElementType type() const;
  GotoElement(const char* src, ConstructionData& cdata);
  GotoElement* clone() const;

  void make_unconditional();

  enum Case { Unconditional, Always, Never, Variable };
  const Case taken() const;

  // The pointer is not counted as a parameter.
  const size_t param_count() const { return repr.size() == 8 ? 0 : 1; }
  string get_param(int i) const { return i == 0 ? (repr.size() == 8 ? string() : repr.substr(9, repr.size() - 10)) : string(); }
//	const size_t param_count() const { return params.size(); }
//	string get_param(int i) const { return params[i]; }

  const string data() const;
  const size_t length() const { return repr.size() + 4; }
};

class GotoCaseElement : public PointerElement {
  std::vector<string> cases;
public:
  virtual const ElementType type() const;
  virtual const string data() const;
  virtual const size_t length() const;

  GotoCaseElement(const char* src, ConstructionData& cdata);
  GotoCaseElement* clone() const;

  // The cases are not counted as parameters.
  const size_t param_count() const { return 1; }
  string get_param(int i) const { return i == 0 ? repr.substr(8, repr.size() - 8) : string(); }

  // Accessors for the cases
  const size_t case_count() const { return cases.size(); }
  const string get_case(int i) const { return cases[i]; }
};

class GotoOnElement : public PointerElement {
public:
  virtual const ElementType type() const;
  virtual const string data() const;
  virtual const size_t length() const;

  GotoOnElement(const char* src, ConstructionData& cdata);
  GotoOnElement* clone() const;

  // The pointers are not counted as parameters.
  const size_t param_count() const { return 1; }
  string get_param(int i) const { return i == 0 ? repr.substr(8, repr.size() - 8) : string(); }
};

class GosubWithElement : public PointerElement {
  std::vector<string> params;
public:
  virtual const ElementType type() const;
  virtual const string data() const;
  virtual const size_t length() const;

  GosubWithElement(const char* src, ConstructionData& cdata);
  GosubWithElement* clone() const;

  void make_unconditional();

  enum Case { Unconditional, Always, Never, Variable };
  const Case taken() const;

  // The pointer is not counted as a parameter.
//	const size_t param_count() const { return repr.size() == 8 ? 0 : 1; }
//	string get_param(int i) const { return i == 0 ? (repr.size() == 8 ? string() : repr.substr(9, repr.size() - 10)) : string(); }
  const size_t param_count() const { return params.size(); }
  string get_param(int i) const { return params[i]; }
//  virtual const boost::ptr_vector<libReallive::ExpressionPiece>& getParameters() const;

};


}

#endif
