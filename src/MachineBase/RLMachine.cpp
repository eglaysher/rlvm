// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   RLMachine.cpp
 * @author Elliot Glaysher
 * @date   Sat Oct  7 10:54:19 2006
 *
 * @brief  Implementation of the main RLMachine class
 *
 */

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "MachineBase/RLMachine.hpp"

#include <string>
#include <sstream>
#include <iostream>
#include <iterator>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assign.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>

#include "LongOperations/PauseLongOperation.hpp"
#include "LongOperations/TextoutLongOperation.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/Memory.hpp"
#include "MachineBase/OpcodeLog.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RealLiveDLL.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/StackFrame.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/StringUtilities.hpp"
#include "Utilities/algoplus.hpp"
#include "Utilities/dateUtil.hpp"
#include "libReallive/archive.h"
#include "libReallive/bytecode.h"
#include "libReallive/expression.h"
#include "libReallive/gameexe.h"
#include "libReallive/intmemref.h"
#include "libReallive/scenario.h"

namespace fs = boost::filesystem;

using namespace std;
using namespace libReallive;

using boost::assign::list_of;
using boost::bind;
using boost::function;
using boost::lexical_cast;
using boost::shared_ptr;

// -----------------------------------------------------------------------

/// Seen files are terminated with the string "SeenEnd", which isn't NULL
/// terminated and has a bunch of random garbage after it.
static const char seen_end[] = {
  130, 114,  // S
  130, 133,  // e
  130, 133,  // e
  130, 142,  // n
  130, 100,  // E
  130, 142,  // n
  130, 132   // d
};

static const std::string SeenEnd(seen_end, 14);

// -----------------------------------------------------------------------
// RLMachine
// -----------------------------------------------------------------------

RLMachine::RLMachine(System& in_system, Archive& in_archive)
    : memory_(new Memory(in_system.gameexe())),
      halted_(false),
      print_undefined_opcodes_(false),
      halt_on_exception_(true),
      archive_(in_archive),
      line_(0),
      system_(in_system),
      mark_savepoints_(true),
      delay_stack_modifications_(false) {
  // Search in the Gameexe for #SEEN_START and place us there
  Gameexe& gameexe = in_system.gameexe();
  libReallive::Scenario* scenario = NULL;
  if (gameexe.exists("SEEN_START")) {
    int first_seen = gameexe("SEEN_START").to_int();
    scenario = in_archive.scenario(first_seen);

    if (scenario == NULL)
      cerr << "WARNING: Invalid #SEEN_START in Gameexe" << endl;
  }

  if (scenario == NULL) {
    // if SEEN_START is undefined, then just grab the first SEEN.
    scenario = in_archive.scenario(archive_.begin()->first);
  }

  if (scenario == 0)
    throw rlvm::Exception("Invalid scenario file");
  pushStackFrame(StackFrame(scenario, scenario->begin(),
                            StackFrame::TYPE_ROOT));

  // Initial value of the savepoint
  markSavepoint();

  // Load the "DLLs" required
  GameexeFilteringIterator it = gameexe.filtering_begin("DLL.");
  GameexeFilteringIterator end = gameexe.filtering_end();
  for (; it != end; ++it) {
    string index_str = it->key().substr(it->key().find_first_of(".") + 1);
    int index = lexical_cast<int>(index_str);
    const string& name = it->to_string("");
    try {
      loadDLL(index, name);
    } catch(rlvm::Exception& e) {
      cerr << "WARNING: Don't know what to do with DLL '" << name << "'"
           << endl;
    }
  }
}

// -----------------------------------------------------------------------

RLMachine::~RLMachine() {
  if (undefined_log_)
    cerr << *undefined_log_;
}

// -----------------------------------------------------------------------

void RLMachine::attachModule(RLModule* module) {
  int module_type = module->moduleType();
  int module_number = module->moduleNumber();
  unsigned int packed_module = packModuleNumber(module_type, module_number);

  ModuleMap::iterator it = modules_.find(packed_module);
  if (it != modules_.end()) {
    RLModule& cur_mod = *it->second;
    ostringstream ss;
    ss << "Module identification clash: tyring to overwrite "
       << cur_mod << " with " << *module << endl;

    throw rlvm::Exception(ss.str());
  }

  modules_.insert(packed_module, module);
}

