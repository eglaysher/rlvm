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

#ifndef SRC_LIBREALLIVE_BYTECODE_H_
#define SRC_LIBREALLIVE_BYTECODE_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "libreallive/bytecode_fwd.h"
#include "libreallive/defs.h"
#include "libreallive/expression.h"

class RLMachine;

namespace libreallive {

class CommandElement;

// Returns a representation of the non-special cased function.
CommandElement* BuildFunctionElement(const char* stream);

void PrintParameterString(std::ostream& oss,
                          const std::vector<std::string>& paramseters);

struct ConstructionData {
  ConstructionData(size_t kt, pointer_t pt);
  ~ConstructionData();

  std::vector<unsigned long> kidoku_table;
  pointer_t null;
  typedef std::map<unsigned long, pointer_t> offsets_t;
  offsets_t offsets;
};

class Pointers {
 public:
  Pointers();
  ~Pointers();

  typedef std::vector<pointer_t>::iterator iterator;
  iterator begin() { return targets.begin(); }
  iterator end() { return targets.end(); }

  void reserve(size_t i) { target_ids.reserve(i); }
  void push_id(unsigned long id) { target_ids.push_back(id); }
  pointer_t& operator[] (long idx) { return targets.at(idx); }
  const pointer_t& operator[] (long idx) const { return targets.at(idx); }
  const size_t size() const { return targets.size(); }
  const size_t idSize() const { return target_ids.size(); }

  void SetPointers(ConstructionData& cdata);

 private:
  std::vector<unsigned long> target_ids;
  std::vector<pointer_t> targets;
};

// Base classes for bytecode elements.

class BytecodeElement {
 public:
  static const int kInvalidEntrypoint = -999;

  BytecodeElement();
  virtual ~BytecodeElement();

  // Prints a human readable version of this bytecode element to |oss|. This
  // tries to match Haeleth's kepago language as much as is feasible.
  virtual void PrintSourceRepresentation(RLMachine* machine,
                                         std::ostream& oss) const;

  // Returns the length of this element in bytes in the source file.
  virtual const size_t GetBytecodeLength() const = 0;

  // Used to connect pointers in the bytecode after we've created all
  // BytecodeElements in a Scenario.
  virtual void SetPointers(ConstructionData& cdata);

  // Needed for MetaElement during reading the script
  virtual const int GetEntrypoint() const;

  // Fat interface: takes a FunctionElement and returns all data serialized for
  // writing to disk so the exact command can be replayed later. Throws in all
  // other cases.
  virtual string GetSerializedCommand(RLMachine& machine) const;

  // Execute this bytecode instruction on this virtual machine
  virtual void RunOnMachine(RLMachine& machine) const;

  // Read the next element from a stream.
  static BytecodeElement* Read(const char* stream,
                               const char* end,
                               ConstructionData& cdata);

 protected:
  static char entrypoint_marker;
  BytecodeElement(const BytecodeElement& c);

 private:
  friend class Script;
};

class CommaElement : public BytecodeElement {
 public:
  CommaElement();
  virtual ~CommaElement();

  // Overridden from BytecodeElement:
  virtual void PrintSourceRepresentation(RLMachine* machine,
                                         std::ostream& oss) const final;
  virtual const size_t GetBytecodeLength() const final;
};

// Metadata elements: source line, kidoku, and entrypoint markers.
class MetaElement : public BytecodeElement {
 public:
  MetaElement(const ConstructionData* cv, const char* src);
  virtual ~MetaElement();

  const int value() const { return value_; }
  void set_value(const int value) { value_ = value; }

  // Overridden from BytecodeElement:
  virtual void PrintSourceRepresentation(RLMachine* machine,
                                         std::ostream& oss) const final;
  virtual const size_t GetBytecodeLength() const final;
  virtual const int GetEntrypoint() const final;
  virtual void RunOnMachine(RLMachine& machine) const final;

 private:
  enum MetaElementType { Line_ = '\n', Kidoku_ = '@', Entrypoint_ };
  MetaElementType type_;
  int value_;
  int entrypoint_index_;
};

// Display-text elements.
class TextoutElement : public BytecodeElement {
 public:
  TextoutElement(const char* src, const char* file_end);
  virtual ~TextoutElement();

  const string GetText() const;

  // Overridden from BytecodeElement::
  virtual void PrintSourceRepresentation(RLMachine* machine,
                                         std::ostream& oss) const final;
  virtual const size_t GetBytecodeLength() const final;
  virtual void RunOnMachine(RLMachine& machine) const final;

 private:
  string repr;
};

// Expression elements.
// Construct from long to build a representation of an integer constant.

// A BytecodeElement that represents an expression
class ExpressionElement : public BytecodeElement {
 public:
  explicit ExpressionElement(const long val);
  explicit ExpressionElement(const char* src);
  ExpressionElement(const ExpressionElement& rhs);
  virtual ~ExpressionElement();

  // Returns an ExpressionPiece representing this expression.
  const ExpressionPiece& ParsedExpression() const;

  // Overridden from BytecodeElement:
  virtual void PrintSourceRepresentation(RLMachine* machine,
                                         std::ostream& oss) const final;
  virtual const size_t GetBytecodeLength() const final;
  virtual void RunOnMachine(RLMachine& machine) const final;

 private:
  int length_;

