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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include <boost/archive/text_iarchive.hpp>  // NOLINT
#include <boost/archive/text_oarchive.hpp>  // NOLINT
#include <boost/serialization/vector.hpp>   // NOLINT

#include "machine/rlmachine.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#include <functional>
#include <string>
#include <sstream>
#include <iostream>
#include <iterator>
#include <vector>

#include "libreallive/archive.h"
#include "libreallive/bytecode.h"
#include "libreallive/expression.h"
#include "libreallive/gameexe.h"
#include "libreallive/intmemref.h"
#include "libreallive/scenario.h"
#include "long_operations/pause_long_operation.h"
#include "long_operations/textout_long_operation.h"
#include "machine/long_operation.h"
#include "machine/memory.h"
#include "machine/opcode_log.h"
#include "machine/reallive_dll.h"
#include "machine/rlmodule.h"
#include "machine/rloperation.h"
#include "machine/serialization.h"
#include "machine/stack_frame.h"
#include "systems/base/graphics_system.h"
#include "systems/base/system.h"
#include "systems/base/system_error.h"
#include "systems/base/text_page.h"
#include "systems/base/text_system.h"
#include "utilities/date_util.h"
#include "utilities/exception.h"
#include "utilities/string_utilities.h"

namespace fs = boost::filesystem;

using std::cerr;
using std::cout;
using std::endl;

// -----------------------------------------------------------------------

namespace {

// Seen files are terminated with the string "SeenEnd", which isn't NULL
// terminated and has a bunch of random garbage after it.
const char seen_end[] = {130, 114,  // S
                         130, 133,  // e
                         130, 133,  // e
                         130, 142,  // n
                         130, 100,  // E
                         130, 142,  // n
                         130, 132   // d
};

const std::string SeenEnd(seen_end, 14);

bool IsNotLongOp(StackFrame& frame) {
  return frame.frame_type != StackFrame::TYPE_LONGOP;
}

}  // namespace

// -----------------------------------------------------------------------
// RLMachine
// -----------------------------------------------------------------------

RLMachine::RLMachine(System& in_system, libreallive::Archive& in_archive)
    : memory_(new Memory(*this, in_system.gameexe())),
      archive_(in_archive),
      system_(in_system) {
  // Search in the Gameexe for #SEEN_START and place us there
  Gameexe& gameexe = in_system.gameexe();
  libreallive::Scenario* scenario = NULL;
  if (gameexe.Exists("SEEN_START")) {
    int first_seen = gameexe("SEEN_START").ToInt();
    scenario = in_archive.GetScenario(first_seen);

    if (scenario == NULL)
      cerr << "WARNING: Invalid #SEEN_START in Gameexe" << endl;
  }

  if (scenario == NULL) {
    // if SEEN_START is undefined, then just grab the first SEEN.
    scenario = in_archive.GetScenario(archive_.begin()->first);
  }

  if (scenario == 0)
    throw rlvm::Exception("Invalid scenario file");
  PushStackFrame(
      StackFrame(scenario, scenario->begin(), StackFrame::TYPE_ROOT));

  // Initial value of the savepoint
  MarkSavepoint();

  // Load the "DLLs" required
  GameexeFilteringIterator it = gameexe.filtering_begin("DLL.");
  GameexeFilteringIterator end = gameexe.filtering_end();
  for (; it != end; ++it) {
    const string& name = it->ToString("");
    try {
      string index_str = it->key().substr(it->key().find_first_of(".") + 1);
      int index = std::stoi(index_str);
      LoadDLL(index, name);
    }
    catch (rlvm::Exception& e) {
      cerr << "WARNING: Don't know what to do with DLL '" << name << "'"
           << endl;
    }
  }
}

RLMachine::~RLMachine() {
  if (undefined_log_)
    cerr << *undefined_log_;
}

void RLMachine::AttachModule(RLModule* module) {
  int module_type = module->module_type();
  int module_number = module->module_number();
  unsigned int packed_module = PackModuleNumber(module_type, module_number);

  ModuleMap::iterator it = modules_.find(packed_module);
  if (it != modules_.end()) {
    RLModule& cur_mod = *it->second;
    std::ostringstream ss;
    ss << "Module identification clash: trying to overwrite " << cur_mod
       << " with " << *module << std::endl;

    // Free |module| since we took ownership of it
    delete module;

    throw rlvm::Exception(ss.str());
  }

  modules_.emplace(packed_module, std::unique_ptr<RLModule>(module));
}