// -----------------------------------------------------------------------

int RLMachine::getIntValue(const libReallive::IntMemRef& ref) {
  return memory_->getIntValue(ref);
}

// -----------------------------------------------------------------------

void RLMachine::setIntValue(const libReallive::IntMemRef& ref, int value) {
  memory_->setIntValue(ref, value);
}

// -----------------------------------------------------------------------

const std::string& RLMachine::getStringValue(int type, int location) {
  return memory_->getStringValue(type, location);
}

// -----------------------------------------------------------------------

void RLMachine::setStringValue(int type, int number, const std::string& value) {
  memory_->setStringValue(type, number, value);
}

// -----------------------------------------------------------------------

void RLMachine::markSavepoint() {
  savepoint_call_stack_ = call_stack_;
  system().graphics().takeSavepointSnapshot();
}

// -----------------------------------------------------------------------

bool RLMachine::savepointDecide(AttributeFunction func,
                                const std::string& gameexe_key) const {
  //
  if (!mark_savepoints_)
    return false;

  long attribute = (scenario().*func)();
  if (attribute == 1)
    return true;
  else if (attribute == 2)
    return false;

  //
  // check Gameexe key
  Gameexe& gexe = system_.gameexe();
  if (gexe.exists(gameexe_key)) {
    int value = gexe(gameexe_key);
    if (value == 0)
      return false;
    else if (value == 1)
      return true;
  }

  // Assume default of true
  return true;
}

// -----------------------------------------------------------------------

void RLMachine::setMarkSavepoints(const int in) {
  mark_savepoints_ = in;
}

// -----------------------------------------------------------------------

bool RLMachine::shouldSetMessageSavepoint() const {
  return
      savepointDecide(&Scenario::savepointMessage, "SAVEPOINT_MESSAGE");
}

// -----------------------------------------------------------------------

bool RLMachine::shouldSetSelcomSavepoint() const {
  return savepointDecide(&Scenario::savepointSelcom, "SAVEPOINT_SELCOM");
}

// -----------------------------------------------------------------------

bool RLMachine::shouldSetSeentopSavepoint() const {
  return savepointDecide(&Scenario::savepointSeentop, "SAVEPOINT_SEENTOP");
}

// -----------------------------------------------------------------------

void RLMachine::executeNextInstruction() {
  // Do not execute any more instructions if the machine is halted.
  if (halted() == true)
    return;
  else {
    try {
      if (call_stack_.back().frame_type == StackFrame::TYPE_LONGOP) {
        delay_stack_modifications_ = true;
        bool ret_val = (*call_stack_.back().long_op)(*this);
        delay_stack_modifications_ = false;

        if (ret_val)
          popStackFrame();

        // Now we can perform the queued actions
        for (vector<function<void(void)> >::iterator it =
                 delayed_modifications_.begin();
             it != delayed_modifications_.end(); ++it) {
          (*it)();
        }
        delayed_modifications_.clear();
      } else {
        call_stack_.back().ip->runOnMachine(*this);
      }
    } catch(rlvm::UnimplementedOpcode& e) {
      advanceInstructionPointer();

      if (print_undefined_opcodes_) {
        cout << "(SEEN" << call_stack_.back().scenario->sceneNumber()
             << ")(Line " << line_ << "):  " << e.what() << endl;
      }

      if (undefined_log_)
        undefined_log_->increment(e.opcodeName());
    } catch(rlvm::Exception& e) {
      if (halt_on_exception_) {
        halted_ = true;
      } else {
        // Advance the instruction pointer so as to prevent infinite
        // loops where we throw an exception, and then try again.
        advanceInstructionPointer();
      }

      cout << "(SEEN" << call_stack_.back().scenario->sceneNumber()
           << ")(Line " << line_ << ")";

      // We specialcase rlvm::Exception because we might have the name of the
      // opcode.
      if (e.operation() && e.operation()->name()) {
        cout << "[" << e.operation()->name() << "]";
      }

      cout << ":  " <<e.what() << endl;
    } catch(std::exception& e) {
      if (halt_on_exception_) {
        halted_ = true;
      } else {
        // Advance the instruction pointer so as to prevent infinite
        // loops where we throw an exception, and then try again.
        advanceInstructionPointer();
      }

      cout << "(SEEN" << call_stack_.back().scenario->sceneNumber()
           << ")(Line " << line_ << "):  " << e.what() << endl;
    }
  }
}

