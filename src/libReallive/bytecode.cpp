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

#include <exception>
#include <utility>
#include <iostream>
#include <sstream>
#include <boost/scoped_ptr.hpp>

#include "bytecode.h"
#include "scenario.h"
#include "expression.h"

#include "MachineBase/RLMachine.hpp"

using namespace std;

namespace libReallive {

char BytecodeElement::entrypoint_marker = '@';
long BytecodeElement::id_src = 0;

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

BytecodeElement::BytecodeElement(const BytecodeElement& c)
  : id(id_src++) {}

// -----------------------------------------------------------------------

const ElementType BytecodeElement::type() const {
  return Unspecified;
}

// -----------------------------------------------------------------------

const size_t BytecodeElement::offset() const { return offset_; }

// -----------------------------------------------------------------------

const string BytecodeElement::data() const { return string(); }

// -----------------------------------------------------------------------

const size_t BytecodeElement::length() const { return 0; }

// -----------------------------------------------------------------------

string BytecodeElement::serializableData(RLMachine& machine) const {
  throw Error(
      "Can't call serializableData() on things other than FunctionElements");
}

// -----------------------------------------------------------------------

Pointers* BytecodeElement::get_pointers() { return NULL; }

// -----------------------------------------------------------------------

void BytecodeElement::set_pointers(ConstructionData& cdata) {}

// -----------------------------------------------------------------------

BytecodeElement* BytecodeElement::clone() const {  return new BytecodeElement(*this); }

// -----------------------------------------------------------------------

BytecodeElement::~BytecodeElement() {}

// -----------------------------------------------------------------------

BytecodeElement::BytecodeElement()
  : id(id_src++) {}

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
  case 0x00010001: case 0x00010002: case 0x00010006: case 0x00010007:
  case 0x00050002: case 0x00050006: case 0x00050007:
    return new GotoElement(stream, cdata);
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

  return new FunctionElement(stream);
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
  case ',':  return new DataElement(stream, 1);
  case '\n': return new MetaElement(0, stream);
  case '@':  // fall through
  case '!':  return new MetaElement(&cdata, stream);
  case '$':  return new ExpressionElement(stream);
  case '#':  return read_function(stream, cdata);
  default:   return new TextoutElement(stream, end);
  }
}

// -----------------------------------------------------------------------
// DataElement
// -----------------------------------------------------------------------

DataElement::DataElement() {}
DataElement::DataElement(const char* src, const size_t count)
  : repr(src, count) {}

// -----------------------------------------------------------------------

DataElement::~DataElement() {}

// -----------------------------------------------------------------------

const ElementType DataElement::type() const { return Data; }
const string DataElement::data() const { return repr; }
const size_t DataElement::length() const { return repr.size(); }
DataElement* DataElement::clone() const { return new DataElement(*this); }

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

// -----------------------------------------------------------------------