int RLMachine::GetIntValue(const libreallive::IntMemRef& ref) {
  return memory_->GetIntValue(ref);
}

void RLMachine::SetIntValue(const libreallive::IntMemRef& ref, int value) {
  memory_->SetIntValue(ref, value);
}

const std::string& RLMachine::GetStringValue(int type, int location) {
  return memory_->GetStringValue(type, location);
}

void RLMachine::SetStringValue(int type, int number, const std::string& value) {
  memory_->SetStringValue(type, number, value);
}

void RLMachine::HardResetMemory() {
  memory_.reset(new Memory(*this, system().gameexe()));
}

void RLMachine::MarkSavepoint() {
  savepoint_call_stack_ = call_stack_;
  memory_->TakeSavepointSnapshot();
  system().graphics().TakeSavepointSnapshot();
  system().text().TakeSavepointSnapshot();
}

bool RLMachine::SavepointDecide(AttributeFunction func,
                                const std::string& gameexe_key) const {
  if (!mark_savepoints_)
    return false;

  int attribute = (Scenario().*func)();
  if (attribute == 1)
    return true;
  else if (attribute == 2)
    return false;

  //
  // check Gameexe key
  Gameexe& gexe = system_.gameexe();
  if (gexe.Exists(gameexe_key)) {
    int value = gexe(gameexe_key);
    if (value == 0)
      return false;
    else if (value == 1)
      return true;
  }

  // Assume default of true
  return true;
}

void RLMachine::SetMarkSavepoints(const int in) { mark_savepoints_ = in; }

bool RLMachine::ShouldSetMessageSavepoint() const {
  return SavepointDecide(&libreallive::Scenario::savepoint_message,
                         "SAVEPOINT_MESSAGE");
}

bool RLMachine::ShouldSetSelcomSavepoint() const {
  return SavepointDecide(&libreallive::Scenario::savepoint_selcom,
                         "SAVEPOINT_SELCOM");
}

bool RLMachine::ShouldSetSeentopSavepoint() const {
  return SavepointDecide(&libreallive::Scenario::savepoint_seentop,
                         "SAVEPOINT_SEENTOP");
}

void RLMachine::ExecuteNextInstruction() {
  // Do not execute any more instructions if the machine is halted.
  if (halted() == true) {
    return;
  } else {
    try {
      if (call_stack_.back().frame_type == StackFrame::TYPE_LONGOP) {
        delay_stack_modifications_ = true;
        bool ret_val = (*call_stack_.back().long_op)(*this);
        delay_stack_modifications_ = false;

        if (ret_val)
          PopStackFrame();

        // Now we can perform the queued actions
        for (auto const& action : delayed_modifications_) {
          (action)();
        }
        delayed_modifications_.clear();
      } else {
        (*(call_stack_.back().ip))->RunOnMachine(*this);
      }
    }
    catch (rlvm::UnimplementedOpcode& e) {
      AdvanceInstructionPointer();

      if (print_undefined_opcodes_) {
        cout << "(SEEN" << call_stack_.back().scenario->scene_number()
             << ")(Line " << line_ << "):  " << e.what() << endl;
      }

      if (undefined_log_)
        undefined_log_->Increment(e.opcode_name());
    }
    catch (rlvm::Exception& e) {
      if (halt_on_exception_) {
        halted_ = true;
      } else {
        // Advance the instruction pointer so as to prevent infinite
        // loops where we throw an exception, and then try again.
        AdvanceInstructionPointer();
      }

      cout << "(SEEN" << call_stack_.back().scenario->scene_number() << ")(Line "
           << line_ << ")";

      // We specialcase rlvm::Exception because we might have the name of the
      // opcode.
      if (e.operation()) {
        cout << "[" << e.operation()->name() << "]";
      }

      cout << ":  " << e.what() << endl;
    }
    catch (std::exception& e) {
      if (halt_on_exception_) {
        halted_ = true;
      } else {
        // Advance the instruction pointer so as to prevent infinite
        // loops where we throw an exception, and then try again.
        AdvanceInstructionPointer();
      }

      cout << "(SEEN" << call_stack_.back().scenario->scene_number() << ")(Line "
           << line_ << "):  " << e.what() << endl;
    }
  }
}

