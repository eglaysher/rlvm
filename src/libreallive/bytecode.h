// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of libreallive, a dependency of RLVM.
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
#include <stdint.h>

#include "bytecode_fwd.h"
#include "expression.h"

class RLMachine;

namespace libreallive {

class CommandElement;

// Returns a representation of the non-special cased function.
CommandElement* BuildFunctionElement(const char* stream);

void PrintParameterString(std::ostream& oss,
                          const std::vector<std::string>& paramseters);

struct ConstructionData {
  std::vector<unsigned long> kidoku_table;
  pointer_t null;
  typedef std::map<unsigned long, pointer_t> offsets_t;
  offsets_t offsets;

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
protected:
  static char entrypoint_marker;
  BytecodeElement(const BytecodeElement& c);
public:
  virtual const ElementType type() const;
  virtual void print(std::ostream& oss) const;

  virtual const size_t length() const = 0;

  // Fat interface: takes a FunctionElement and returns all data serialized for
  // writing to disk so the exact command can be replayed later. Throws in all
  // other cases.
  virtual string serializableData(RLMachine& machine) const;

  virtual void set_pointers(ConstructionData& cdata);

  virtual ~BytecodeElement();

  BytecodeElement();

  // Execute this bytecode instruction on this virtual machine
  virtual void runOnMachine(RLMachine& machine) const;

  // Needed for MetaElement during reading the script
  virtual const int entrypoint() const;

  // Read the next element from a stream.
  static BytecodeElement* read(const char* stream, const char* end,
                               ConstructionData& cdata);
};

class CommaElement : public BytecodeElement {
 public:
  virtual const ElementType type() const;
  virtual void print(std::ostream& oss) const;
  virtual const size_t length() const;

  CommaElement();
  ~CommaElement();
};

// Metadata elements: source line, kidoku, and entrypoint markers.

class MetaElement : public BytecodeElement {
  enum MetaElementType { Line_ = '\n', Kidoku_ = '@', Entrypoint_ };
  MetaElementType type_;
  int value_;
  int entrypoint_index;
public:
  virtual const ElementType type() const;
  virtual void print(std::ostream& oss) const;
  virtual const size_t length() const;

  const int value() const { return value_; }
  void set_value(const int value) { value_ = value; }
  const int entrypoint() const;

  // Execute this bytecode instruction on this virtual machine
  virtual void runOnMachine(RLMachine& machine) const;

  MetaElement(const ConstructionData* cv, const char* src);
  ~MetaElement();
};

// Display-text elements.

class TextoutElement : public BytecodeElement {
 public:
  virtual const ElementType type() const;
  virtual void print(std::ostream& oss) const;
  virtual const size_t length() const;
  const string text() const;
  TextoutElement(const char* src, const char* file_end);
  TextoutElement();

  // Execute this bytecode instruction on this virtual machine
  virtual void runOnMachine(RLMachine& machine) const;

 private:
  string repr;
};

// Expression elements.
// Construct from long to build a representation of an integer constant.

/**
 * A BytecodeElement that represents an expression
 */
class ExpressionElement : public BytecodeElement {
 public:
  virtual const ElementType type() const;
  virtual void print(std::ostream& oss) const;
  virtual const size_t length() const;
  ExpressionElement(const long val);
  ExpressionElement(const char* src);
  ExpressionElement(const ExpressionElement& rhs);

  // Assumes the expression isn't an assignment and returns the integer value.
  int valueOnly(RLMachine& machine) const;

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

 private:
  string repr;

  // Storage for the parsed expression so we only have to calculate
  // it once (and so we can return it by const reference)
  mutable std::unique_ptr<ExpressionPiece> parsed_expression_;
};

// Command elements.

class CommandElement : public BytecodeElement {
 public:
  virtual const ElementType type() const;
  virtual void print(std::ostream& oss) const;

  const int modtype()  const { return command[1]; }
  const int module()   const { return command[2]; }
  const int opcode()   const { return command[3] | (command[4] << 8); }
  const int argc()     const { return command[5] | (command[6] << 8); }
  const int overload() const { return command[7]; }

  virtual const size_t param_count() const = 0;
  virtual string get_param(int) const = 0;

  std::vector<string> getUnparsedParameters() const;
  bool areParametersParsed() const;

  void setParsedParameters(ExpressionPiecesVector& p) const;
  const ExpressionPiecesVector& getParameters() const;

  // Methods that deal with pointers.
  virtual const size_t pointers_count() const { return 0; }
  virtual pointer_t get_pointer(int i) const { return pointer_t(); }

  // Fat interface stuff for GotoCase. Prevents casting, etc.
  virtual const size_t case_count() const { return 0; }
  virtual const string get_case(int i) const { return ""; }

  CommandElement(const char* src);
  ~CommandElement();

  virtual void runOnMachine(RLMachine& machine) const;

 protected:
  static const int COMMAND_SIZE = 8;
  unsigned char command[COMMAND_SIZE];

  mutable std::vector<std::unique_ptr<ExpressionPiece> > parsed_parameters_;
};

class SelectElement : public CommandElement {
public:
  string repr;