const string MetaElement::data() const {
  string rv(3, 0);
  rv[0] = type_ == Entrypoint_ ? entrypoint_marker : type_;
  insert_i16(rv, 1, value_);
  return rv;
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

MetaElement* MetaElement::clone() const { return new MetaElement(*this); }

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

void
TextoutElement::set_text(const char* src) {
  bool quoted = false;
  repr.clear();
  if (src) while (*src) {
      if (!quoted && (*src == '"' || *src == '#' || *src == '$' ||
                      *src == '\n' || *src == '@' ||
                      *src == ',' || *src == entrypoint_marker)) {
        quoted = true;
        repr.push_back('"');
      }
      if (*src == '"') repr.push_back('\\');
      if ((*src >= 0x81 && *src <= 0x9f) || (*src >= 0xe0 && *src <= 0xef))
        repr.push_back(*src++);
      repr.push_back(*src++);
	}
  if (quoted) repr.push_back('"');
}

// -----------------------------------------------------------------------

void TextoutElement::runOnMachine(RLMachine& machine) const {
  machine.performTextout(*this);
  machine.advanceInstructionPointer();
}

// -----------------------------------------------------------------------

TextoutElement* TextoutElement::clone() const { return new TextoutElement(*this); }

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

// -----------------------------------------------------------------------

ExpressionElement::ExpressionElement(const ExpressionElement& rhs)
  : DataElement(rhs), parsed_expression_(NULL) {
}

// -----------------------------------------------------------------------

ExpressionElement* ExpressionElement::clone() const {
  return new ExpressionElement(*this);
}

// -----------------------------------------------------------------------

const ExpressionPiece& ExpressionElement::parsedExpression() const {
  if (parsed_expression_.get() == 0) {
    const char* location = repr.c_str();
    parsed_expression_.reset(get_assignment(location));
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

CommandElement::CommandElement(
  const int type, const int module, const int opcode, const int argc,
  const int overload) {
  repr.resize(8, 0);
  repr[0] = '#';
  repr[1] = type;
  repr[2] = module;
  insert_i16(repr, 3, opcode);
  insert_i16(repr, 5, argc);
  repr[7] = overload;
}

// -----------------------------------------------------------------------

CommandElement::CommandElement(const char* src) {
  repr.assign(src, 8);
}

// -----------------------------------------------------------------------

CommandElement::CommandElement(const CommandElement& ce)
  : parsed_parameters_() {
}

// -----------------------------------------------------------------------

CommandElement::~CommandElement() {}

// -----------------------------------------------------------------------

const ElementType CommandElement::type() const { return Command; }

// -----------------------------------------------------------------------

const vector<string>& CommandElement::getUnparsedParameters() const {
  size_t numberOfParameters = param_count();
  if (numberOfParameters != unparsed_parameters_.size()) {
    unparsed_parameters_.clear();

    for (size_t i = 0; i < numberOfParameters; ++i)
      unparsed_parameters_.push_back(get_param(i));
  }

  return unparsed_parameters_;
}

// -----------------------------------------------------------------------

bool CommandElement::areParametersParsed() const {
  return param_count() == parsed_parameters_.size();
}

// -----------------------------------------------------------------------

/// This function shows...some deeper truth about mutability and
/// const-ness in C++, but I for one can't figure it out.
void CommandElement::setParsedParameters(
  boost::ptr_vector<libReallive::ExpressionPiece>& parsedParameters) const {
  parsed_parameters_.clear();
  parsed_parameters_.transfer( parsed_parameters_.end(),
                               parsedParameters.begin(),
                               parsedParameters.end(),
                               parsedParameters);
}

// -----------------------------------------------------------------------

const boost::ptr_vector<libReallive::ExpressionPiece>&
CommandElement::getParameters() const {
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

ExpressionElement
SelectElement::window() {
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

const string
SelectElement::data() const {
  string rv(repr);
  rv.push_back('{');
  rv.push_back('\n');
  append_i16(rv, firstline);
  for (params_t::const_iterator it = params.begin(); it != params.end(); ++it) {
    rv += it->cond_text;
    rv += it->text;
    rv.push_back('\n');
    rv.push_back(it->line & 0xff);
    rv.push_back((it->line >> 8) & 0xff);
  }
  rv.push_back('}');
  return rv;
}

// -----------------------------------------------------------------------

const size_t
SelectElement::length() const {
  size_t rv = repr.size() + 5;
  for (params_t::const_iterator it = params.begin(); it != params.end(); ++it)
    rv += it->cond_text.size() + it->text.size() + 3;
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

SelectElement* SelectElement::clone() const { return new SelectElement(*this); }

// -----------------------------------------------------------------------
// FunctionElement
// -----------------------------------------------------------------------

FunctionElement::FunctionElement(const char* src) : CommandElement(src) {
  src += 8;
  if (*src == '(') {
    const char* end = src + 1;
    while (*end != ')') {
      const size_t len = next_data(end);
      params.push_back(string(end, len));
      end += len;
    }
  }
}

// -----------------------------------------------------------------------

const ElementType FunctionElement::type() const { return Function; }

// -----------------------------------------------------------------------

const string
FunctionElement::data() const {
  string rv(repr);
  if (params.size() > 0) {
    rv.push_back('(');
    for (std::vector<string>::const_iterator it = params.begin(); it != params.end(); ++it) rv.append(*it);
    rv.push_back(')');
  }
  return rv;
}

// -----------------------------------------------------------------------

const size_t
FunctionElement::length() const {
  if (params.size() > 0) {
    size_t rv(repr.size() + 2);
    for (std::vector<string>::const_iterator it = params.begin(); it != params.end(); ++it) rv += it->size();
    return rv;
  } else {
    return repr.size();
  }
}

// -----------------------------------------------------------------------

std::string FunctionElement::serializableData(RLMachine& machine) const {
  string rv(repr);
  if (params.size() > 0) {
    rv.push_back('(');
    for (std::vector<string>::const_iterator it = params.begin();
         it != params.end(); ++it) {
      const char* data = it->c_str();
      boost::scoped_ptr<ExpressionPiece> expression(get_data(data));
      rv.append(expression->serializedValue(machine));
    }
    rv.push_back(')');
  }
  return rv;

}

// -----------------------------------------------------------------------

const size_t FunctionElement::param_count() const { return params.size(); }
string FunctionElement::get_param(int i) const { return params[i]; }

// -----------------------------------------------------------------------

FunctionElement* FunctionElement::clone() const { return new FunctionElement(*this); }

// -----------------------------------------------------------------------
// PointerElement
// -----------------------------------------------------------------------

PointerElement::PointerElement(const char* src) : CommandElement(src) {}

// -----------------------------------------------------------------------

PointerElement::~PointerElement() {}

// -----------------------------------------------------------------------

void PointerElement::set_pointers(ConstructionData& cdata) {
  targets.set_pointers(cdata);
}

// -----------------------------------------------------------------------

Pointers* PointerElement::get_pointers() { return &targets; }

// -----------------------------------------------------------------------

const Pointers& PointerElement::get_pointersRef() const { return targets; }

// -----------------------------------------------------------------------
// GotoElement
// -----------------------------------------------------------------------

GotoElement::GotoElement(const char* src, ConstructionData& cdata)
  : PointerElement(src) {
  src += 8;
  const int op = (module() * 100000) | opcode();
  if (op != 100000 && op != 100005 && op != 500001 && op != 500005)  {
    if (*src++ != '(') throw Error("GotoElement(): expected `('");
    int expr = next_expr(src);
    repr.push_back('(');
    repr.append(src, expr);
//        params.push_back(string(src, expr));
    repr.push_back(')');
    src += expr;
    if (*src++ != ')') throw Error("GotoElement(): expected `)'");
  }
  targets.push_id(read_i32(src));
}

// -----------------------------------------------------------------------

const ElementType GotoElement::type() const { return Goto; }

// -----------------------------------------------------------------------

GotoElement* GotoElement::clone() const { return new GotoElement(*this); }

// -----------------------------------------------------------------------

const string
GotoElement::data() const {
  string rv(repr);
  append_i32(rv, targets[0]->offset());
  return rv;
}

// -----------------------------------------------------------------------

const GotoElement::Case
GotoElement::taken() const {
  const int op = (module() * 100000) | opcode();
  if (op == 100000 || op == 500001) return Unconditional;
  const string arg(get_param(0));
  // For now, just look for straightforward integer-integer comparisons.
  if (arg.size() == 14 && arg[0] == '$' && arg[1] == 0xff && arg[6] == '\\' && arg[8] == '$' && arg[9] == 0xff) {
    const long i1 = read_i32(arg, 2);
    const long i2 = read_i32(arg, 10);
    bool result;
    switch (arg[7]) {
    case 0x28: result = i1 == i2; break;
    case 0x29: result = i1 != i2; break;
    case 0x2a: result = i1 <= i2; break;
    case 0x2b: result = i1 <  i2; break;
    case 0x2c: result = i1 >= i2; break;
    case 0x2d: result = i1 >  i2; break;
    default:
      return Variable;
    }
    return result == (opcode() % 5 == 1) ? Always : Never;
  }
  return Variable;
}

// -----------------------------------------------------------------------

void
GotoElement::make_unconditional() {
  if (opcode() < 5) set_opcode(module() == 1 ? 0 : 1); else set_opcode(5);
  repr.resize(8);
}

// -----------------------------------------------------------------------
// GotoCaseElement
// -----------------------------------------------------------------------

GotoCaseElement::GotoCaseElement(const char* src, ConstructionData& cdata)
  : PointerElement(src) {
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

GotoCaseElement* GotoCaseElement::clone() const { return new GotoCaseElement(*this); }

// -----------------------------------------------------------------------

const string
GotoCaseElement::data() const {
  string rv(repr);
  rv += '{';
  for (unsigned int i = 0; i < cases.size(); ++i) {
    rv += cases[i];
    append_i32(rv, targets[i]->offset());
  }
  rv += '}';
  return rv;
}

// -----------------------------------------------------------------------

const size_t
GotoCaseElement::length() const {
  size_t rv = repr.size() + 2;
  for (unsigned int i = 0; i < cases.size(); ++i) rv += cases[i].size() + 4;
  return rv;
}

// -----------------------------------------------------------------------
// GotoOnElement
// -----------------------------------------------------------------------

GotoOnElement::GotoOnElement(const char* src, ConstructionData& cdata)
  : PointerElement(src) {
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

GotoOnElement* GotoOnElement::clone() const { return new GotoOnElement(*this); }

// -----------------------------------------------------------------------

const size_t GotoOnElement::length() const {
  return repr.size() + argc() * 4 + 2;
}

// -----------------------------------------------------------------------

const string
GotoOnElement::data() const {
  string rv(repr);
  rv += '{';
  for (unsigned int i = 0; i < targets.size(); ++i) {
    append_i32(rv, targets[i]->offset());
  }
  rv += '}';
  return rv;
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
  : PointerElement(src) {
  src += 8;
  if (*src == '(') {
    src++;
    repr.push_back('(');

    while (*src != ')') {
      int expr = next_data(src);
      repr.append(src, expr);
      params.push_back(string(src, expr));
      src += expr;
    }
    src++;

    repr.push_back(')');
  }

  targets.push_id(read_i32(src));
}

// -----------------------------------------------------------------------

const ElementType GosubWithElement::type() const { return Goto; }

// -----------------------------------------------------------------------

GosubWithElement* GosubWithElement::clone() const { return new GosubWithElement(*this); }

// -----------------------------------------------------------------------

const string
GosubWithElement::data() const {
  string rv(repr);
  append_i32(rv, targets[0]->offset());
  return rv;
}

// -----------------------------------------------------------------------

const size_t GosubWithElement::length() const { return repr.size() + 4; }

}