void RLMachine::ExecuteUntilHalted() {
  while (!halted()) {
    ExecuteNextInstruction();
  }
}

void RLMachine::AdvanceInstructionPointer() {
  if (!replaying_graphics_stack()) {
    std::vector<StackFrame>::reverse_iterator it =
        find_if(call_stack_.rbegin(), call_stack_.rend(), IsNotLongOp);

    if (it != call_stack_.rend()) {
      it->ip++;
      if (it->ip == it->scenario->end())
        halted_ = true;
    }
  }
}

std::string RLMachine::GetCommandName(const libreallive::CommandElement& f) {
  ModuleMap::iterator it =
      modules_.find(PackModuleNumber(f.modtype(), f.module()));
  std::string name;
  if (it != modules_.end())
    name = it->second->GetCommandName(*this, f);
  return name;
}

void RLMachine::ExecuteCommand(const libreallive::CommandElement& f) {
  ModuleMap::iterator it =
      modules_.find(PackModuleNumber(f.modtype(), f.module()));
  if (it != modules_.end()) {
    it->second->DispatchFunction(*this, f);
  } else {
    throw rlvm::UnimplementedOpcode(*this, f);
  }
}

void RLMachine::Jump(int scenario_num, int entrypoint) {
  // Check to make sure it's a valid scenario
  libreallive::Scenario* scenario = archive_.GetScenario(scenario_num);
  if (scenario == 0) {
    std::ostringstream oss;
    oss << "Invalid scenario number in jump (" << scenario_num << ", "
        << entrypoint << ")";
    throw rlvm::Exception(oss.str());
  }

  if (call_stack_.back().frame_type == StackFrame::TYPE_LONGOP) {
    // For some reason this is slow; REALLY slow, so for now I'm trying to
    // optimize the common case (no long operations on the back of the stack. I
    // assume there's some weird speed issue with reverse_iterator?
    //
    // The lag is noticeable on the CLANNAD menu, without profiling tools.
    std::vector<StackFrame>::reverse_iterator it =
        find_if(call_stack_.rbegin(), call_stack_.rend(), IsNotLongOp);

    if (it != call_stack_.rend()) {
      it->scenario = scenario;
      it->ip = scenario->FindEntrypoint(entrypoint);
    }
  } else {
    call_stack_.back().scenario = scenario;
    call_stack_.back().ip = scenario->FindEntrypoint(entrypoint);
  }
}

void RLMachine::Farcall(int scenario_num, int entrypoint) {
  libreallive::Scenario* scenario = archive_.GetScenario(scenario_num);
  if (scenario == 0) {
    std::ostringstream oss;
    oss << "Invalid scenario number in farcall (" << scenario_num << ", "
        << entrypoint << ")";
    throw rlvm::Exception(oss.str());
  }

  libreallive::Scenario::const_iterator it =
      scenario->FindEntrypoint(entrypoint);

  if (entrypoint == 0 && ShouldSetSeentopSavepoint())
    MarkSavepoint();

  PushStackFrame(StackFrame(scenario, it, StackFrame::TYPE_FARCALL));
}

void RLMachine::ReturnFromFarcall() {
  // Check to make sure the types match up.
  if (call_stack_.back().frame_type != StackFrame::TYPE_FARCALL) {
    throw rlvm::Exception("Callstack type mismatch in returnFromFarcall()");
  }

  PopStackFrame();
}

void RLMachine::GotoLocation(libreallive::BytecodeList::iterator new_location) {
  // Modify the current frame of the call stack so that it's
  call_stack_.back().ip = new_location;
}

void RLMachine::Gosub(libreallive::BytecodeList::iterator new_location) {
  PushStackFrame(StackFrame(
      call_stack_.back().scenario, new_location, StackFrame::TYPE_GOSUB));
}

void RLMachine::ReturnFromGosub() {
  // Check to make sure the types match up.
  if (call_stack_.back().frame_type != StackFrame::TYPE_GOSUB) {
    throw rlvm::Exception("Callstack type mismatch in returnFromGosub()");
  }

  PopStackFrame();
}

