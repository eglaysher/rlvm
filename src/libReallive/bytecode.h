/* Bytecode class structure. */

#ifndef BYTECODE_H
#define BYTECODE_H

#include "defs.h"
#include <boost/scoped_ptr.hpp>

namespace libReallive {

// A nasty-hacky (but simple and efficient) form of RTTI.
enum ElementType { 
	Unspecified, Data, 
	Line, Kidoku, Entrypoint, 
	Textout, Expression, 
	// Everything after this point must be a CommandElement or subtype.
	Command, Select, Function, 
	// Everything after this point must be a PointerElement or subtype.
	Goto, GotoCase, GotoOn
};

// List definitions.
class ExpressionPiece;
class BytecodeElement;
typedef boost::ptr_list<BytecodeElement> BytecodeList;
typedef BytecodeList::iterator pointer_t;

struct ConstructionData {
	pointer_t null;
	std::vector<unsigned long> kidoku_table;
	typedef std::map<unsigned long, pointer_t> offsets_t;
	offsets_t offsets;	
private:
  friend class Script;
  ConstructionData(size_t kt, pointer_t pt);
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

	virtual const ElementType type() const { return Unspecified; }

	const size_t offset() const { return offset_; }

	virtual const string data() const { return string(); }
	virtual const size_t length() const { return 0; }
	
	virtual Pointers* get_pointers() { return NULL; }
	virtual void set_pointers(ConstructionData& cdata) {}
	
	// Note that x.clone() != x, since the copy constructor assigns a new id.
	virtual BytecodeElement* clone() const { return new BytecodeElement(*this); }
	
	virtual ~BytecodeElement() {}
	
	BytecodeElement() : id(id_src++) {}

  // Needed for MetaElement during reading the script
  virtual const int entrypoint() const { return -999; }	

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
	virtual const ElementType type() const { return Data; }
	virtual const string data() const { return repr; }
	virtual const size_t length() const { return repr.size(); }
	
	virtual DataElement* clone() const { return new DataElement(*this); }
  DataElement();
  DataElement(const char* src, const size_t count);
};

} namespace std {
template<> struct less<libReallive::pointer_t> {
	bool operator() (const libReallive::pointer_t& a, const libReallive::pointer_t& b) {
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
	const ElementType type() const { return type_ == Line_ ? Line : (type_ == Kidoku_ ? Kidoku : Entrypoint); }

	const string data() const { 
		string rv(3, 0);
		rv[0] = type_ == Entrypoint_ ? entrypoint_marker : type_;
		insert_i16(rv, 1, value_);
		return rv;
	}
	const size_t length() const { return 3; }

	const int value() const { return value_; }
	void set_value(const int value) { value_ = value; }
	const int entrypoint() const { return type_ == Entrypoint_ ? entrypoint_index : -999; }
	
  MetaElement(const ConstructionData* cv, const char* src);
	
	MetaElement* clone() const { return new MetaElement(*this); }
};

// Display-text elements.

class TextoutElement : public DataElement {
public:
	const ElementType type() const { return Textout; }
	const string text() const;
	void set_text(const char* src);
	void set_text(const string& src) { set_text(src.c_str()); }
	TextoutElement(const char* src);
  TextoutElement();
	TextoutElement* clone() const { return new TextoutElement(*this); }
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
  const ElementType type() const { return Expression; }
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
};

// Command elements.

class CommandElement : public DataElement {
public:
	virtual const ElementType type() const { return Command; }
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
//  const ExpressionPiece& get_parsed_parameter(

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
};

class SelectElement : public CommandElement {
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
	params_t params;
	int firstline;
public:
	const ElementType type() const { return Select; }
	ExpressionElement window();
	const string text(const int index) const;
	
	const string data() const;
	const size_t length() const;
	
	const size_t param_count() const { return params.size(); }
	string get_param(int i) const { string rv(params[i].cond); rv.append(params[i].text); return rv; }
	
	SelectElement(const char* src);
	SelectElement* clone() const { return new SelectElement(*this); }
};

class FunctionElement : public CommandElement {
	std::vector<string> params;
public:
	virtual const ElementType type() const { return Function; }
	FunctionElement(const char* src);

	const string data() const;
	const size_t length() const;

	const size_t param_count() const { return params.size(); }
	string get_param(int i) const { return params[i]; }

	virtual FunctionElement* clone() const { return new FunctionElement(*this); }
};

class PointerElement : public CommandElement {
protected:
	Pointers targets;
public:
  PointerElement(const char* src);
	virtual const ElementType type() const = 0;
	virtual PointerElement* clone() const = 0;
	virtual const string data() const = 0;
	virtual const size_t length() const = 0;
	void set_pointers(ConstructionData& cdata) { targets.set_pointers(cdata); }
	Pointers* get_pointers() { return &targets; }
  const Pointers& get_pointersRef() const { return targets; }
};

class GotoElement : public PointerElement {
	std::vector<string> params;
public:
	const ElementType type() const { return Goto; }
	GotoElement(const char* src, ConstructionData& cdata);
	GotoElement* clone() const { return new GotoElement(*this); }

	void make_unconditional();

	enum Case { Unconditional, Always, Never, Variable };
	const Case taken() const;

	// The pointer is not counted as a parameter.
//	const size_t param_count() const { return repr.size() == 8 ? 0 : 1; }
//	string get_param(int i) const { return i == 0 ? (repr.size() == 8 ? string() : repr.substr(9, repr.size() - 10)) : string(); }
	const size_t param_count() const { return params.size(); }
	string get_param(int i) const { return params[i]; }

	const string data() const;
	const size_t length() const { return repr.size() + 4; }
};

class GotoCaseElement : public PointerElement {
	std::vector<string> cases;
public:
	const ElementType type() const { return GotoCase; }
	GotoCaseElement(const char* src, ConstructionData& cdata);
	GotoCaseElement* clone() const { return new GotoCaseElement(*this); }

	const string data() const;
	const size_t length() const;

	// The cases are not counted as parameters.
	const size_t param_count() const { return 1; }
	string get_param(int i) const { return i == 0 ? repr.substr(8, repr.size() - 8) : string(); }

  // Accessors for the cases
  const size_t case_count() const { return cases.size(); }
  const string get_case(int i) const { return cases[i]; }
};

class GotoOnElement : public PointerElement {
public:
	const ElementType type() const { return GotoOn; }
	GotoOnElement(const char* src, ConstructionData& cdata);
	GotoOnElement* clone() const { return new GotoOnElement(*this); }

	const string data() const;
	const size_t length() const { return repr.size() + targets.size() * 4 + 2; }

	// The pointers are not counted as parameters.
	const size_t param_count() const { return 1; }
	string get_param(int i) const { return i == 0 ? repr.substr(8, repr.size() - 8) : string(); }
};

}

#endif
