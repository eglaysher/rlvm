/* Copyright (c) 2006 Peter Jolly
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "scenario.h"
#include "compression.h"
#include <cassert>

#include <iostream>

using namespace std;

namespace libReallive {

Metadata::Metadata() : encoding(0) {}
  
void
Metadata::assign(const char* input)
{
  const int meta_len = read_i32(input),
            id_len   = read_i32(input + 4) + 1;
  if (meta_len < id_len + 17) return; // malformed metadata
  as_string.assign(input, meta_len);
  encoding = input[id_len + 16];
}
  
Header::Header(const char* data, const size_t length)
{
  if (length < 0x1d0 || read_i32(data + 4) != 10002)
    throw Error("not a RealLive bytecode file");
  if (read_i32(data) != 0x1d0)
    throw Error("unsupported bytecode version");

  // Debug entrypoints
  zminusone = read_i32(data + 0x2c);
  zminustwo = read_i32(data + 0x30);

  // Misc settings	
  savepoint_message = read_i32(data + 0x1c4);
  savepoint_selcom  = read_i32(data + 0x1c8);
  savepoint_seentop = read_i32(data + 0x1cc);

  // Dramatis personae
  int dplen = read_i32(data + 0x18);
  dramatis_personae.reserve(dplen);
  int offs = read_i32(data + 0x14);
  while (dplen--) {
    int elen = read_i32(data + offs);
    dramatis_personae.push_back(string(data + offs + 4, elen - 1));
    offs += elen + 4;
  }

  // If this scenario was compiled with RLdev, it may include a
  // potentially-useful metadata block.  Check for that and read it if
  // it's present.
  offs = read_i32(data + 0x14) + read_i32(data + 0x1c);
  if (offs != read_i32(data + 0x20))
    rldev_metadata.assign(data + offs);
}

Script::Script(const Header& hdr, const char* data, const size_t length)
  : uptodate(true), strip(false)
{
  // Kidoku/entrypoint table
  const int kidoku_offs = read_i32(data + 0x08);
  const size_t kidoku_length = read_i32(data + 0x0c);
  ConstructionData cdat(kidoku_length, elts.end());
  for (int i = 0; i < kidoku_length; ++i) 
    cdat.kidoku_table[i] =  read_i32(data + kidoku_offs + i * 4);

  // Decompress data
  const size_t dlen = read_i32(data + 0x24);
  std::auto_ptr<char> uncompressed = std::auto_ptr<char>(new char[dlen]);
  Compression::decompress(data + read_i32(data + 0x20),
                          read_i32(data + 0x28),
                          uncompressed.get(),
                          dlen);
  // Read bytecode
  const char* stream = uncompressed.get();
  int pos = 0;
  while (pos < dlen) {
    // Read element
    elts.push_back(BytecodeElement::read(stream, cdat));
    pointer_t it = elts.end();
    cdat.offsets[pos] = --it;
    it->offset_ = pos;
      
    // Keep track of the entrypoints
    if(it->type() == Entrypoint) {
      entrypointAssociations.insert(make_pair(it->entrypoint(), it));
    }

    // Advance
    size_t l = it->length();
    if (l <= 0) l = 1; // Failsafe: always advance at least one byte.
    stream += l;
    pos += l;
  }
  lencache = pos;
  // Resolve pointers
  for (pointer_t it = elts.begin(); it != elts.end(); ++it) {
    it->set_pointers(cdat);
    Pointers* ptrs = it->get_pointers();
    if (ptrs)
      for (Pointers::iterator pit = ptrs->begin(); pit != ptrs->end(); ++pit) 
        labels[*pit].push_back(it);
  }
}

void
Script::update_offsets()
{
  size_t offset = 0;
  int kidoku_idx = 0;
  for (pointer_t it = elts.begin(); it != elts.end(); ++it) {
    it->offset_ = offset;
    if (!strip || it->type() != Line)
      offset += it->length();
    if (it->type() == Kidoku || it->type() == Entrypoint)
      static_cast<MetaElement&>(*it).set_value(kidoku_idx++);
  }
}

void
Script::recalculate(const bool force)
{
  if (uptodate && !force) return;
  update_offsets();
  lencache = elts.back().offset() + elts.back().length();
  uptodate = true;
}

void
Script::update_pointers(pointer_t& old_target, pointer_t& new_target)
{
  labelmap::iterator it = labels.find(old_target);
  if (it != labels.end()) {
    pointer_list& l = it->second;
    for (pointer_list::iterator pi = l.begin(); pi != l.end(); ++pi) {
      assert((*pi)->type() >= Goto);
      Pointers& ptrs = *(*pi)->get_pointers();
      for (Pointers::iterator pt = ptrs.begin(); pt != ptrs.end(); ++pt) {
        if (*pt == old_target) *pt = new_target;
      }
      labels[new_target].push_back(*pi);
    }
    labels.erase(it);
  }
}

inline void
Script::remove_label(pointer_t& pt, pointer_t& it)
{
  pointer_list& l = labels[pt];
  l.erase(std::remove(l.begin(), l.end(), it), l.end());
  if (l.size() == 0) labels.erase(pt);
}

inline void
Script::remove_elt(pointer_t& it)
{
  pointer_t new_it(it);
  update_pointers(it, ++new_it);
  it = elts.erase(it);
}

const pointer_t Script::getEntrypoint(int entrypoint) const
{
  pointernumber::const_iterator it = entrypointAssociations.find(entrypoint);
  if(it == entrypointAssociations.end())
    throw Error("Unknown entrypoint");

  return it->second;
}

const string*
Scenario::rebuild()
{
  // Initialise
  string* rv = new string(0x1d0, 0);
  script.recalculate(true);
      
  // Create new header
  insert_i32(*rv, 0x00, 0x1d0);
  insert_i32(*rv, 0x04, 10002);
  insert_i32(*rv, 0x08, 0x1d0);
  std::vector<long> kidoku_table;
  std::vector<long> entrypoints(100, -1);
      
  // Build and compress scenario
  Compression::RealliveCompressor comp;
  size_t offset = 0;
  int line = 0;
  for (BytecodeList::const_iterator it = script.elts.begin();
       it != script.elts.end(); ++it)
  {
    const string data = it->data();
    if (it->type() == Line) {
      line = static_cast<const MetaElement&>(*it).value();
      if (script.strip) continue;
    }
    else if (it->type() == Kidoku) {
      kidoku_table.push_back(line);
    }
    else if (it->type() == Entrypoint) {
      int idx = static_cast<const MetaElement&>(*it).entrypoint();
      entrypoints[idx] = offset;
      kidoku_table.push_back(idx + 1000000);
    }
    comp.WriteData(data.data(), data.size());
    offset += data.size();
  }
  assert(script.strip || offset == script.size());
  comp.WriteDataEnd();
  comp.Deflate();
  comp.Flush();

  // Fix entrypoints table (unnecessary, but for compatibility)
  for (int i = 1; i < 100; ++i)
    if (entrypoints[i] == -1)
      entrypoints[i] = entrypoints[i - 1];
      
  // Fill in header
  int dramatis_length = script.strip ? 0 : header.dramatis_length();
  rv->reserve(kidoku_table.size() * 4 + 0x1d0 + dramatis_length);
  insert_i32(*rv, 0x0c, kidoku_table.size());
  insert_i32(*rv, 0x10, kidoku_table.size() * 4);
  insert_i32(*rv, 0x14, kidoku_table.size() * 4 + 0x1d0);
  insert_i32(*rv, 0x18, script.strip ? 0 : header.dramatis_personae.size());
  insert_i32(*rv, 0x1c, dramatis_length);
  //              0x20 is filled in under "Append and encrypt data" below.
  insert_i32(*rv, 0x24, script.size());
  insert_i32(*rv, 0x28, comp.Length() + 8);
  insert_i32(*rv, 0x2c, header.zminusone);
  insert_i32(*rv, 0x30, header.zminustwo);
  for (int i = 0; i < 100; ++i) insert_i32(*rv, 0x34 + i * 4, entrypoints[i]);
  insert_i32(*rv, 0x1c4, header.savepoint_message);
  insert_i32(*rv, 0x1c8, header.savepoint_selcom);
  insert_i32(*rv, 0x1cc, header.savepoint_seentop);
      
  // Write kidoku, dramatis personae, and metadata (if any)
  for (std::vector<long>::const_iterator it = kidoku_table.begin();
       it != kidoku_table.end(); ++it)
    append_i32(*rv, *it);
  if (!script.strip) {
    for (std::vector<string>::const_iterator it =
           header.dramatis_personae.begin(); 
         it != header.dramatis_personae.end(); ++it) 
    {
      append_i32(*rv, it->size() + 1);
      rv->append(*it);
      rv->push_back(0);
    }
  }
  rv->append(header.rldev_metadata.to_string());
      
  // Append and encrypt data
  size_t data_offset = rv->size();
  insert_i32(*rv, 0x20, data_offset);
  append_i32(*rv, comp.Length() + 8);
  append_i32(*rv, script.size());
  rv->append(comp.Data(), comp.Length() + 8);
  Compression::apply_mask(*rv, data_offset);
      
  // Return rebuilt scenario
  return rv;
}

Scenario::const_iterator Scenario::findEntrypoint(int entrypoint) const
{
  return script.getEntrypoint(entrypoint);
}

// Classification functions for optimisation code.

inline bool
is_line(const pointer_t& it)
{
  return it->type() == Line;
}

bool
is_goto_statement(const pointer_t& it)
{
  if (it->type() != Goto) return false;
  const CommandElement& elt = static_cast<const CommandElement&>(*it);
  return (elt.module() == 1 && elt.opcode() == 0)
      || (elt.module() == 5 && elt.opcode() == 1);
}

bool
does_not_return(const pointer_t& it)
{
  // It is safe for this to be conservative. If in doubt, return false.
  if (it->type() < Command) return false;
  const CommandElement& elt = static_cast<const CommandElement&>(*it);
  const int op = (elt.module() * 100000) | elt.opcode();
  return op == 100000 // goto
      || op == 100010 // ret
      || op == 100011 // jump
      || op == 100013 // rtl
      || op == 100017 // ret_with
      || op == 100019 // rtl_with
      || op == 401200 // end
      || op == 401201 // MenuReturn	    
      || op == 401202 // MenuReturn2	    
      || op == 401203 // ReturnMenu	    
      || op == 500001 // goto (Kinetic)
      || op == 500010 // ret (Kinetic)
      || op == 500011 // jump (Kinetic)
      || op == 500013 // rtl (Kinetic)
      ;
}

// Optimisation code per se.

Scenario&
Scenario::optimise()
{
  // It's not safe to keep any cached data around.
  script.invalidate();

  // Scan for possible optimisations.
  pointer_t it = begin();
  while (it != end()) {
    // If this is a line number immediately followed by another line
    // number, it's redundant; we can safely remove it.
    if (next(it) != end() && is_line(it) && is_line(next(it)))
      script.remove_elt(it);
      	
    // Various optimisations are possible for pointers.
    if (it->type() >= Goto) {
      Pointers& ptrs = *it->get_pointers();
      for (Pointers::iterator pt = ptrs.begin(); pt != ptrs.end(); ++pt) {
        // If the pointer points to a "goto" statement, we can
        // redirect it to the destination of that jump, cutting out
        // the middleman.
        while (is_goto_statement(*pt)) {
          // Remove obsolete pointer from label map
          script.remove_label(*pt, it);
          // Update pointer to refer to the ultimate target
          Pointers* other_ptrs = (*pt)->get_pointers();
          assert(other_ptrs != NULL);
          *pt = (*other_ptrs)[0];
          // Insert updated pointer into label map
          script.labels[*pt].push_back(it);
        }
      }
      	
      // Further optimisations are possible for gotos.
      if (it->type() == Goto) {
        // Gotos to the next statement can be removed.
        assert(ptrs.size() == 1);
        pointer_t& dest = ptrs[0];
        pointer_t seek(it);
        do {
          ++seek;
        } while (seek != end() && seek != dest 
                 && (seek->type() == Line || seek->type() == Kidoku
                     || seek->type() == Entrypoint));
        if (seek == dest) {
          it = erase(it);
          continue;
        }
        else {
          // Conditional jumps that are always taken can be replaced
          // with unconditional jumps.
          GotoElement& elt = static_cast<GotoElement&>(*it);
          const GotoElement::Case taken = elt.taken();
          if (taken == GotoElement::Always) {
            elt.make_unconditional();
          }				
          // Likewise, conditional jumps that are never taken can be
          // deleted.
          else if (taken == GotoElement::Never) {
            it = erase(it);
            continue;
          }
        }
        // Similarly, a goto to a non-returning statement can be
        // replaced with that statement directly.
        if (is_goto_statement(it) & does_not_return(dest)) {
          insert(next(it), dest->clone());
          // This is safe.  Outgoing pointers are erased
          // automatically; incoming pointers are pushed onto the next
          // element, i.e. the one we just inserted.
          it = erase(it);
        }
      }
    }
      	
    // For unconditional jumps and returns, we can delete any
    // following commands, up to the next entrypoint or pointer
    // destination, as dead code.
    if (does_not_return(it)) {
      ++it;
      while (it != end()
             && script.labels.find(it) == script.labels.end()
             && it->type() != Entrypoint) {
        it = erase(it);
      }
      continue;
    }
      	
    // Nothing else interesting: proceed.
    ++it;
  }
  return *this;
}

}