// -----------------------------------------------------------------------

void RLMachine::executeUntilHalted() {
  while (!halted()) {
    executeNextInstruction();
  }
}

// -----------------------------------------------------------------------

void RLMachine::advanceInstructionPointer() {
  std::vector<StackFrame>::reverse_iterator it =
      find_if (call_stack_.rbegin(), call_stack_.rend(),
              bind(&StackFrame::frame_type, _1) != StackFrame::TYPE_LONGOP);

  if (it != call_stack_.rend()) {
    it->ip++;
    if (it->ip == it->scenario->end())
      halted_ = true;
  }
}

// -----------------------------------------------------------------------

void RLMachine::executeCommand(const CommandElement& f) {
  ModuleMap::iterator it = modules_.find(packModuleNumber(f.modtype(),
                                                          f.module()));
  if (it != modules_.end()) {
    it->second->dispatchFunction(*this, f);
  } else {
    throw rlvm::UnimplementedOpcode(f.modtype(), f.module(), f.opcode(),
                                    f.overload());
  }
}

// -----------------------------------------------------------------------

void RLMachine::jump(int scenario_num, int entrypoint) {
  // Check to make sure it's a valid scenario
  libReallive::Scenario* scenario = archive_.scenario(scenario_num);
  if (scenario == 0) {
    ostringstream oss;
    oss << "Invalid scenario number in jump (" << scenario_num << ", "
        << entrypoint << ")";
    throw rlvm::Exception(oss.str());
  }

  if (call_stack_.back().frame_type == StackFrame::TYPE_LONGOP) {
    // TODO: For some reason this is slow; REALLY slow, so for now I'm trying
    // to optimize the common case (no long operations on the back of the
    // stack. I assume there's some weird speed issue with reverse_iterator?
    //
    // The lag is noticeable on the CLANNAD menu, without profiling tools.
    std::vector<StackFrame>::reverse_iterator it =
        find_if (call_stack_.rbegin(), call_stack_.rend(),
                bind(&StackFrame::frame_type, _1) != StackFrame::TYPE_LONGOP);

    if (it != call_stack_.rend()) {
      it->scenario = scenario;
      it->ip = scenario->findEntrypoint(entrypoint);
    }
  } else {
    call_stack_.back().scenario = scenario;
    call_stack_.back().ip = scenario->findEntrypoint(entrypoint);
  }
}

// -----------------------------------------------------------------------

void RLMachine::farcall(int scenario_num, int entrypoint) {
  libReallive::Scenario* scenario = archive_.scenario(scenario_num);
  if (scenario == 0) {
    ostringstream oss;
    oss << "Invalid scenario number in farcall (" << scenario_num << ", "
        << entrypoint << ")";
    throw rlvm::Exception(oss.str());
  }

  libReallive::Scenario::const_iterator it =
      scenario->findEntrypoint(entrypoint);

  if (entrypoint == 0 && shouldSetSeentopSavepoint())
    markSavepoint();

  pushStackFrame(StackFrame(scenario, it, StackFrame::TYPE_FARCALL));
}

// -----------------------------------------------------------------------

void RLMachine::returnFromFarcall() {
  // Check to make sure the types match up.
  if (call_stack_.back().frame_type != StackFrame::TYPE_FARCALL) {
    throw rlvm::Exception("Callstack type mismatch in returnFromFarcall()");
  }

  popStackFrame();
}

// -----------------------------------------------------------------------

void RLMachine::gotoLocation(libReallive::BytecodeList::iterator new_location) {
  // Modify the current frame of the call stack so that it's
  call_stack_.back().ip = new_location;
}

// -----------------------------------------------------------------------