  static const int OPTION_COLOUR = 0x30;
  static const int OPTION_TITLE = 0x31;
  static const int OPTION_HIDE = 0x32;
  static const int OPTION_BLANK = 0x33;
  static const int OPTION_CURSOR = 0x34;

  struct Condition {
    string condition;
    uint8_t effect;
    string effect_argument;
  };

  struct Param {
    std::vector<Condition> cond_parsed;
    string cond_text;
    string text;
    int line;
    Param() : cond_text(), text(), line(0) {}
    Param(const char* tsrc, const size_t tlen, const int lnum)
        : cond_text(), text(tsrc, tlen), line(lnum) {}
    Param(const std::vector<Condition>& conditions,
          const char* csrc, const size_t clen,
          const char* tsrc, const size_t tlen, const int lnum)
        : cond_parsed(conditions), cond_text(csrc, clen), text(tsrc, tlen),
          line(lnum) {}
  };
  typedef std::vector<Param> params_t;
private:
  params_t params;
  int firstline;
  int uselessjunk;
public:
  virtual const ElementType type() const;
  ExpressionElement window() const;
  const string text(const int index) const;

  const size_t length() const;

  const size_t param_count() const;
  string get_param(int i) const;

  const params_t& getRawParams() const { return params; }

  SelectElement(const char* src);
};

class FunctionElement : public CommandElement {
 public:
  virtual const ElementType type() const;
  FunctionElement(const char* src, const std::vector<string>& params);

  virtual const size_t length() const;
  virtual string serializableData(RLMachine& machine) const;

  virtual const size_t param_count() const;
  virtual string get_param(int i) const;

 private:
  std::vector<string> params;
};

class VoidFunctionElement : public CommandElement {
 public:
  virtual const ElementType type() const;
  VoidFunctionElement(const char* src);

  virtual const size_t length() const;
  virtual string serializableData(RLMachine& machine) const;

  virtual const size_t param_count() const;
  virtual string get_param(int i) const;
};

class SingleArgFunctionElement : public CommandElement {
 public:
  virtual const ElementType type() const;
  SingleArgFunctionElement(const char* src,
                           const std::string& arg);

  virtual const size_t length() const;
  virtual string serializableData(RLMachine& machine) const;

  virtual const size_t param_count() const;
  virtual string get_param(int i) const;

 private:
  std::string arg_;
};

class PointerElement : public CommandElement {
 public:
  PointerElement(const char* src);
  ~PointerElement();

  virtual void set_pointers(ConstructionData& cdata);
  virtual const size_t pointers_count() const;
  virtual pointer_t get_pointer(int i) const;

 protected:
  Pointers targets;
};

class GotoElement : public CommandElement {
 public:
  virtual const ElementType type() const;
  GotoElement(const char* src, ConstructionData& cdata);

  // The pointer is not counted as a parameter.
  virtual const size_t param_count() const;
  virtual string get_param(int i) const;
  virtual const size_t length() const;

  virtual void set_pointers(ConstructionData& cdata);
  virtual const size_t pointers_count() const;
  virtual pointer_t get_pointer(int i) const;

 private:
  unsigned long id_;
  pointer_t pointer_;
};

class GotoIfElement : public CommandElement {
 public:
  virtual const ElementType type() const;
  GotoIfElement(const char* src, ConstructionData& cdata);

  // The pointer is not counted as a parameter.
  virtual const size_t param_count() const;
  virtual string get_param(int i) const;
  virtual const size_t length() const;

  virtual void set_pointers(ConstructionData& cdata);
  virtual const size_t pointers_count() const;
  virtual pointer_t get_pointer(int i) const;

 private:
  unsigned long id_;
  pointer_t pointer_;
  string repr;
};

class GotoCaseElement : public PointerElement {
 public:
  virtual const ElementType type() const;
  virtual const size_t length() const;

  GotoCaseElement(const char* src, ConstructionData& cdata);

  // The cases are not counted as parameters.
  virtual const size_t param_count() const;
  virtual string get_param(int i) const;

  // Accessors for the cases
  const size_t case_count() const { return cases.size(); }
  const string get_case(int i) const { return cases[i]; }

 private:
  string repr;
  std::vector<string> cases;
};

class GotoOnElement : public PointerElement {
 public:
  virtual const ElementType type() const;
  virtual const size_t length() const;

  GotoOnElement(const char* src, ConstructionData& cdata);

  // The pointers are not counted as parameters.
  virtual const size_t param_count() const;
  virtual string get_param(int i) const;

 private:
  string repr;
};

class GosubWithElement : public CommandElement {
 public:
  virtual const ElementType type() const;
  virtual const size_t length() const;

  GosubWithElement(const char* src, ConstructionData& cdata);

  // The pointer is not counted as a parameter.
  virtual const size_t param_count() const;
  virtual string get_param(int i) const;

  virtual void set_pointers(ConstructionData& cdata);
  virtual const size_t pointers_count() const;
  virtual pointer_t get_pointer(int i) const;

 private:
  unsigned long id_;
  pointer_t pointer_;
  int repr_size;
  std::vector<string> params;
};

}

#endif
