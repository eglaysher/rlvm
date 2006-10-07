/* Bytecode class structure. */

#include <exception>
#include <utility>
#include <iostream>

#include "bytecode.h"
#include "scenario.h"
#include "expression.h"

using namespace std;

namespace libReallive {

char BytecodeElement::entrypoint_marker = '@';
long BytecodeElement::id_src = 0;

ConstructionData::ConstructionData(size_t kt, pointer_t pt)
  : kidoku_table(kt), null(pt) {}

BytecodeElement::BytecodeElement(const BytecodeElement& c)
  : id(id_src++) {}

inline BytecodeElement*
read_function(const char* stream, ConstructionData& cdata)
{
	// opcode: 0xttmmoooo (Type, Module, Opcode: e.g. 0x01030101 = 1:03:00257
	const unsigned long opcode = (stream[1] << 24) | (stream[2] << 16) | (stream[4] << 8) | stream[3];
	switch (opcode) {
	case 0x00010000: case 0x00010005: 
	case 0x00050001: case 0x00050005:
	case 0x00010001: case 0x00010002: case 0x00010006: case 0x00010007:	case 0x00010010: 
	case 0x00050002: case 0x00050006: case 0x00050007:
		return new GotoElement(stream, cdata);
	case 0x00010003: case 0x00010008:
	case 0x00050003: case 0x00050008:
		return new GotoOnElement(stream, cdata);
	case 0x00010004: case 0x00010009:
	case 0x00050004: case 0x00050009:
		return new GotoCaseElement(stream, cdata);
	}
	if (stream[2] == 2)
		return new SelectElement(stream);
	else
		return new FunctionElement(stream);
}

BytecodeElement*
BytecodeElement::read(const char* stream, ConstructionData& cdata)
{
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
	default:   return new TextoutElement(stream);
	}
}

// DataElement implementation.

DataElement::DataElement() {}
DataElement::DataElement(const char* src, const size_t count)
  : repr(src, count) {}

// MetaElement implementation

MetaElement::MetaElement(const ConstructionData* cv, const char* src) {
  value_ = read_i16(src + 1);
  if (!cv) {
    type_ = Line_; 
  }
  else if (cv->kidoku_table.at(value_) >= 1000000) {
    type_ = Entrypoint_;
    entrypoint_index = cv->kidoku_table[value_] - 1000000;
  }
  else {
    type_ = Kidoku_;
  }
}

// TextoutElement implementation.

TextoutElement::TextoutElement(const char* src)
{
	const char* end = src;
	bool quoted = false;
	while (true) {
		if (quoted) {
			quoted = *end != '"';
			if (*end == '\\' && end[1] == '"') ++end;
		}
		else {
			if (*end == ',') ++end;
			quoted = *end == '"';
			if (!*end || *end == '#' || *end == '$' || *end == '\n' || *end == '@' || *end == entrypoint_marker)
				break;
		}
		if ((*end >= 0x81 && *end <= 0x9f) || (*end >= 0xe0 && *end <= 0xef)) 
			end += 2;
		else
			++end;
	}
	repr.assign(src, end);
}

TextoutElement::TextoutElement() {}

const string 
TextoutElement::text() const
{
	string rv;
	bool quoted = false;
	string::const_iterator it = repr.begin();
	while (it != repr.end()) {
		if (*it == '"') {
			++it;
			quoted = !quoted;
		}
		else if (quoted && *it == '\\') {
			++it;
			if (*it == '"') {
				++it;
				rv.push_back('"');
			}
			else rv.push_back('\\');
		}
		else {
			if ((*it >= 0x81 && *it <= 0x9f) || (*it >= 0xe0 && *it <= 0xef)) 
				rv.push_back(*it++);
			rv.push_back(*it++);
		}
	}
	return rv;
}

void 
TextoutElement::set_text(const char* src)
{
	bool quoted = false;
	repr.clear();
	if (src) while (*src) {
		if (!quoted && (*src == '"' || *src == '#' || *src == '$' || *src == '\n' || *src == '@' ||
			            *src == ',' || *src == entrypoint_marker)) {
			quoted = true;
			repr.push_back('"');
		}
		if (*src == '"') repr.push_back('\\');
		if ((*src >= 0x81 && *src <= 0x9f) || (*src >= 0xe0 && *src <= 0xef)) repr.push_back(*src++);
		repr.push_back(*src++);
	}
	if (quoted) repr.push_back('"');
}

// ExpressionElement implementation

ExpressionElement::ExpressionElement(const char* src)
{
	// Don't parse the expression, just isolate it.
	const char* end = src;
	end += next_token(end);
	if (*end == '\\') {
		end += 2;
		end += next_expr(end);
	}
	repr.assign(src, end);
}

ExpressionElement::ExpressionElement(const long val)
{
  repr.resize(6, '$');
  repr[1] = 0xff;
  insert_i32(repr, 2, val); 
}

ExpressionElement::ExpressionElement(const ExpressionElement& rhs)
  : DataElement(rhs), m_parsedExpression(NULL)
{ 
}

ExpressionElement* ExpressionElement::clone() const {
  return new ExpressionElement(*this); 
}

const ExpressionPiece& ExpressionElement::parsedExpression() const {
  if(m_parsedExpression.get() == 0) {
    const char* location = repr.c_str();
    m_parsedExpression.reset(get_assignment(location));
  } 
  
  return *m_parsedExpression;
}