void RLMachine::PushStringValueUp(int index, const std::string& val) {
  if (index < 0 || index > 2) {
    throw rlvm::Exception("Invalid index in pushStringValue");
  }

  // Find the first real stack frame.
  std::vector<StackFrame>::reverse_iterator it =
      find_if(call_stack_.rbegin(), call_stack_.rend(), IsNotLongOp);
  if (it != call_stack_.rend()) {
    // Now try to move one stack frame up.
    it++;
    it = find_if(it, call_stack_.rend(), IsNotLongOp);

    if (it != call_stack_.rend()) {
      if ((index + 1) > it->strK.size())
        it->strK.resize(index + 1);
      it->strK[index] = val;
    }
  }
}

void RLMachine::PushLongOperation(LongOperation* long_operation) {
  PushStackFrame(StackFrame(
      call_stack_.back().scenario, call_stack_.back().ip, long_operation));
}

void RLMachine::PushStackFrame(const StackFrame& frame) {
  if (delay_stack_modifications_) {
    delayed_modifications_.push_back(
        std::bind(&RLMachine::PushStackFrame, this, frame));
    return;
  }

  call_stack_.push_back(frame);

  // Font hack. Try using a western font if we haven't already loaded a font.
  if (GetTextEncoding() == 2)
    system().set_use_western_font();
}

void RLMachine::PopStackFrame() {
  if (delay_stack_modifications_) {
    delayed_modifications_.push_back(
        std::bind(&RLMachine::PopStackFrame, this));
    return;
  }

  call_stack_.pop_back();
}

int RLMachine::GetStackSize() {
  return call_stack_.size();
}

int* RLMachine::CurrentIntLBank() {
  std::vector<StackFrame>::reverse_iterator it =
      find_if(call_stack_.rbegin(), call_stack_.rend(), IsNotLongOp);
  if (it != call_stack_.rend()) {
    return it->intL;
  }

  throw rlvm::Exception("No valid intL bank");
}

std::vector<std::string>& RLMachine::CurrentStrKBank() {
  std::vector<StackFrame>::reverse_iterator it =
      find_if(call_stack_.rbegin(), call_stack_.rend(), IsNotLongOp);
  if (it != call_stack_.rend()) {
    return it->strK;
  }

  throw rlvm::Exception("No valid strK bank");
}

void RLMachine::ClearLongOperationsOffBackOfStack() {
  if (delay_stack_modifications_) {
    delayed_modifications_.push_back(
        std::bind(&RLMachine::ClearLongOperationsOffBackOfStack, this));
    return;
  }

  // Need to do stuff here...
  while (call_stack_.size() &&
         call_stack_.back().frame_type == StackFrame::TYPE_LONGOP) {
    call_stack_.pop_back();
  }
}

void RLMachine::Reset() {
  call_stack_.clear();
  savepoint_call_stack_.clear();
  system().Reset();
}

void RLMachine::LocalReset() {
  savepoint_call_stack_.clear();
  memory_->local().reset();
  system().Reset();
}

std::shared_ptr<LongOperation> RLMachine::CurrentLongOperation() const {
  if (call_stack_.size() &&
      call_stack_.back().frame_type == StackFrame::TYPE_LONGOP) {
    return call_stack_.back().long_op;
  }

  return std::shared_ptr<LongOperation>();
}

void RLMachine::ClearCallstack() {
  while (call_stack_.size())
    PopStackFrame();
}

int RLMachine::SceneNumber() const {
  return call_stack_.back().scenario->scene_number();
}

const libreallive::Scenario& RLMachine::Scenario() const {
  return *call_stack_.back().scenario;
}

void RLMachine::ExecuteExpression(const libreallive::ExpressionElement& e) {
  e.ParsedExpression().GetIntegerValue(*this);
  AdvanceInstructionPointer();
}

int RLMachine::GetTextEncoding() const {
  return call_stack_.back().scenario->encoding();
}

int RLMachine::GetProbableEncodingType() const {
  return archive_.GetProbableEncodingType();
}

void RLMachine::PerformTextout(const libreallive::TextoutElement& e) {
  std::string unparsed_text = e.GetText();
  if (boost::starts_with(unparsed_text, SeenEnd)) {
    unparsed_text = SeenEnd;
    Halt();
  }

  PerformTextout(unparsed_text);
}

