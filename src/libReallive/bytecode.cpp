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

/* Bytecode class structure. */

#include <iomanip>
#include <exception>
#include <utility>
#include <sstream>

#include "bytecode.h"
#include "scenario.h"
#include "expression.h"

#include "MachineBase/RLMachine.hpp"

using namespace std;

namespace libReallive {

char BytecodeElement::entrypoint_marker = '@';

CommandElement* BuildFunctionElement(const char* stream) {
  const char* ptr = stream;
  ptr += 8;
  std::vector<string> params;
  if (*ptr == '(') {
    const char* end = ptr + 1;
    while (*end != ')') {
      const size_t len = next_data(end);
      params.push_back(string(end, len));
      end += len;
    }
  }

  if (params.size() == 0)
    return new VoidFunctionElement(stream);
  else if (params.size() == 1)
    return new SingleArgFunctionElement(stream, params.front());
  else
    return new FunctionElement(stream, params);
}

void PrintParameterString(std::ostream& oss,
                          const std::vector<std::string>& parameters) {
  bool first = true;
  oss << "(";
  for (std::string const& param : parameters) {
    if (!first) {
      oss << ", ";
    }
    first = false;

    // Take the binary stuff and try to get usefull, printable values.
    const char* start = param.c_str();
    try {
      std::unique_ptr<ExpressionPiece> piece(get_data(start));
      oss << piece->getDebugString();
    } catch (libReallive::Error& e) {
      // Any error throw here is a parse error.
      oss << "{RAW : " << parsableToPrintableString(param) << "}";
    }
  }
  oss << ")";
}

// -----------------------------------------------------------------------
// ConstructionData
// -----------------------------------------------------------------------

ConstructionData::ConstructionData(size_t kt, pointer_t pt)
  : kidoku_table(kt), null(pt) {}

// -----------------------------------------------------------------------

ConstructionData::~ConstructionData() {}

// -----------------------------------------------------------------------
// BytecodeElement
// -----------------------------------------------------------------------

BytecodeElement::BytecodeElement(const BytecodeElement& c) {}

// -----------------------------------------------------------------------

const ElementType BytecodeElement::type() const {
  return Unspecified;
}

void BytecodeElement::print(std::ostream& oss) const {
  oss << "<unspecified bytecode>" << endl;
}

// -----------------------------------------------------------------------

string BytecodeElement::serializableData(RLMachine& machine) const {
  throw Error(
      "Can't call serializableData() on things other than FunctionElements");
}

// -----------------------------------------------------------------------

void BytecodeElement::set_pointers(ConstructionData& cdata) {}

// -----------------------------------------------------------------------

BytecodeElement::~BytecodeElement() {}

// -----------------------------------------------------------------------

BytecodeElement::BytecodeElement() {}

// -----------------------------------------------------------------------

const int BytecodeElement::entrypoint() const { return -999; }

// -----------------------------------------------------------------------

inline BytecodeElement*
read_function(const char* stream, ConstructionData& cdata) {
  // opcode: 0xttmmoooo (Type, Module, Opcode: e.g. 0x01030101 = 1:03:00257
  const unsigned long opcode = (stream[1] << 24) | (stream[2] << 16) |
    (stream[4] << 8) | stream[3];
  switch (opcode) {
  case 0x00010000: case 0x00010005:
  case 0x00050001: case 0x00050005:
    return new GotoElement(stream, cdata);
  case 0x00010001: case 0x00010002: case 0x00010006: case 0x00010007:
  case 0x00050002: case 0x00050006: case 0x00050007:
    return new GotoIfElement(stream, cdata);
  case 0x00010003: case 0x00010008:
  case 0x00050003: case 0x00050008:
    return new GotoOnElement(stream, cdata);
  case 0x00010004: case 0x00010009:
  case 0x00050004: case 0x00050009:
    return new GotoCaseElement(stream, cdata);
  case 0x00010010:
    return new GosubWithElement(stream, cdata);

  // Select elements.
  case 0x00020000: case 0x00020001:
  case 0x00020002: case 0x00020003:
  case 0x00020010:
    return new SelectElement(stream);
  }

  return BuildFunctionElement(stream);
}

// -----------------------------------------------------------------------

void BytecodeElement::runOnMachine(RLMachine& machine) const {
  machine.advanceInstructionPointer();
}

// -----------------------------------------------------------------------

BytecodeElement*
BytecodeElement::read(const char* stream, const char* end,
                      ConstructionData& cdata) {
  const char c = *stream;
  if (c == '!') entrypoint_marker = '!';
  switch (c) {
  case 0:
  case ',':  return new CommaElement;
  case '\n': return new MetaElement(0, stream);
  case '@':  // fall through
  case '!':  return new MetaElement(&cdata, stream);
  case '$':  return new ExpressionElement(stream);
  case '#':  return read_function(stream, cdata);
  default:   return new TextoutElement(stream, end);
  }
}

// -----------------------------------------------------------------------
// CommaElement
// -----------------------------------------------------------------------

CommaElement::CommaElement() {}
CommaElement::~CommaElement() {}

const ElementType CommaElement::type() const {
  return Data;
}

void CommaElement::print(std::ostream& oss) const {
  oss << "<CommaElement>" << endl;
}

const size_t CommaElement::length() const {
  return 1;
}

// -----------------------------------------------------------------------
// MetaElement
// -----------------------------------------------------------------------

MetaElement::MetaElement(const ConstructionData* cv, const char* src) {
  value_ = read_i16(src + 1);
  if (!cv) {
    type_ = Line_;
  } else if (cv->kidoku_table.at(value_) >= 1000000) {
    type_ = Entrypoint_;
    entrypoint_index = cv->kidoku_table[value_] - 1000000;
  } else {
    type_ = Kidoku_;
  }
}

// -----------------------------------------------------------------------

MetaElement::~MetaElement() {}

// -----------------------------------------------------------------------

const ElementType MetaElement::type() const {
  return type_ == Line_ ? Line
    : (type_ == Kidoku_ ? Kidoku
       : Entrypoint);
}

void MetaElement::print(std::ostream& oss) const {
  if (type_ == Line_)
    oss << "#line " << value_ << endl;
  else if (type_ == Entrypoint_)
    oss << "#entrypoint " << value_ << endl;
  else
    oss << "{- Kidoku " << value_ << " -}" << endl;
}

// -----------------------------------------------------------------------

const size_t MetaElement::length() const { return 3; }

// -----------------------------------------------------------------------

const int MetaElement::entrypoint() const {
  return type_ == Entrypoint_ ? entrypoint_index : -999;
}

// -----------------------------------------------------------------------

void MetaElement::runOnMachine(RLMachine& machine) const {
  if (type_ == Line_)
    machine.setLineNumber(value_);
  else if (type_ == Kidoku_)
    machine.setKidokuMarker(value_);

  machine.advanceInstructionPointer();
}

// -----------------------------------------------------------------------
// TextoutElement
// -----------------------------------------------------------------------

TextoutElement::TextoutElement(const char* src, const char* file_end) {
  const char* end = src;
  bool quoted = false;
  while (true && end < file_end) {
    if (quoted) {
      quoted = *end != '"';
      if (*end == '\\' && end[1] == '"') ++end;
    } else {
      if (*end == ',') ++end;
      quoted = *end == '"';
      if (!*end || *end == '#' || *end == '$' || *end == '\n' ||
          *end == '@' || *end == entrypoint_marker)
        break;
    }
    if ((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef))
      end += 2;
    else
      ++end;
  }
  repr.assign(src, end);
}

// -----------------------------------------------------------------------

TextoutElement::TextoutElement() {}

// -----------------------------------------------------------------------

const ElementType TextoutElement::type() const { return Textout; }

void TextoutElement::print(std::ostream& oss) const {
  oss << "\"" << text() << "\"" << endl;
}

const size_t TextoutElement::length() const { return repr.size(); }

// -----------------------------------------------------------------------

const string
TextoutElement::text() const {
  string rv;
  bool quoted = false;
  string::const_iterator it = repr.begin();
  while (it != repr.end()) {
    if (*it == '"') {
      ++it;
      quoted = !quoted;
    } else if (quoted && *it == '\\') {
      ++it;
      if (*it == '"') {
        ++it;
        rv.push_back('"');
      } else rv.push_back('\\');
    } else {
      if ((*it >= 0x81 && *it <= 0x9f) || (*it >= 0xe0 && *it <= 0xef))
        rv.push_back(*it++);
      rv.push_back(*it++);
    }
  }
  return rv;
}

// -----------------------------------------------------------------------

void TextoutElement::runOnMachine(RLMachine& machine) const {
  machine.performTextout(*this);
  machine.advanceInstructionPointer();
}

// -----------------------------------------------------------------------
// ExpressionElement
// -----------------------------------------------------------------------

ExpressionElement::ExpressionElement(const char* src) {
  // Don't parse the expression, just isolate it.
  const char* end = src;
  end += next_token(end);
  if (*end == '\\') {
    end += 2;
    end += next_expr(end);
  }
  repr.assign(src, end);
}

// -----------------------------------------------------------------------

ExpressionElement::ExpressionElement(const long val) {
  repr.resize(6, '$');
  repr[1] = 0xff;
  insert_i32(repr, 2, val);
}

// -----------------------------------------------------------------------

const ElementType ExpressionElement::type() const {
  return Expression;
}

void ExpressionElement::print(std::ostream& oss) const {
  oss << parsedExpression().getDebugString() << endl;
}

const size_t ExpressionElement::length() const {
  return repr.size();
}

// -----------------------------------------------------------------------

ExpressionElement::ExpressionElement(const ExpressionElement& rhs)
    : parsed_expression_(nullptr) {
}

// -----------------------------------------------------------------------

int ExpressionElement::valueOnly(RLMachine& machine) const {
  const char* location = repr.c_str();
  std::unique_ptr<ExpressionPiece> e(get_expression(location));
  return e->integerValue(machine);
}

// -----------------------------------------------------------------------

const ExpressionPiece& ExpressionElement::parsedExpression() const {
  if (parsed_expression_.get() == 0) {
    const char* location = repr.c_str();
    parsed_expression_ = get_assignment(location);
  }

  return *parsed_expression_;
}

// -----------------------------------------------------------------------

void ExpressionElement::runOnMachine(RLMachine& machine) const {
  machine.executeExpression(*this);
}

// -----------------------------------------------------------------------
// CommandElement
// -----------------------------------------------------------------------

CommandElement::CommandElement(const char* src) {
  memcpy(command, src, 8);
}

// -----------------------------------------------------------------------

CommandElement::~CommandElement() {}

// -----------------------------------------------------------------------

const ElementType CommandElement::type() const { return Command; }

void CommandElement::print(std::ostream& oss) const {
  oss << "op<" << modtype()
      << ":" << setw(3) << setfill('0') << module()
      << ":" << setw(5) << setfill('0') << opcode()
      << ", " << overload() << ">";

  PrintParameterString(oss, getUnparsedParameters());

  oss << endl;
}

// -----------------------------------------------------------------------

vector<string> CommandElement::getUnparsedParameters() const {
  vector<string> parameters;
  size_t numberOfParameters = param_count();
  for (size_t i = 0; i < numberOfParameters; ++i)
    parameters.push_back(get_param(i));
  return parameters;
}

// -----------------------------------------------------------------------

bool CommandElement::areParametersParsed() const {
  return param_count() == parsed_parameters_.size();
}

// -----------------------------------------------------------------------

void CommandElement::setParsedParameters(
  ExpressionPiecesVector& parsedParameters) const {
  parsed_parameters_.clear();
  parsed_parameters_ = std::move(parsedParameters);
}

// -----------------------------------------------------------------------

const ExpressionPiecesVector& CommandElement::getParameters() const {
  return parsed_parameters_;
}

// -----------------------------------------------------------------------

void CommandElement::runOnMachine(RLMachine& machine) const {
  machine.executeCommand(*this);
}

// -----------------------------------------------------------------------
// SelectElement
// -----------------------------------------------------------------------

SelectElement::SelectElement(const char* src)
  : CommandElement(src), uselessjunk(0) {
  repr.assign(src, 8);

  src += 8;
  if (*src == '(') {
    const int elen = next_expr(src);
    repr.append(src, elen);
    src += elen;
  }
  if (*src++ != '{') throw Error("SelectElement(): expected `{'");
  if (*src == '\n') { firstline = read_i16(src + 1); src += 3; } else firstline = 0;
  for (int i = 0; i < argc(); ++i) {
    // Skip preliminary metadata.
    while (*src == ',') ++src;
    // Read condition, if present.
    const char* cond = src;
    std::vector<Condition> cond_parsed;
    if (*src == '(') {
      ++src;
      while (*src != ')') {
        Condition c;
        if (*src == '(') {
          int len = next_expr(src);
          c.condition = string(src, len);
          src += len;
        }
        bool seekarg = *src != '2' && *src != '3';
        c.effect = *src;
        ++src;
        if (seekarg && *src != ')' && (*src < '0' || *src > '9')) {
          int len = next_expr(src);
          c.effect_argument = string(src, len);
          src += len;
        }
        cond_parsed.push_back(c);
      }
      if (*src++ != ')') throw Error("SelectElement(): expected `)'");
    }
    size_t clen = src - cond;
    // Read text.
    const char* text = src;
    src += next_string(src);
    size_t tlen = src - text;
    // Add parameter to list.
    if (*src != '\n') throw Error("SelectElement(): expected `\\n'");
    int lnum = read_i16(src + 1);
    src += 3;
    params.push_back(Param(cond_parsed, cond, clen, text, tlen, lnum));
  }

  // HACK?: In Kotomi's path in CLANNAD, there's a select with empty options
  // outside the count specified by argc().
  //
  // There are comments inside of disassembler.ml that seem to indicate that
  // NULL arguments are allowed. I am not sure if this is a hack or if this is
  // the proper behaviour. Also, why the hell would the official RealLive
  // compiler generate this bytecode. WTF?
  while (*src == '\n') {
    // The only thing allowed other than a 16 bit integer.
    src += 3;
    uselessjunk++;
  }

  if (*src++ != '}') throw Error("SelectElement(): expected `}'");
}

// -----------------------------------------------------------------------

const ElementType SelectElement::type() const {
  return Select;
}

// -----------------------------------------------------------------------

ExpressionElement SelectElement::window() const {
  return repr[8] == '('
    ? ExpressionElement(repr.data() + 9)
    : ExpressionElement(-1);
}

// -----------------------------------------------------------------------

const string
SelectElement::text(const int index) const {
  string rv;
  bool quoted = false;
  const string& s = params.at(index).text;
  const char* it = s.data(), *eit = s.data() + s.size();
  while (it < eit) {
    if (*it == '"') {
      ++it;
      quoted = !quoted;
    } else if (!quoted && strcmp((char*)it, "###PRINT(") == 0) {
      it += 9;
      rv += "\\s{";
      int ne = next_expr(it);
      rv.append(it, ne);
      it += ne + 1;
      rv += '}';
    } else if (*it == '\\') {
      rv += "\\\\";
    } else {
      if ((*it >= 0x81 && *it <= 0x9f) || (*it >= 0xe0 && *it <= 0xef))
        rv.push_back(*it++);
      rv.push_back(*it++);
    }
  }
  return rv;
}

// -----------------------------------------------------------------------

const size_t
SelectElement::length() const {
  size_t rv = repr.size() + 5;
  for (Param const& param : params)
    rv += param.cond_text.size() + param.text.size() + 3;
  rv += (uselessjunk * 3);
  return rv;
}

// -----------------------------------------------------------------------

const size_t SelectElement::param_count() const {
  return params.size();
}

// -----------------------------------------------------------------------

string SelectElement::get_param(int i) const {
  string rv(params[i].cond_text);
  rv.append(params[i].text);
  return rv;
}

// -----------------------------------------------------------------------
// FunctionElement
// -----------------------------------------------------------------------

FunctionElement::FunctionElement(const char* src,
                                 const std::vector<string>& params)
    : CommandElement(src),
      params(params) {
}

// -----------------------------------------------------------------------

const ElementType FunctionElement::type() const { return Function; }

// -----------------------------------------------------------------------

const size_t
FunctionElement::length() const {
  if (params.size() > 0) {
    size_t rv(COMMAND_SIZE + 2);
    for (std::string const& param : params)
      rv += param.size();
    return rv;
  } else {
    return COMMAND_SIZE;
  }
}

// -----------------------------------------------------------------------

std::string FunctionElement::serializableData(RLMachine& machine) const {
  string rv;
  for (int i = 0; i < COMMAND_SIZE; ++i)
    rv.push_back(command[i]);
  if (params.size() > 0) {
    rv.push_back('(');
    for (string const& param : params) {
      const char* data = param.c_str();
      std::unique_ptr<ExpressionPiece> expression(get_data(data));
      rv.append(expression->serializedValue(machine));
    }
    rv.push_back(')');
  }
  return rv;
}

// -----------------------------------------------------------------------

const size_t FunctionElement::param_count() const {
  // Because line number metaelements can be placed inside parameters (!?!?!),
  // it's possible that our last parameter consists only of the data for a
  // source line MetaElement. We can't detect this during parsing (because just
  // dropping the parameter will put the stream cursor in the wrong place), so
  // hack this here.
  if (!params.empty()) {
    string final = params.back();
    if (final.size() == 3 && final[0] == '\n')
      return params.size() - 1;
  }
  return params.size();
}

string FunctionElement::get_param(int i) const { return params[i]; }

// -----------------------------------------------------------------------
// VoidFunctionElement
// -----------------------------------------------------------------------

VoidFunctionElement::VoidFunctionElement(const char* src)
    : CommandElement(src) {
}

// -----------------------------------------------------------------------

const ElementType VoidFunctionElement::type() const { return Function; }

// -----------------------------------------------------------------------

const size_t VoidFunctionElement::length() const { return COMMAND_SIZE; }

// -----------------------------------------------------------------------

std::string VoidFunctionElement::serializableData(RLMachine& machine) const {
  string rv;
  for (int i = 0; i < COMMAND_SIZE; ++i)
    rv.push_back(command[i]);
  return rv;
}

// -----------------------------------------------------------------------

const size_t VoidFunctionElement::param_count() const { return 0; }
string VoidFunctionElement::get_param(int i) const { return std::string(); }

// -----------------------------------------------------------------------
// SingleArgFunctionElement
// -----------------------------------------------------------------------

SingleArgFunctionElement::SingleArgFunctionElement(const char* src,
                                                   const std::string& arg)
    : CommandElement(src),
      arg_(arg) {
}

// -----------------------------------------------------------------------

const ElementType SingleArgFunctionElement::type() const { return Function; }

// -----------------------------------------------------------------------

const size_t SingleArgFunctionElement::length() const {
  return COMMAND_SIZE + 2 + arg_.size();
}

// -----------------------------------------------------------------------

std::string SingleArgFunctionElement::serializableData(
    RLMachine& machine) const {
  string rv;
  for (int i = 0; i < COMMAND_SIZE; ++i)
    rv.push_back(command[i]);
  rv.push_back('(');
  const char* data = arg_.c_str();
  std::unique_ptr<ExpressionPiece> expression(get_data(data));
  rv.append(expression->serializedValue(machine));
  rv.push_back(')');
  return rv;
}

// -----------------------------------------------------------------------

const size_t SingleArgFunctionElement::param_count() const { return 1; }
string SingleArgFunctionElement::get_param(int i) const {
  return i == 0 ? arg_ : std::string();
}

// -----------------------------------------------------------------------
// PointerElement
// -----------------------------------------------------------------------

PointerElement::PointerElement(const char* src)
    : CommandElement(src) {
}

// -----------------------------------------------------------------------

PointerElement::~PointerElement() {}

// -----------------------------------------------------------------------

void PointerElement::set_pointers(ConstructionData& cdata) {
  targets.set_pointers(cdata);
}

const size_t PointerElement::pointers_count() const {
  return targets.size();
}

pointer_t PointerElement::get_pointer(int i) const {
  return targets[i];
}

// -----------------------------------------------------------------------
// GotoElement
// -----------------------------------------------------------------------

GotoElement::GotoElement(const char* src, ConstructionData& cdata)
    : CommandElement(src) {
  src += 8;

  id_ = read_i32(src);
}

// -----------------------------------------------------------------------

const ElementType GotoElement::type() const { return Goto; }

// -----------------------------------------------------------------------

const size_t GotoElement::param_count() const {
  return 0;
}

string GotoElement::get_param(int i) const {
  return std::string();
}

const size_t GotoElement::length() const {
  return 12;
}

void GotoElement::set_pointers(ConstructionData& cdata) {
  ConstructionData::offsets_t::const_iterator it =
      cdata.offsets.find(id_);
  assert(it != cdata.offsets.end());
  pointer_ = it->second;
}

const size_t GotoElement::pointers_count() const {
  return 1;
}

pointer_t GotoElement::get_pointer(int i) const {
  assert(i == 0);
  return pointer_;
}

// -----------------------------------------------------------------------
// GotoIfElement
// -----------------------------------------------------------------------

GotoIfElement::GotoIfElement(const char* src, ConstructionData& cdata)
    : CommandElement(src) {
  repr.assign(src, 8);
  src += 8;

  if (*src++ != '(') throw Error("GotoIfElement(): expected `('");
  int expr = next_expr(src);
  repr.push_back('(');
  repr.append(src, expr);
  repr.push_back(')');
  src += expr;
  if (*src++ != ')') throw Error("GotoIfElement(): expected `)'");

  id_ = read_i32(src);
}

// -----------------------------------------------------------------------

const ElementType GotoIfElement::type() const { return Goto; }

// -----------------------------------------------------------------------

const size_t GotoIfElement::param_count() const {
  return repr.size() == 8 ? 0 : 1;
}

string GotoIfElement::get_param(int i) const {
  return i == 0 ? (repr.size() == 8 ? string() : repr.substr(9, repr.size() - 10)) : string();
}

const size_t GotoIfElement::length() const {
  return repr.size() + 4;
}

void GotoIfElement::set_pointers(ConstructionData& cdata) {
  ConstructionData::offsets_t::const_iterator it =
      cdata.offsets.find(id_);
  assert(it != cdata.offsets.end());
  pointer_ = it->second;
}

const size_t GotoIfElement::pointers_count() const {
  return 1;
}

pointer_t GotoIfElement::get_pointer(int i) const {
  assert(i == 0);
  return pointer_;
}

// -----------------------------------------------------------------------
// GotoCaseElement
// -----------------------------------------------------------------------

GotoCaseElement::GotoCaseElement(const char* src, ConstructionData& cdata)
    : PointerElement(src) {
  repr.assign(src, 8);
  src += 8;
  // Condition
  const int expr = next_expr(src);
  repr.append(src, expr);
  src += expr;
  // Cases
  if (*src++ != '{') throw Error("GotoCaseElement(): expected `{'");
  int i = argc();
  cases.reserve(i);
  targets.reserve(i);
  while (i--) {
    if (src[0] != '(') throw Error("GotoCaseElement(): expected `('");
    if (src[1] == ')') {
      cases.push_back("()");
      src += 2;
    } else {
      int cexpr = next_expr(src + 1);
      cases.push_back(string(src, cexpr + 2));
      src += cexpr + 1;
      if (*src++ != ')') throw Error("GotoCaseElement(): expected `)'");
    }
    targets.push_id(read_i32(src));
    src += 4;
  }
  if (*src != '}') throw Error("GotoCaseElement(): expected `}'");
}

// -----------------------------------------------------------------------

const ElementType GotoCaseElement::type() const { return GotoCase; }

// -----------------------------------------------------------------------

const size_t
GotoCaseElement::length() const {
  size_t rv = repr.size() + 2;
  for (unsigned int i = 0; i < cases.size(); ++i) rv += cases[i].size() + 4;
  return rv;
}

const size_t GotoCaseElement::param_count() const {
  return 1;
}

string GotoCaseElement::get_param(int i) const {
  return i == 0 ? repr.substr(8, repr.size() - 8) : string();
}

// -----------------------------------------------------------------------
// GotoOnElement
// -----------------------------------------------------------------------

GotoOnElement::GotoOnElement(const char* src, ConstructionData& cdata)
    : PointerElement(src) {
  repr.assign(src, 8);
  src += 8;
  // Condition
  const int expr = next_expr(src);
  repr.append(src, expr);
  src += expr;
  // Pointers
  if (*src++ != '{') throw Error("GotoOnElement(): expected `{'");
  int i = argc();
  targets.reserve(i);
  while (i--) {
    targets.push_id(read_i32(src));
    src += 4;
  }
  if (*src != '}') throw Error("GotoOnElement(): expected `}'");
}

// -----------------------------------------------------------------------

const ElementType GotoOnElement::type() const { return GotoOn; }

// -----------------------------------------------------------------------

const size_t GotoOnElement::length() const {
  return repr.size() + argc() * 4 + 2;
}

const size_t GotoOnElement::param_count() const {
  return 1;
}

string GotoOnElement::get_param(int i) const {
  return i == 0 ? repr.substr(8, repr.size() - 8) : string();
}


// -----------------------------------------------------------------------

void
Pointers::set_pointers(ConstructionData& cdata) {
  assert(target_ids.size() != 0);
  targets.reserve(target_ids.size());
  for (unsigned int i = 0; i < target_ids.size(); ++i) {
    ConstructionData::offsets_t::const_iterator it =
      cdata.offsets.find(target_ids[i]);
    assert(it != cdata.offsets.end());
    targets.push_back(it->second);
  }
  target_ids.clear();
}

// -----------------------------------------------------------------------
// GosubWithElement
// -----------------------------------------------------------------------

GosubWithElement::GosubWithElement(const char* src, ConstructionData& cdata)
    : CommandElement(src),
      repr_size(8) {
  src += 8;
  if (*src == '(') {
    src++;
    repr_size++;

    while (*src != ')') {
      int expr = next_data(src);
      repr_size += expr;
      params.push_back(string(src, expr));
      src += expr;
    }
    src++;

    repr_size++;
  }

  id_ = read_i32(src);
}

// -----------------------------------------------------------------------

const ElementType GosubWithElement::type() const { return Goto; }

// -----------------------------------------------------------------------

const size_t GosubWithElement::length() const {
  return repr_size + 4;
}

const size_t GosubWithElement::param_count() const {
  return params.size();
}

string GosubWithElement::get_param(int i) const {
  return params[i];
}

void GosubWithElement::set_pointers(ConstructionData& cdata) {
  ConstructionData::offsets_t::const_iterator it =
      cdata.offsets.find(id_);
  assert(it != cdata.offsets.end());
  pointer_ = it->second;
}

const size_t GosubWithElement::pointers_count() const {
  return 1;
}

pointer_t GosubWithElement::get_pointer(int i) const {
  assert(i == 0);
  return pointer_;
}

}