// CommandElement implementations
CommandElement::CommandElement(const int type, const int module, const int opcode, const int argc, const int overload) { 
  repr.resize(8, 0);
  repr[0] = '#';
  repr[1] = type;
  repr[2] = module;
  insert_i16(repr, 3, opcode);
  insert_i16(repr, 5, argc);
  repr[7] = overload;
}

CommandElement::CommandElement(const char* src) {
  repr.assign(src, 8);
}

// SelectElement implementation
SelectElement::SelectElement(const char* src) : CommandElement(src)
{
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
		if (*src == '(') {
			++src;
			while (*src != ')') {
				if (*src == '(') src += next_expr(src);
				bool seekarg = *src != '2' && *src != '3';
				++src;
				if (seekarg && *src != ')' && (*src < '0' || *src > '9')) src += next_expr(src);
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
		params.push_back(Param(cond, clen, text, tlen, lnum));
	}
	if (*src++ != '}') throw Error("SelectElement(): expected `}'");
}

ExpressionElement
SelectElement::window()
{
	 return repr[8] == '(' 
	      ? ExpressionElement(repr.data() + 9) 
	      : ExpressionElement(-1);
}

const string
SelectElement::text(const int index) const
{
	string rv;
	bool quoted = false;
	const string& s = params.at(index).text;
	const char* it = s.data(), *eit = s.data() + s.size();
	while (it < eit) {
		if (*it == '"') {
			++it;
			quoted = !quoted;
		}
		else if (!quoted && strcmp((char*)it, "###PRINT(") == 0) {
			it += 9;
			rv += "\\s{";
			int ne = next_expr(it);
			rv.append(it, ne);
			it += ne + 1;
			rv += '}';
		}
		else if (*it == '\\') {
			rv += "\\\\";
		}
		else {
			if ((*it >= 0x81 && *it <= 0x9f) || (*it >= 0xe0 && *it <= 0xef)) 
				rv.push_back(*it++);
			rv.push_back(*it++);
		}
	}
	return rv;
}

const string
SelectElement::data() const
{
	string rv(repr);
	rv.push_back('{');
	rv.push_back('\n');
	append_i16(rv, firstline);
	for (params_t::const_iterator it = params.begin(); it != params.end(); ++it) {
		rv += it->cond;
		rv += it->text;
		rv.push_back('\n');
		rv.push_back(it->line & 0xff);
		rv.push_back((it->line >> 8) & 0xff);
	}
	rv.push_back('}');
	return rv;
}

const size_t 
SelectElement::length() const
{
	size_t rv = repr.size() + 5;
	for (params_t::const_iterator it = params.begin(); it != params.end(); ++it) 
		rv += it->cond.size() + it->text.size() + 3;
	return rv;
}

FunctionElement::FunctionElement(const char* src) : CommandElement(src) 
{
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

const string 
FunctionElement::data() const 
{
	string rv(repr);
	if (params.size() > 0) {
		rv.push_back('(');
		for (std::vector<string>::const_iterator it = params.begin(); it != params.end(); ++it) rv.append(*it);
		rv.push_back(')');
	}
	return rv;
}

const size_t 
FunctionElement::length() const
{
	if (params.size() > 0) {
		size_t rv(repr.size() + 2);	
		for (std::vector<string>::const_iterator it = params.begin(); it != params.end(); ++it) rv += it->size();
		return rv;
	}
	else {
		return repr.size();
	}
}

PointerElement::PointerElement(const char* src) : CommandElement(src) {}

GotoElement::GotoElement(const char* src, ConstructionData& cdata) : PointerElement(src)
{
	src += 8;
	const int op = (module() * 100000) | opcode();
	if (op != 100000 && op != 100005 && op != 500001 && op != 500005)  {
		if (*src++ != '(') throw Error("GotoElement(): expected `('");
		repr.push_back('(');

        while(*src != ')') {
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

const string
GotoElement::data() const
{
	string rv(repr);
	append_i32(rv, targets[0]->offset());
	return rv;
}

const GotoElement::Case
GotoElement::taken() const
{
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

void 
GotoElement::make_unconditional()
{
	if (opcode() < 5) set_opcode(module() == 1 ? 0 : 1); else set_opcode(5);
	repr.resize(8);
}

GotoCaseElement::GotoCaseElement(const char* src, ConstructionData& cdata) : PointerElement(src) 
{
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
		}
		else {
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

const string
GotoCaseElement::data() const
{
	string rv(repr);
	rv += '{';
	for (int i = 0; i < cases.size(); ++i) {
		rv += cases[i];
		append_i32(rv, targets[i]->offset());
	}
	rv += '}';
	return rv;
}

const size_t
GotoCaseElement::length() const
{
	size_t rv = repr.size() + 2;
	for (int i = 0; i < cases.size(); ++i) rv += cases[i].size() + 4;
	return rv;
}

GotoOnElement::GotoOnElement(const char* src, ConstructionData& cdata) : PointerElement(src) 
{
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

const string
GotoOnElement::data() const
{
	string rv(repr);
	rv += '{';
	for (int i = 0; i < targets.size(); ++i) {
		append_i32(rv, targets[i]->offset());
	}
	rv += '}';
	return rv;	
}

void
Pointers::set_pointers(ConstructionData& cdata)
{
	assert(target_ids.size() != 0);
	targets.reserve(target_ids.size());
	for (int i = 0; i < target_ids.size(); ++i) {
		ConstructionData::offsets_t::const_iterator it = cdata.offsets.find(target_ids[i]);
		assert(it != cdata.offsets.end());
		targets.push_back(it->second);
	}
	target_ids.clear();
}

}
