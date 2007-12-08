// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#include "libReallive/archive.h"
#include "libReallive/bytecode.h"
#include "libReallive/scenario.h"
#include "libReallive/expression.h"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/Memory.hpp"
#include "MachineBase/StackFrame.hpp"

#include "libReallive/intmemref.h"
#include "libReallive/gameexe.h"

#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include "Modules/cp932toUnicode.hpp"

#include "Modules/TextoutLongOperation.hpp"
#include "Modules/PauseLongOperation.hpp"

#include "dateUtil.hpp"
#include "algoplus.hpp"

#include <string>
#include <sstream>
#include <iostream>
#include <iterator>

#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/assign.hpp>

namespace fs = boost::filesystem;

using namespace std;
using namespace libReallive;

using boost::bind;
using boost::assign::list_of;

// -----------------------------------------------------------------------
// RLMachine
// -----------------------------------------------------------------------

RLMachine::RLMachine(System& inSystem, Archive& inArchive) 
  : m_memory(new Memory), m_halted(false), m_haltOnException(true), 
    m_archive(inArchive), m_system(inSystem), m_markSavepoints(true)
{
  // Search in the Gameexe for #SEEN_START and place us there
  Gameexe& gameexe = inSystem.gameexe();
  libReallive::Scenario* scenario = NULL;
  if(gameexe.exists("SEEN_START"))
  {
    int firstSeen = gameexe("SEEN_START").to_int();
    scenario = inArchive.scenario(firstSeen);

    if(scenario == NULL)
      cerr << "WARNING: Invalid #SEEN_START in Gameexe" << endl;
  }

  if(scenario == NULL)
  {
    // if SEEN_START is undefined, then just grab the first SEEN.
    scenario = inArchive.scenario(m_archive.begin()->first);
  }

  if(scenario == 0)
    throw rlvm::Exception("Invalid scenario file");
  pushStackFrame(StackFrame(scenario, scenario->begin(), StackFrame::TYPE_ROOT));

  // Initial value of the savepoint
  markSavepoint();
}

// -----------------------------------------------------------------------

RLMachine::~RLMachine()
{}

// -----------------------------------------------------------------------

void RLMachine::attachModule(RLModule* module) 
{
  int moduleType = module->moduleType();
  int moduleNumber = module->moduleNumber();
  unsigned int packedModule = packModuleNumber(moduleType, moduleNumber);

  ModuleMap::iterator it = modules.find(packedModule);
  if(it != modules.end())
  {
    RLModule& curMod = *it;
    ostringstream ss;
    ss << "Module identification clash: tyring to overwrite "
       << curMod << " with " << *module << endl;

    throw rlvm::Exception(ss.str());
  }

  modules.insert(packedModule, module);
}

// -----------------------------------------------------------------------

// void RLMachine::saveGlobalMemory()
// {
//   fs::path home = m_system.gameSaveDirectory() / "global.sav";
//   fs::ofstream file(home);
//   if(!file)
//   {
// 	ostringstream oss;
// 	oss << "Could not open global memory file.";
// 	throw rlvm::Exception(oss.str());
//   }

//   Serialization::saveGlobalMemoryTo(file, *this);
// }

// -----------------------------------------------------------------------

int RLMachine::getIntValue(const libReallive::IntMemRef& ref)
{
  return m_memory->getIntValue(ref);
}

// -----------------------------------------------------------------------

void RLMachine::setIntValue(const libReallive::IntMemRef& ref, int value)
{
  m_memory->setIntValue(ref, value);
}

// -----------------------------------------------------------------------

const std::string& RLMachine::getStringValue(int type, int location)
{
  return m_memory->getStringValue(type, location);
}

// -----------------------------------------------------------------------

void RLMachine::setStringValue(int type, int number, const std::string& value)
{
  m_memory->setStringValue(type, number, value);
}

// -----------------------------------------------------------------------

void RLMachine::markSavepoint()
{
  savepointCallStack = callStack;
  system().graphics().takeSavepointSnapshot();
}

// -----------------------------------------------------------------------

bool RLMachine::savepointDecide(AttributeFunction func, 
                                const std::string& gameexeKey) const
{
  // 
  if(!m_markSavepoints)
    return false;

  long attribute = (scenario().*func)();
  if(attribute == 1)
    return true;
  else if(attribute == 2)
    return false;

  //
  // check Gameexe key
  Gameexe& gexe = m_system.gameexe();
  if(gexe.exists(gameexeKey))
  {
    int value = gexe(gameexeKey);
    if(value == 0)
      return false;
    else if(value == 1)
      return true;
  }

  // Assume default of true
  return true;
}

