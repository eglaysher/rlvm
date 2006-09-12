class Scenario;

struct FilePos {
	const char* data;
	size_t length;
	FilePos() : data(NULL), length(0) {}
	FilePos(const char* d, const size_t l) : data(d), length(l) {}
};

class Header {
	friend class Scenario;
	Header(const char* data, const size_t length);
public:
	long zminusone, zminustwo, savepoint_message, savepoint_selcom, savepoint_seentop;
	std::vector<string> dramatis_personae;
	string rldev_metadata;
	const size_t dramatis_length() const {
		size_t rv = 0;
		for (std::vector<string>::const_iterator it = dramatis_personae.begin();
			 it != dramatis_personae.end(); ++it) rv += it->size() + 5;
		return rv;
	}
};

class Script {
	friend class Scenario;
	BytecodeList elts;
	bool strip;
	Script(const Header& hdr, const char* data, const size_t length);
	
	// Recalculate all internal data lazily
	mutable bool uptodate;
	mutable size_t lencache;
	void recalculate(const bool force = false);

	// Recalculate offset data (do this before serialising)
	void update_offsets();

	// Pointer/label handling.
	typedef std::vector<pointer_t> pointer_list;
	typedef std::map<pointer_t, pointer_list> labelmap;
	labelmap labels;
	void update_pointers(pointer_t&, pointer_t&);
	void remove_label(pointer_t&, pointer_t&);
	void remove_elt(pointer_t&);
public:
	// Flag size/offset recalculation as necessary (call when any data has changed)
	void invalidate() { uptodate = false; }

	const size_t size() { recalculate(); return lencache; }
};