void RLMachine::PerformTextout(const std::string& cp932str) {
  std::string name_parsed_text;
  try {
    parseNames(*memory_, cp932str, name_parsed_text);
  }
  catch (rlvm::Exception& e) {
    // WEIRD: Sometimes rldev (and the official compiler?) will generate strings
    // that aren't valid shift_jis. Fall back while I figure out how to handle
    // this.
    name_parsed_text = cp932str;
  }

  std::string utf8str = cp932toUTF8(name_parsed_text, GetTextEncoding());
  TextSystem& ts = system().text();

  // Display UTF-8 characters
  std::unique_ptr<TextoutLongOperation> ptr(
      new TextoutLongOperation(*this, utf8str));

  if (system().ShouldFastForward() ||
      ts.message_no_wait() ||
      ts.script_message_nowait()) {
    ptr->set_no_wait();
  }

  // Run the textout operation once. If it doesn't fully succeed, push it onto
  // the stack.
  if (!(*ptr)(*this)) {
    PushLongOperation(ptr.release());
  }
}

void RLMachine::SetKidokuMarker(int kidoku_number) {
  // Check to see if we mark savepoints on textout
  if (ShouldSetMessageSavepoint() &&
      system_.text().GetCurrentPage().number_of_chars_on_page() == 0)
    MarkSavepoint();

  // Mark if we've previously read this piece of text.
  system_.text().SetKidokuRead(
      memory().HasBeenRead(SceneNumber(), kidoku_number));

  // Record the kidoku pair in global memory.
  memory().RecordKidoku(SceneNumber(), kidoku_number);
}

bool RLMachine::DllLoaded(const std::string& name) {
  for (auto const& dll : loaded_dlls_) {
    if (dll.second->GetDLLName() == name)
      return true;
  }

  return false;
}

void RLMachine::LoadDLL(int slot, const std::string& name) {
  RealLiveDLL* dll = RealLiveDLL::BuildDLLNamed(*this, name);
  if (dll) {
    loaded_dlls_.emplace(slot, std::unique_ptr<RealLiveDLL>(dll));
  } else {
    std::ostringstream oss;
    oss << "Can't load emulated DLL named '" << name << "'";
    throw rlvm::Exception(oss.str());
  }
}

void RLMachine::UnloadDLL(int slot) { loaded_dlls_.erase(slot); }

int RLMachine::CallDLL(int slot,
                       int one,
                       int two,
                       int three,
                       int four,
                       int five) {
  DLLMap::iterator it = loaded_dlls_.find(slot);
  if (it != loaded_dlls_.end()) {
    return it->second->CallDLL(*this, one, two, three, four, five);
  } else {
    std::ostringstream oss;
    oss << "Attempt to callDLL(" << one << ", " << two << ", " << three << ", "
        << four << ", " << five << ") on slot " << slot
        << " when no DLL is loaded there!";
    throw rlvm::Exception(oss.str());
  }
}

unsigned int RLMachine::PackModuleNumber(int modtype, int module) {
  return (modtype << 8) | module;
}

void RLMachine::SetPrintUndefinedOpcodes(bool in) {
  print_undefined_opcodes_ = in;
}

void RLMachine::RecordUndefinedOpcodeCounts() {
  undefined_log_.reset(new OpcodeLog);
}

void RLMachine::Halt() { halted_ = true; }

void RLMachine::SetHaltOnException(bool halt_on_exception) {
  halt_on_exception_ = halt_on_exception;
}

void RLMachine::SetLineNumber(const int i) {
  line_ = i;

  if (on_line_actions_) {
    ActionMap::iterator it =
        on_line_actions_->find(std::make_pair(SceneNumber(), line_));
    if (it != on_line_actions_->end()) {
      it->second();
    }
  }
}

void RLMachine::AddLineAction(const int seen,
                              const int line,
                              std::function<void(void)> function) {
  if (!on_line_actions_)
    on_line_actions_.reset(new ActionMap);

  (*on_line_actions_)[std::make_pair(seen, line)] = function;
}

template <class Archive>
void RLMachine::save(Archive& ar, unsigned int version) const {
  int line_num = line_number();
  ar& line_num;

  // Save the state of the stack when the last save point was hit
  ar& savepoint_call_stack_;
}

template <class Archive>
void RLMachine::load(Archive& ar, unsigned int version) {
  ar& line_;

  // Just thaw the call_stack_; all preprocessing was done at freeze
  // time.
  // assert(call_stack_.size() == 0);
  ar& call_stack_;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void RLMachine::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    unsigned int version) const;

template void RLMachine::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    unsigned int version);
