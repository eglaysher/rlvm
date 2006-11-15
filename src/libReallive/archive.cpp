
#include "archive.h"
#include "string.h"

namespace libReallive {

Archive::Archive(string filename) : name(filename), info(filename, Read)
{
	// Read TOC
	const char* idx = info.get();
	for (int i = 0; i < 10000; ++i, idx += 8) {
		const int offs = read_i32(idx);
		if (offs) scenarios[i] = FilePos(info.get() + offs, read_i32(idx + 4));
	}
}

Archive::~Archive()
{
	for (accessed_t::iterator it = accessed.begin(); it != accessed.end(); ++it) delete it->second;
}

Scenario*
Archive::scenario(int index) {
	accessed_t::const_iterator at = accessed.find(index);
	if (at != accessed.end()) return at->second;
	scenarios_t::const_iterator st = scenarios.find(index);
	if (st != scenarios.end()) return accessed[index] = new Scenario(st->second, index);
	return NULL;
}

void
Archive::write_to(string filename)
{
	// Handle the special case of writing to the input file.
	if (filename == name) {
		commit();
		return;
	}	
	// Get length of output file.
	size_t outlen = 80000;
	std::map<int, const string*> compressed;
	for (scenarios_t::const_iterator it = scenarios.begin(); it != scenarios.end(); ++it) {
		accessed_t::const_iterator at = accessed.find(it->first);
		if (at == accessed.end()) {
			outlen += it->second.length;
			compressed[it->first] = NULL;
		}
		else {
//printf("SEEN%04d\n", it->first);
			const string* s = at->second->rebuild();
			outlen += s->size();
			compressed[it->first] = s;
		}
	}
	// Write output.
	Mapping out(filename, Write, outlen);
	char* omem = out.get();
	char* ptr = out.get() + 80000;
	memset((void*) omem, 0, 80000);
	for (scenarios_t::const_iterator it = scenarios.begin(); it != scenarios.end(); ++it) {
		char* idxpos = omem + it->first * 8;
		insert_i32(idxpos, ptr - omem);
		const string* s = compressed[it->first];
		if (s == NULL) {
			insert_i32(idxpos + 4, it->second.length);
			memcpy(ptr, it->second.data, it->second.length);
			ptr += it->second.length;
		}
		else {
			insert_i32(idxpos + 4, s->size());
			memcpy(ptr, s->data(), s->size());
			ptr += s->size();
			delete s;
		}
	}
}

void
Archive::reset()
{
	for (accessed_t::iterator it = accessed.begin(); it != accessed.end(); ++it) delete it->second;
	accessed.clear();
}

void
Archive::commit()
{
	if (accessed.empty()) return;
	// Get unique filename.
	char tmpfile[256];
	for (unsigned long i = 0; i < 0xffffffff; ++i) {
		sprintf(tmpfile, "%s.%x", name.c_str(), i);
		FILE* f;
		if (f = fopen(tmpfile, "r")) fclose(f); else break;
	}
	// Output.
	write_to(tmpfile);
	reset();
	info.replace(tmpfile);
}

}