void RLMachine::gosub(libReallive::BytecodeList::iterator new_location) {
  pushStackFrame(StackFrame(call_stack_.back().scenario, new_location,
                            StackFrame::TYPE_GOSUB));
}

// -----------------------------------------------------------------------

void RLMachine::returnFromGosub() {
  // Check to make sure the types match up.
  if (call_stack_.back().frame_type != StackFrame::TYPE_GOSUB) {
    throw rlvm::Exception("Callstack type mismatch in returnFromGosub()");
  }

  popStackFrame();
}

// -----------------------------------------------------------------------

void RLMachine::pushLongOperation(LongOperation* long_operation) {
  pushStackFrame(StackFrame(call_stack_.back().scenario, call_stack_.back().ip,
                            long_operation));
}

// -----------------------------------------------------------------------

void RLMachine::pushStackFrame(const StackFrame& frame) {
  if (delay_stack_modifications_) {
    delayed_modifications_.push_back(
        bind(&RLMachine::pushStackFrame, this, frame));
    return;
  }

  call_stack_.push_back(frame);
}

// -----------------------------------------------------------------------

void RLMachine::popStackFrame() {
  if (delay_stack_modifications_) {
    delayed_modifications_.push_back(bind(&RLMachine::popStackFrame, this));
    return;
  }

  call_stack_.pop_back();
}

// -----------------------------------------------------------------------

void RLMachine::clearLongOperationsOffBackOfStack() {
  if (delay_stack_modifications_) {
    delayed_modifications_.push_back(
        bind(&RLMachine::clearLongOperationsOffBackOfStack, this));
    return;
  }

  // Need to do stuff here...
  while (call_stack_.size() &&
         call_stack_.back().frame_type == StackFrame::TYPE_LONGOP) {
    call_stack_.pop_back();
  }
}

// -----------------------------------------------------------------------

void RLMachine::reset() {
  call_stack_.clear();
  savepoint_call_stack_.clear();
  system().reset();
}

// -----------------------------------------------------------------------

void RLMachine::localReset() {
  savepoint_call_stack_.clear();
  memory_->local().reset();
  system().reset();
}

// -----------------------------------------------------------------------

shared_ptr<LongOperation> RLMachine::currentLongOperation() const {
  if (call_stack_.size() &&
      call_stack_.back().frame_type == StackFrame::TYPE_LONGOP) {
    return call_stack_.back().long_op;
  }

  return shared_ptr<LongOperation>();
}

// -----------------------------------------------------------------------

void RLMachine::clearCallstack() {
  while (call_stack_.size())
    popStackFrame();
}

// -----------------------------------------------------------------------

int RLMachine::sceneNumber() const {
  return call_stack_.back().scenario->sceneNumber();
}

// -----------------------------------------------------------------------

const Scenario& RLMachine::scenario() const {
  return *call_stack_.back().scenario;
}

// -----------------------------------------------------------------------

void RLMachine::executeExpression(const ExpressionElement& e) {
  e.parsedExpression().integerValue(*this);
  advanceInstructionPointer();
}

// -----------------------------------------------------------------------

int RLMachine::getTextEncoding() const {
  return call_stack_.back().scenario->encoding();
}

// -----------------------------------------------------------------------

void RLMachine::performTextout(const TextoutElement& e) {
  std::string unparsed_text = e.text();
  if (boost::starts_with(unparsed_text, SeenEnd)) {
    unparsed_text = SeenEnd;
    halt();
  }

  performTextout(unparsed_text);
}

// -----------------------------------------------------------------------

void RLMachine::performTextout(const std::string& cp932str) {
  std::string name_parsed_text;
  try {
    parseNames(*memory_, cp932str, name_parsed_text);
  } catch(rlvm::Exception& e) {
    // WEIRD: Sometimes rldev (and the official compiler?) will generate strings
    // that aren't valid shift_jis. Fall back while I figure out how to handle
    // this.
    name_parsed_text = cp932str;
  }

  std::string utf8str = cp932toUTF8(name_parsed_text, getTextEncoding());
  TextSystem& ts = system().text();

  // Display UTF-8 characters
  auto_ptr<TextoutLongOperation> ptr(new TextoutLongOperation(*this, utf8str));

  if (system().fastForward() || ts.messageNoWait())
    ptr->setNoWait();

  pushLongOperation(ptr.release());
}