  // Storage for the parsed expression so we only have to calculate
  // it once (and so we can return it by const reference)
  ExpressionPiece parsed_expression_;
};

// Command elements.

class CommandElement : public BytecodeElement {
 public:
  explicit CommandElement(const char* src);
  virtual ~CommandElement();

  // Identity information.
  const int modtype()  const { return command[1]; }
  const int module()   const { return command[2]; }
  const int opcode()   const { return command[3] | (command[4] << 8); }
  const int argc()     const { return command[5] | (command[6] << 8); }
  const int overload() const { return command[7]; }

  // Returns the raw byte strings of this command elements parameters.
  std::vector<string> GetUnparsedParameters() const;

  // Whether the RLOperation has cached the parsed versions of the parameters.
  bool AreParametersParsed() const;

  // Gets/Sets the cached parameters.
  void SetParsedParameters(ExpressionPiecesVector p) const;
  const ExpressionPiecesVector& GetParsedParameters() const;

  // Returns the number of parameters.
  virtual const size_t GetParamCount() const = 0;
  virtual string GetParam(int index) const = 0;

  // Methods that deal with pointers.
  virtual const size_t GetPointersCount() const;
  virtual pointer_t GetPointer(int i) const;

  // Fat interface stuff for GotoCase. Prevents casting, etc.
  virtual const size_t GetCaseCount() const;
  virtual const string GetCase(int i) const;

  // Overridden from BytecodeElement:
  virtual void PrintSourceRepresentation(RLMachine* machine,
                                         std::ostream& oss) const final;
  virtual void RunOnMachine(RLMachine& machine) const final;

 protected:
  static const int COMMAND_SIZE = 8;
  unsigned char command[COMMAND_SIZE];

  mutable std::vector<ExpressionPiece> parsed_parameters_;
};

class SelectElement : public CommandElement {
 public:
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

  explicit SelectElement(const char* src);
  virtual ~SelectElement();

  // Returns the expression in the source code which refers to which window to
  // display.
  ExpressionPiece GetWindowExpression() const;

  const params_t& raw_params() const { return params; }

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;

 private:
  string repr;
  params_t params;
  int firstline;
  int uselessjunk;
};

class FunctionElement : public CommandElement {
 public:
  FunctionElement(const char* src, const std::vector<string>& params);
  virtual ~FunctionElement();

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;
  virtual string GetSerializedCommand(RLMachine& machine) const final;

 private:
  std::vector<string> params;
};

class VoidFunctionElement : public CommandElement {
 public:
  explicit VoidFunctionElement(const char* src);
  virtual ~VoidFunctionElement();

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;
  virtual string GetSerializedCommand(RLMachine& machine) const final;
};

class SingleArgFunctionElement : public CommandElement {
 public:
  SingleArgFunctionElement(const char* src,
                           const std::string& arg);
  virtual ~SingleArgFunctionElement();

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;
  virtual string GetSerializedCommand(RLMachine& machine) const final;

 private:
  std::string arg_;
};

class PointerElement : public CommandElement {
 public:
  explicit PointerElement(const char* src);
  virtual ~PointerElement();

  // Overridden from CommandElement:
  virtual const size_t GetPointersCount() const final;
  virtual pointer_t GetPointer(int i) const final;

  // Overridden from BytecodeElement:
  virtual void SetPointers(ConstructionData& cdata) final;

 protected:
  Pointers targets;
};

class GotoElement : public CommandElement {
 public:
  GotoElement(const char* src, ConstructionData& cdata);
  virtual ~GotoElement();

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;
  virtual const size_t GetPointersCount() const final;
  virtual pointer_t GetPointer(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;
  virtual void SetPointers(ConstructionData& cdata) final;

 private:
  unsigned long id_;
  pointer_t pointer_;
};

class GotoIfElement : public CommandElement {
 public:
  GotoIfElement(const char* src, ConstructionData& cdata);
  virtual ~GotoIfElement();

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;
  virtual const size_t GetPointersCount() const final;
  virtual pointer_t GetPointer(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;
  virtual void SetPointers(ConstructionData& cdata) final;

 private:
  unsigned long id_;
  pointer_t pointer_;
  string repr;
};

class GotoCaseElement : public PointerElement {
 public:
  GotoCaseElement(const char* src, ConstructionData& cdata);
  virtual ~GotoCaseElement();

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;
  virtual const size_t GetCaseCount() const final;
  virtual const string GetCase(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;

 private:
  string repr;
  std::vector<string> cases;
};

class GotoOnElement : public PointerElement {
 public:
  GotoOnElement(const char* src, ConstructionData& cdata);
  virtual ~GotoOnElement();

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;

 private:
  string repr;
};

class GosubWithElement : public CommandElement {
 public:
  GosubWithElement(const char* src, ConstructionData& cdata);
  virtual ~GosubWithElement();

  // Overridden from CommandElement:
  virtual const size_t GetParamCount() const final;
  virtual string GetParam(int i) const final;
  virtual const size_t GetPointersCount() const final;
  virtual pointer_t GetPointer(int i) const final;

  // Overridden from BytecodeElement:
  virtual const size_t GetBytecodeLength() const final;
  virtual void SetPointers(ConstructionData& cdata) final;

 private:
  unsigned long id_;
  pointer_t pointer_;
  int repr_size;
  std::vector<string> params;
};

}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_BYTECODE_H_
