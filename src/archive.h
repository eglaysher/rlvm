
#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "defs.h"
#include "scenario.h"
#include "filemap.h"

namespace libReallive {

class Archive {
	typedef std::map<int, FilePos> scenarios_t;
	typedef std::map<int, Scenario*> accessed_t;
	scenarios_t scenarios;
	accessed_t accessed;
	string name;
	Mapping info;
public:
	Archive(string filename);
	~Archive();

	typedef std::map<int, FilePos>::const_iterator const_iterator;
	const_iterator begin() { return scenarios.begin(); }
	const_iterator end()   { return scenarios.end(); }
	
	Scenario* scenario(int index);
	
	void reset();
	void commit();
	
	void write_to(string filename);
};

}

#endif