// -----------------------------------------------------------------------

void RLMachine::setKidokuMarker(int kidoku_number) {
  // Check to see if we mark savepoints on textout
  if (shouldSetMessageSavepoint() &&
      system_.text().currentPage().numberOfCharsOnPage() == 0)
    markSavepoint();

  // Mark if we've previously read this piece of text.
  system_.text().setKidokuRead(
      memory().hasBeenRead(sceneNumber(), kidoku_number));

  // Record the kidoku pair in global memory.
  memory().recordKidoku(sceneNumber(), kidoku_number);
}

// -----------------------------------------------------------------------

bool RLMachine::dllLoaded(const std::string& name) {
  for (DLLMap::const_iterator it = loaded_dlls_.begin();
       it != loaded_dlls_.end(); ++it) {
    if (it->second->name() == name)
      return true;
  }

  return false;
}

// -----------------------------------------------------------------------

void RLMachine::loadDLL(int slot, const std::string& name) {
  auto_ptr<RealLiveDLL> dll(RealLiveDLL::BuildDLLNamed(*this, name));
  if (dll.get()) {
    loaded_dlls_.insert(slot, dll);
  } else {
    ostringstream oss;
    oss << "Can't load emulated dll named '" << name << "'";
    throw rlvm::Exception(oss.str());
  }
}

// -----------------------------------------------------------------------

void RLMachine::unloadDLL(int slot) {
  loaded_dlls_.erase(slot);
}

// -----------------------------------------------------------------------

int RLMachine::callDLL(int slot, int one, int two, int three, int four,
                       int five) {
  DLLMap::iterator it = loaded_dlls_.find(slot);
  if (it != loaded_dlls_.end()) {
    return it->second->callDLL(*this, one, two, three, four, five);
  } else {
    ostringstream oss;
    oss << "Attempt to callDLL(" << one << ", " << two << ", " << three
        << ", " << four << ", " << five << ") on slot " << slot
        << " when no DLL is loaded there!";
    throw rlvm::Exception(oss.str());
  }
}

// -----------------------------------------------------------------------

unsigned int RLMachine::packModuleNumber(int modtype, int module) {
  return (modtype << 8) | module;
}

// -----------------------------------------------------------------------

void RLMachine::unpackModuleNumber(unsigned int packed_module_number,
                                   int& modtype, int& module) {
  modtype = packed_module_number >> 8;
  module = packed_module_number && 0xFF;
}

// -----------------------------------------------------------------------

void RLMachine::setPrintUndefinedOpcodes(bool in) {
  print_undefined_opcodes_ = in;
}

// -----------------------------------------------------------------------

void RLMachine::recordUndefinedOpcodeCounts() {
  undefined_log_.reset(new OpcodeLog);
}

// -----------------------------------------------------------------------

void RLMachine::halt() {
  halted_ = true;
}

// -----------------------------------------------------------------------

void RLMachine::setHaltOnException(bool halt_on_exception) {
  halt_on_exception_ = halt_on_exception;
}

// -----------------------------------------------------------------------

void RLMachine::setLineNumber(const int i) {
  line_ = i;
}

// -----------------------------------------------------------------------

template<class Archive>
void RLMachine::save(Archive & ar, unsigned int version) const {
  int line_num = lineNumber();
  ar & line_num;

  /// Save the state of the stack when the last save point was hit
  ar & savepoint_call_stack_;
}

// -----------------------------------------------------------------------

template<class Archive>
void RLMachine::load(Archive & ar, unsigned int version) {
  ar & line_;

  // Just thaw the call_stack_; all preprocessing was done at freeze
  // time.
  // assert(call_stack_.size() == 0);
  ar & call_stack_;
}

// -----------------------------------------------------------------------


// Explicit instantiations for text archives (since we hide the
// implementation)

template void RLMachine::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive & ar, unsigned int version) const;

template void RLMachine::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive & ar, unsigned int version);