// -----------------------------------------------------------------------

void RLMachine::setMarkSavepoints(const int in)
{
  m_markSavepoints = in;
}

// -----------------------------------------------------------------------

bool RLMachine::shouldSetMessageSavepoint() const
{
  return savepointDecide(&Scenario::savepointMessage, "SAVEPOINT_MESSAGE");
}

// -----------------------------------------------------------------------

bool RLMachine::shouldSetSelcomSavepoint() const
{
  return savepointDecide(&Scenario::savepointSelcom, "SAVEPOINT_SELCOM");
}

// -----------------------------------------------------------------------

bool RLMachine::shouldSetSeentopSavepoint() const
{
  return savepointDecide(&Scenario::savepointSeentop, "SAVEPOINT_SEENTOP");
}

// -----------------------------------------------------------------------

void RLMachine::executeNextInstruction() 
{
  // Do not execute any more instructions if the machine is halted.
  if(halted() == true)
    return;
  else 
  {
    try 
    {
      if(callStack.back().frameType == StackFrame::TYPE_LONGOP)
      {
        bool retVal = (*callStack.back().longOp)(*this);
        if(retVal)
          popStackFrame();
      }
      else
        callStack.back().ip->runOnMachine(*this);
    }
    catch(std::exception& e) {
      if(m_haltOnException) {
        m_halted = true;
      } else {
        // Advance the instruction pointer so as to prevent infinite
        // loops where we throw an exception, and then try again.
        advanceInstructionPointer();
      }

      cout << "(SEEN" << callStack.back().scenario()->sceneNumber() 
           << ")(Line " << m_line << "):  " << e.what() << endl;
    }
  }
}

// -----------------------------------------------------------------------

void RLMachine::executeUntilHalted()
{
  while(!halted()) {
    executeNextInstruction();
  }
}

// -----------------------------------------------------------------------

void RLMachine::advanceInstructionPointer()
{
  std::vector<StackFrame>::reverse_iterator it = 
    find_if(callStack.rbegin(), callStack.rend(),
            bind(&StackFrame::frameType, _1) != StackFrame::TYPE_LONGOP);

  if(it != callStack.rend())
  {
    it->ip++;
    if(it->ip == it->scenario()->end())
      m_halted = true;
  }
}

// -----------------------------------------------------------------------

void RLMachine::executeCommand(const CommandElement& f) {
//   cerr << "About to execute opcode<" << f.modtype() << ":" << f.module() << ":" 
//        << f.opcode() << ", " << f.overload() << ">" << endl;
  ModuleMap::iterator it = modules.find(packModuleNumber(f.modtype(), f.module()));
  if(it != modules.end()) {
    it->dispatchFunction(*this, f);
  } else {
    ostringstream ss;
    ss << "Undefined module<" << f.modtype() << ":" << f.module() << ">";
    throw rlvm::Exception(ss.str());
  }
}

// -----------------------------------------------------------------------

void RLMachine::jump(int scenarioNum, int entrypoint) 
{
  // Check to make sure it's a valid scenario
  libReallive::Scenario* scenario = m_archive.scenario(scenarioNum);
  if(scenario == 0)
    throw rlvm::Exception("Invalid scenario number in jump");

  callStack.back().setScenario(scenario);
  callStack.back().ip = scenario->findEntrypoint(entrypoint);
}

// -----------------------------------------------------------------------

void RLMachine::farcall(int scenarioNum, int entrypoint) 
{
  libReallive::Scenario* scenario = m_archive.scenario(scenarioNum);
  if(scenario == 0)
    throw rlvm::Exception("Invalid scenario number in jump");

  libReallive::Scenario::const_iterator it = scenario->findEntrypoint(entrypoint);

  if(entrypoint == 0 && shouldSetSeentopSavepoint())
    markSavepoint();

  pushStackFrame(StackFrame(scenario, it, StackFrame::TYPE_FARCALL));
}

// -----------------------------------------------------------------------

void RLMachine::returnFromFarcall() 
{
  // Check to make sure the types match up.
  if(callStack.back().frameType != StackFrame::TYPE_FARCALL) {
    throw rlvm::Exception("Callstack type mismatch in returnFromFarcall()");
  }

  popStackFrame();
}

// -----------------------------------------------------------------------

void RLMachine::gotoLocation(BytecodeList::iterator newLocation) {
  // Modify the current frame of the call stack so that it's 
  callStack.back().ip = newLocation;
}

// -----------------------------------------------------------------------

