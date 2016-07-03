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

#include "libreallive/bytecode.h"

#include <cassert>
#include <cstring>
#include <exception>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "libreallive/scenario.h"
#include "libreallive/expression.h"

#include "machine/rlmachine.h"

namespace libreallive {

namespace {

inline BytecodeElement* ReadFunction(const char* stream,
                                     ConstructionData& cdata) {
  // opcode: 0xttmmoooo (Type, Module, Opcode: e.g. 0x01030101 = 1:03:00257
  const unsigned long opcode =
      (stream[1] << 24) | (stream[2] << 16) | (stream[4] << 8) | stream[3];
  switch (opcode) {
    case 0x00010000:
    case 0x00010005:
    case 0x00050001:
    case 0x00050005:
    case 0x00060001:
    case 0x00060005:
      return new GotoElement(stream, cdata);
    case 0x00010001:
    case 0x00010002:
    case 0x00010006:
    case 0x00010007:
    case 0x00050002:
    case 0x00050006:
    case 0x00050007:
    case 0x00060000:
    case 0x00060002:
    case 0x00060006:
    case 0x00060007:
      return new GotoIfElement(stream, cdata);
    case 0x00010003:
    case 0x00010008:
    case 0x00050003:
    case 0x00050008:
    case 0x00060003:
    case 0x00060008:
      return new GotoOnElement(stream, cdata);
    case 0x00010004:
    case 0x00010009:
    case 0x00050004:
    case 0x00050009:
    case 0x00060004:
    case 0x00060009:
      return new GotoCaseElement(stream, cdata);
    case 0x00010010:
    case 0x00060010:
      return new GosubWithElement(stream, cdata);

    // Select elements.
    case 0x00020000:
    case 0x00020001:
    case 0x00020002:
    case 0x00020003:
    case 0x00020010:
      return new SelectElement(stream);
  }

  return BuildFunctionElement(stream);
}

}  // namespace

char BytecodeElement::entrypoint_marker = '@';

CommandElement* BuildFunctionElement(const char* stream) {
  const char* ptr = stream;
  ptr += 8;
  std::vector<std::string> params;
  if (*ptr == '(') {
    const char* end = ptr + 1;
    while (*end != ')') {
      const size_t len = NextData(end);
      params.emplace_back(end, len);
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
      ExpressionPiece piece(GetData(start));
      oss << piece.GetDebugString();
    }
    catch (libreallive::Error& e) {
      // Any error throw here is a parse error.
      oss << "{RAW : " << ParsableToPrintableString(param) << "}";
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
// Pointers
// -----------------------------------------------------------------------

Pointers::Pointers() {}

Pointers::~Pointers() {}

void Pointers::SetPointers(ConstructionData& cdata) {
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
// BytecodeElement
// -----------------------------------------------------------------------

BytecodeElement::BytecodeElement() {}

BytecodeElement::~BytecodeElement() {}

void BytecodeElement::PrintSourceRepresentation(RLMachine* machine,
                                                std::ostream& oss) const {
  oss << "<unspecified bytecode>" << std::endl;
}

void BytecodeElement::SetPointers(ConstructionData& cdata) {}

const int BytecodeElement::GetEntrypoint() const { return kInvalidEntrypoint; }

string BytecodeElement::GetSerializedCommand(RLMachine& machine) const {
  throw Error(
      "Can't call GetSerializedCommand() on things other than "
      "FunctionElements");
}

void BytecodeElement::RunOnMachine(RLMachine& machine) const {
  machine.AdvanceInstructionPointer();
}

// static
BytecodeElement* BytecodeElement::Read(const char* stream,
                                       const char* end,
                                       ConstructionData& cdata) {
  const char c = *stream;
  if (c == '!')
    entrypoint_marker = '!';
  switch (c) {
    case 0:
    case ',':
      return new CommaElement;
    case '\n':
      return new MetaElement(0, stream);
    case '@':  // fall through
    case '!':
      return new MetaElement(&cdata, stream);
    case '$':
      return new ExpressionElement(stream);
    case '#':
      return ReadFunction(stream, cdata);
    default:
      return new TextoutElement(stream, end);
  }
}

BytecodeElement::BytecodeElement(const BytecodeElement& c) {}

// -----------------------------------------------------------------------
// CommaElement
// -----------------------------------------------------------------------

CommaElement::CommaElement() {}
CommaElement::~CommaElement() {}

void CommaElement::PrintSourceRepresentation(RLMachine* machine,
                                             std::ostream& oss) const {
  oss << "<CommaElement>" << std::endl;
}

const size_t CommaElement::GetBytecodeLength() const { return 1; }

// -----------------------------------------------------------------------
// MetaElement
// -----------------------------------------------------------------------

MetaElement::MetaElement(const ConstructionData* cv, const char* src) {
  value_ = read_i16(src + 1);
  if (!cv) {
    type_ = Line_;
  } else if (cv->kidoku_table.at(value_) >= 1000000) {
    type_ = Entrypoint_;
    entrypoint_index_ = cv->kidoku_table[value_] - 1000000;
  } else {
    type_ = Kidoku_;
  }
}

MetaElement::~MetaElement() {}

void MetaElement::PrintSourceRepresentation(RLMachine* machine,
                                            std::ostream& oss) const {
  if (type_ == Line_)
    oss << "#line " << value_ << std::endl;
  else if (type_ == Entrypoint_)
    oss << "#entrypoint " << value_ << std::endl;
  else
    oss << "{- Kidoku " << value_ << " -}" << std::endl;
}

const size_t MetaElement::GetBytecodeLength() const { return 3; }

const int MetaElement::GetEntrypoint() const {
  return type_ == Entrypoint_ ? entrypoint_index_ : kInvalidEntrypoint;
}

void MetaElement::RunOnMachine(RLMachine& machine) const {
  if (type_ == Line_)
    machine.SetLineNumber(value_);
  else if (type_ == Kidoku_)
    machine.SetKidokuMarker(value_);

  machine.AdvanceInstructionPointer();
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
      if (*end == '\\' && end[1] == '"')
        ++end;
    } else {
      if (*end == ',')
        ++end;
      quoted = *end == '"';
      if (!*end || *end == '#' || *end == '$' || *end == '\n' || *end == '@' ||
          *end == entrypoint_marker)
        break;
    }
    if ((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef))
      end += 2;
    else
      ++end;
  }
  repr.assign(src, end);
}

TextoutElement::~TextoutElement() {}

const string TextoutElement::GetText() const {
  string rv;
  bool quoted = false;
  string::const_iterator it = repr.cbegin();
  while (it != repr.cend()) {
    if (*it == '"') {
      ++it;
      quoted = !quoted;
    } else if (quoted && *it == '\\') {
      ++it;
      if (*it == '"') {
        ++it;
        rv.push_back('"');
      } else {
        rv.push_back('\\');
      }
    } else {
      if ((*it >= 0x81 && *it <= 0x9f) || (*it >= 0xe0 && *it <= 0xef))
        rv.push_back(*it++);
      rv.push_back(*it++);
    }
  }
  return rv;
}

void TextoutElement::PrintSourceRepresentation(RLMachine* machine,
                                               std::ostream& oss) const {
  oss << "\"" << GetText() << "\"" << std::endl;
}

const size_t TextoutElement::GetBytecodeLength() const { return repr.size(); }

void TextoutElement::RunOnMachine(RLMachine& machine) const {
  machine.PerformTextout(*this);
  machine.AdvanceInstructionPointer();
}

// -----------------------------------------------------------------------
// ExpressionElement
// -----------------------------------------------------------------------

ExpressionElement::ExpressionElement(const char* src)
    : parsed_expression_(invalid_expression_piece_t()) {
  const char* end = src;
  parsed_expression_ = GetAssignment(end);
  length_ = std::distance(src, end);
}

ExpressionElement::ExpressionElement(const long val)
    : length_(0),
      parsed_expression_(ExpressionPiece::IntConstant(val)) {
}

ExpressionElement::ExpressionElement(const ExpressionElement& rhs)
    : length_(0),
      parsed_expression_(rhs.parsed_expression_) {
}

ExpressionElement::~ExpressionElement() {}

const ExpressionPiece& ExpressionElement::ParsedExpression() const {
  return parsed_expression_;
}

void ExpressionElement::PrintSourceRepresentation(RLMachine* machine,
                                                  std::ostream& oss) const {
  oss << ParsedExpression().GetDebugString() << std::endl;
}

const size_t ExpressionElement::GetBytecodeLength() const {
  return length_;
}

void ExpressionElement::RunOnMachine(RLMachine& machine) const {
  machine.ExecuteExpression(*this);
}

// -----------------------------------------------------------------------
// CommandElement
// -----------------------------------------------------------------------

CommandElement::CommandElement(const char* src) { memcpy(command, src, 8); }

CommandElement::~CommandElement() {}

std::vector<std::string> CommandElement::GetUnparsedParameters() const {
  std::vector<std::string> parameters;
  size_t param_count = GetParamCount();
  for (size_t i = 0; i < param_count; ++i)
    parameters.push_back(GetParam(i));
  return parameters;
}

bool CommandElement::AreParametersParsed() const {
  return GetParamCount() == parsed_parameters_.size();
}

void CommandElement::SetParsedParameters(
    ExpressionPiecesVector parsedParameters) const {
  parsed_parameters_ = std::move(parsedParameters);
}

const ExpressionPiecesVector& CommandElement::GetParsedParameters() const {
  return parsed_parameters_;
}

const size_t CommandElement::GetPointersCount() const { return 0; }

pointer_t CommandElement::GetPointer(int i) const { return pointer_t(); }

const size_t CommandElement::GetCaseCount() const { return 0; }

const string CommandElement::GetCase(int i) const { return ""; }

void CommandElement::PrintSourceRepresentation(RLMachine* machine,
                                               std::ostream& oss) const {
  std::string name = machine->GetCommandName(*this);

  if (name != "") {
    oss << name;
  } else {
    oss << "op<" << modtype() << ":" << std::setw(3) << std::setfill('0')
        << module() << ":"
        << std::setw(5) << std::setfill('0') << opcode() << ", " << overload()
        << ">";
  }

  PrintParameterString(oss, GetUnparsedParameters());

  oss << std::endl;
}

void CommandElement::RunOnMachine(RLMachine& machine) const {
  machine.ExecuteCommand(*this);
}

// -----------------------------------------------------------------------
// SelectElement
// -----------------------------------------------------------------------

SelectElement::SelectElement(const char* src)
    : CommandElement(src), uselessjunk(0) {
  repr.assign(src, 8);

  src += 8;
  if (*src == '(') {
    const int elen = NextExpression(src);
    repr.append(src, elen);
    src += elen;
  }

  if (*src++ != '{')
    throw Error("SelectElement(): expected `{'");

  if (*src == '\n') {
    firstline = read_i16(src + 1);
    src += 3;
  } else {
    firstline = 0;
  }

  for (int i = 0; i < argc(); ++i) {
    // Skip preliminary metadata.
    while (*src == ',')
      ++src;
    // Read condition, if present.
    const char* cond = src;
    std::vector<Condition> cond_parsed;
    if (*src == '(') {
      ++src;
      while (*src != ')') {
        Condition c;
        if (*src == '(') {
          int len = NextExpression(src);
          c.condition = string(src, len);
          src += len;
        }
        bool seekarg = *src != '2' && *src != '3';
        c.effect = *src;
        ++src;
        if (seekarg && *src != ')' && (*src < '0' || *src > '9')) {
          int len = NextExpression(src);
          c.effect_argument = string(src, len);
          src += len;
        }
        cond_parsed.push_back(c);
      }
      if (*src++ != ')')
        throw Error("SelectElement(): expected `)'");
    }
    size_t clen = src - cond;
    // Read text.
    const char* text = src;
    src += NextString(src);
    size_t tlen = src - text;
    // Add parameter to list.
    if (*src != '\n')
      throw Error("SelectElement(): expected `\\n'");
    int lnum = read_i16(src + 1);
    src += 3;
    params.emplace_back(cond_parsed, cond, clen, text, tlen, lnum);
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

  if (*src++ != '}')
    throw Error("SelectElement(): expected `}'");
}

SelectElement::~SelectElement() {}

ExpressionPiece SelectElement::GetWindowExpression() const {
  if (repr[8] == '(') {
    const char* location = repr.c_str() + 9;
    return GetExpression(location);
  }
  return ExpressionPiece::IntConstant(-1);
}

const size_t SelectElement::GetParamCount() const { return params.size(); }

string SelectElement::GetParam(int i) const {
  string rv(params[i].cond_text);
  rv.append(params[i].text);
  return rv;
}

const size_t SelectElement::GetBytecodeLength() const {
  size_t rv = repr.size() + 5;
  for (Param const& param : params)
    rv += param.cond_text.size() + param.text.size() + 3;
  rv += (uselessjunk * 3);
  return rv;
}

// -----------------------------------------------------------------------
// FunctionElement
// -----------------------------------------------------------------------

FunctionElement::FunctionElement(const char* src,
                                 const std::vector<string>& params)
    : CommandElement(src), params(params) {}

FunctionElement::~FunctionElement() {}

const size_t FunctionElement::GetParamCount() const {
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

string FunctionElement::GetParam(int i) const { return params[i]; }

const size_t FunctionElement::GetBytecodeLength() const {
  if (params.size() > 0) {
    size_t rv(COMMAND_SIZE + 2);
    for (std::string const& param : params)
      rv += param.size();
    return rv;
  } else {
    return COMMAND_SIZE;
  }
}

std::string FunctionElement::GetSerializedCommand(RLMachine& machine) const {
  string rv;
  for (int i = 0; i < COMMAND_SIZE; ++i)
    rv.push_back(command[i]);
  if (params.size() > 0) {
    rv.push_back('(');
    for (string const& param : params) {
      const char* data = param.c_str();
      ExpressionPiece expression(GetData(data));
      rv.append(expression.GetSerializedExpression(machine));
    }
    rv.push_back(')');
  }
  return rv;
}

// -----------------------------------------------------------------------
// VoidFunctionElement
// -----------------------------------------------------------------------

VoidFunctionElement::VoidFunctionElement(const char* src)
    : CommandElement(src) {}

VoidFunctionElement::~VoidFunctionElement() {}

const size_t VoidFunctionElement::GetParamCount() const { return 0; }

string VoidFunctionElement::GetParam(int i) const { return std::string(); }

const size_t VoidFunctionElement::GetBytecodeLength() const {
  return COMMAND_SIZE;
}

std::string VoidFunctionElement::GetSerializedCommand(RLMachine& machine)
    const {
  string rv;
  for (int i = 0; i < COMMAND_SIZE; ++i)
    rv.push_back(command[i]);
  return rv;
}

// -----------------------------------------------------------------------
// SingleArgFunctionElement
// -----------------------------------------------------------------------

SingleArgFunctionElement::SingleArgFunctionElement(const char* src,
                                                   const std::string& arg)
    : CommandElement(src), arg_(arg) {}

SingleArgFunctionElement::~SingleArgFunctionElement() {}

const size_t SingleArgFunctionElement::GetParamCount() const { return 1; }

string SingleArgFunctionElement::GetParam(int i) const {
  return i == 0 ? arg_ : std::string();
}

const size_t SingleArgFunctionElement::GetBytecodeLength() const {
  return COMMAND_SIZE + 2 + arg_.size();
}

std::string SingleArgFunctionElement::GetSerializedCommand(RLMachine& machine)
    const {
  string rv;
  for (int i = 0; i < COMMAND_SIZE; ++i)
    rv.push_back(command[i]);
  rv.push_back('(');
  const char* data = arg_.c_str();
  ExpressionPiece expression(GetData(data));
  rv.append(expression.GetSerializedExpression(machine));
  rv.push_back(')');
  return rv;
}

// -----------------------------------------------------------------------
// PointerElement
// -----------------------------------------------------------------------

PointerElement::PointerElement(const char* src) : CommandElement(src) {}

PointerElement::~PointerElement() {}

const size_t PointerElement::GetPointersCount() const { return targets.size(); }

pointer_t PointerElement::GetPointer(int i) const { return targets[i]; }

void PointerElement::SetPointers(ConstructionData& cdata) {
  targets.SetPointers(cdata);
}

// -----------------------------------------------------------------------
// GotoElement
// -----------------------------------------------------------------------

GotoElement::GotoElement(const char* src, ConstructionData& cdata)
    : CommandElement(src) {
  src += 8;

  id_ = read_i32(src);
}

GotoElement::~GotoElement() {}

const size_t GotoElement::GetParamCount() const {
  // The pointer is not counted as a parameter.
  return 0;
}

string GotoElement::GetParam(int i) const { return std::string(); }

const size_t GotoElement::GetPointersCount() const { return 1; }

pointer_t GotoElement::GetPointer(int i) const {
  assert(i == 0);
  return pointer_;
}

const size_t GotoElement::GetBytecodeLength() const { return 12; }

void GotoElement::SetPointers(ConstructionData& cdata) {
  ConstructionData::offsets_t::const_iterator it = cdata.offsets.find(id_);
  assert(it != cdata.offsets.end());
  pointer_ = it->second;
}

// -----------------------------------------------------------------------
// GotoIfElement
// -----------------------------------------------------------------------

GotoIfElement::GotoIfElement(const char* src, ConstructionData& cdata)
    : CommandElement(src) {
  repr.assign(src, 8);
  src += 8;

  if (*src++ != '(')
    throw Error("GotoIfElement(): expected `('");
  int expr = NextExpression(src);
  repr.push_back('(');
  repr.append(src, expr);
  repr.push_back(')');
  src += expr;
  if (*src++ != ')')
    throw Error("GotoIfElement(): expected `)'");

  id_ = read_i32(src);
}

GotoIfElement::~GotoIfElement() {}

const size_t GotoIfElement::GetParamCount() const {
  // The pointer is not counted as a parameter.
  return repr.size() == 8 ? 0 : 1;
}

string GotoIfElement::GetParam(int i) const {
  return i == 0
             ? (repr.size() == 8 ? string() : repr.substr(9, repr.size() - 10))
             : string();
}

const size_t GotoIfElement::GetPointersCount() const { return 1; }

pointer_t GotoIfElement::GetPointer(int i) const {
  assert(i == 0);
  return pointer_;
}

const size_t GotoIfElement::GetBytecodeLength() const {
  return repr.size() + 4;
}

void GotoIfElement::SetPointers(ConstructionData& cdata) {
  ConstructionData::offsets_t::const_iterator it = cdata.offsets.find(id_);
  assert(it != cdata.offsets.end());
  pointer_ = it->second;
}

// -----------------------------------------------------------------------
// GotoCaseElement
// -----------------------------------------------------------------------

GotoCaseElement::GotoCaseElement(const char* src, ConstructionData& cdata)
    : PointerElement(src) {
  repr.assign(src, 8);
  src += 8;
  // Condition
  const int expr = NextExpression(src);
  repr.append(src, expr);
  src += expr;
  // Cases
  if (*src++ != '{')
    throw Error("GotoCaseElement(): expected `{'");
  int i = argc();
  cases.reserve(i);
  targets.reserve(i);
  while (i--) {
    if (src[0] != '(')
      throw Error("GotoCaseElement(): expected `('");
    if (src[1] == ')') {
      cases.push_back("()");
      src += 2;
    } else {
      int cexpr = NextExpression(src + 1);
      cases.emplace_back(src, cexpr + 2);
      src += cexpr + 1;
      if (*src++ != ')')
        throw Error("GotoCaseElement(): expected `)'");
    }
    targets.push_id(read_i32(src));
    src += 4;
  }
  if (*src != '}')
    throw Error("GotoCaseElement(): expected `}'");
}

GotoCaseElement::~GotoCaseElement() {}

const size_t GotoCaseElement::GetParamCount() const {
  // The cases are not counted as parameters.
  return 1;
}

string GotoCaseElement::GetParam(int i) const {
  return i == 0 ? repr.substr(8, repr.size() - 8) : string();
}

const size_t GotoCaseElement::GetCaseCount() const { return cases.size(); }

const string GotoCaseElement::GetCase(int i) const { return cases[i]; }

const size_t GotoCaseElement::GetBytecodeLength() const {
  size_t rv = repr.size() + 2;
  for (unsigned int i = 0; i < cases.size(); ++i)
    rv += cases[i].size() + 4;
  return rv;
}

// -----------------------------------------------------------------------
// GotoOnElement
// -----------------------------------------------------------------------

GotoOnElement::GotoOnElement(const char* src, ConstructionData& cdata)
    : PointerElement(src) {
  repr.assign(src, 8);
  src += 8;
  // Condition
  const int expr = NextExpression(src);
  repr.append(src, expr);
  src += expr;
  // Pointers
  if (*src++ != '{')
    throw Error("GotoOnElement(): expected `{'");
  int i = argc();
  targets.reserve(i);
  while (i--) {
    targets.push_id(read_i32(src));
    src += 4;
  }
  if (*src != '}')
    throw Error("GotoOnElement(): expected `}'");
}

GotoOnElement::~GotoOnElement() {}

const size_t GotoOnElement::GetParamCount() const { return 1; }

string GotoOnElement::GetParam(int i) const {
  return i == 0 ? repr.substr(8, repr.size() - 8) : string();
}

const size_t GotoOnElement::GetBytecodeLength() const {
  return repr.size() + argc() * 4 + 2;
}

// -----------------------------------------------------------------------
// GosubWithElement
// -----------------------------------------------------------------------

GosubWithElement::GosubWithElement(const char* src, ConstructionData& cdata)
    : CommandElement(src), repr_size(8) {
  src += 8;
  if (*src == '(') {
    src++;
    repr_size++;

    while (*src != ')') {
      int expr = NextData(src);
      repr_size += expr;
      params.emplace_back(src, expr);
      src += expr;
    }
    src++;

    repr_size++;
  }

  id_ = read_i32(src);
}

GosubWithElement::~GosubWithElement() {}

const size_t GosubWithElement::GetParamCount() const {
  // The pointer is not counted as a parameter.
  return params.size();
}

string GosubWithElement::GetParam(int i) const { return params[i]; }

const size_t GosubWithElement::GetPointersCount() const { return 1; }

pointer_t GosubWithElement::GetPointer(int i) const {
  assert(i == 0);
  return pointer_;
}

const size_t GosubWithElement::GetBytecodeLength() const {
  return repr_size + 4;
}

void GosubWithElement::SetPointers(ConstructionData& cdata) {
  ConstructionData::offsets_t::const_iterator it = cdata.offsets.find(id_);
  assert(it != cdata.offsets.end());
  pointer_ = it->second;
}

}  // namespace libreallive