void RLMachine::gosub(BytecodeList::iterator newLocation) 
{
  pushStackFrame(StackFrame(callStack.back().scenario(), newLocation, 
                            StackFrame::TYPE_GOSUB));
}

// -----------------------------------------------------------------------

void RLMachine::returnFromGosub()
{
  // Check to make sure the types match up.
  if(callStack.back().frameType != StackFrame::TYPE_GOSUB) {
    throw rlvm::Exception("Callstack type mismatch in returnFromGosub()");
  }

  popStackFrame();
}

// -----------------------------------------------------------------------

void RLMachine::pushLongOperation(LongOperation* longOperation)
{
  pushStackFrame(StackFrame(callStack.back().scenario(), callStack.back().ip,
                            longOperation));
}

// -----------------------------------------------------------------------

void RLMachine::pushStackFrame(const StackFrame& frame)
{
  if(callStack.size() && callStack.back().frameType == StackFrame::TYPE_LONGOP)
	callStack.back().longOp->looseFocus();

  callStack.push_back(frame);
}

// -----------------------------------------------------------------------

void RLMachine::popStackFrame()
{
  callStack.pop_back();  

  if(callStack.size() && callStack.back().frameType == StackFrame::TYPE_LONGOP)
	callStack.back().longOp->gainFocus();
}

// -----------------------------------------------------------------------

bool RLMachine::inLongOperation() const
{
  return callStack.size() && 
    callStack.back().frameType == StackFrame::TYPE_LONGOP;
}

// -----------------------------------------------------------------------

void RLMachine::clearCallstack() 
{
  while(callStack.size())
    popStackFrame();
}

// -----------------------------------------------------------------------

int RLMachine::sceneNumber() const
{
  return callStack.back().scenario()->sceneNumber();
}

// -----------------------------------------------------------------------

const Scenario& RLMachine::scenario() const
{
  return *callStack.back().scenario();
}

// -----------------------------------------------------------------------

void RLMachine::executeExpression(const ExpressionElement& e) 
{
  e.parsedExpression().integerValue(*this);
  advanceInstructionPointer();
}

// -----------------------------------------------------------------------

int RLMachine::getTextEncoding() const
{
  return callStack.back().scenario()->encoding();
}

// -----------------------------------------------------------------------

void RLMachine::performTextout(const TextoutElement& e)
{
  std::string utf8str = cp932toUTF8(e.text(), getTextEncoding());

  TextSystem& ts = system().text();

  // Display UTF-8 characters
  auto_ptr<TextoutLongOperation> ptr(new TextoutLongOperation(*this, utf8str));

  if(ts.messageNoWait())
    ptr->setNoWait();
 
  pushLongOperation(ptr.release());
  advanceInstructionPointer();
}

// -----------------------------------------------------------------------

/// @todo Only markSavepoint() if the current screen is empty.
void RLMachine::setKidokuMarker(int kidokuNumber)
{
  // Check to see if we mark savepoints on textout
  if(shouldSetMessageSavepoint())
    markSavepoint();
  else
    cerr << "Shouldn't set message savepoint!" << endl;  
}

// -----------------------------------------------------------------------

unsigned int RLMachine::packModuleNumber(int modtype, int module)
{
  return (modtype << 8) | module;
}

// -----------------------------------------------------------------------

void RLMachine::unpackModuleNumber(unsigned int packedModuleNumber, int& modtype,  
                                   int& module)
{
  modtype = packedModuleNumber >> 8;
  module = packedModuleNumber && 0xFF;
}

// -----------------------------------------------------------------------

void RLMachine::halt() 
{
  m_halted = true; 
}

// -----------------------------------------------------------------------

void RLMachine::setHaltOnException(bool haltOnException) 
{
  m_haltOnException = haltOnException;
}

// -----------------------------------------------------------------------

void RLMachine::setLineNumber(const int i) 
{
//  cerr << "On line " << i << endl;
  m_line = i;
}

// -----------------------------------------------------------------------

template<class Archive>
void RLMachine::save(Archive & ar, unsigned int version) const
{
  int lineNum = lineNumber();
  ar & lineNum;

  /// Save the state of the stack when the last save point was hit
  ar & savepointCallStack;
}

// -----------------------------------------------------------------------

template<class Archive>
void RLMachine::load(Archive & ar, unsigned int version)
{
  ar & m_line;

  // Just thaw the callStack; all preprocessing was done at freeze
  // time.
  callStack.clear();
  ar & callStack;
}

// -----------------------------------------------------------------------


// Explicit instantiations for text archives (since we hide the
// implementation)

template void RLMachine::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void RLMachine::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
