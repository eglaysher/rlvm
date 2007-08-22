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
//
// The parts that were stolen from xclannad are:
//
// Copyright (c) 2004-2006  Kazunori "jagarl" Ueno
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#include "libReallive/archive.h"
#include "libReallive/bytecode.h"
#include "libReallive/scenario.h"
#include "libReallive/expression.h"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/LongOperation.hpp"

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

#include <string>
#include <sstream>
#include <iostream>
#include <iterator>

#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/assign.hpp>

#include "json/value.h"
#include "json/reader.h"
#include "json/writer.h"

namespace fs = boost::filesystem;

using namespace std;
using namespace libReallive;

using boost::bind;
using boost::assign::list_of;

const IntegerBank_t LOCAL_INTEGER_BANKS =
  list_of(make_pair(INTB_LOCATION, 'A'))
  (make_pair(INTB_LOCATION, 'B'))
  (make_pair(INTC_LOCATION, 'C'))
  (make_pair(INTD_LOCATION, 'D'))
  (make_pair(INTE_LOCATION, 'E'))
  (make_pair(INTF_LOCATION, 'F'));

const IntegerBank_t GLOBAL_INTEGER_BANKS =
  list_of(make_pair(INTG_LOCATION, 'G'))
  (make_pair(INTZ_LOCATION, 'Z'));


// -----------------------------------------------------------------------
// Stack Frame
// -----------------------------------------------------------------------

/**
 * Internally used type that represents a stack frame in RLMachine's
 * call stack.
 */
struct RLMachine::StackFrame {
  /// The scenario in the SEEN file for this stack frame.
  libReallive::Scenario const* scenario;
    
  /// The instruction pointer in the stack frame.
  libReallive::Scenario::const_iterator ip;

  /// Pointer to the owned LongOperation if this is of TYPE_LONGOP.
  boost::shared_ptr<LongOperation> longOp;

  /**
   * The function that pushed the current frame onto the
   * stack. Used in error checking.
   */
  enum FrameType {
    TYPE_ROOT,    /**< Added by the Machine's constructor */
    TYPE_GOSUB,   /**< Added by a call by gosub */
    TYPE_FARCALL, /**< Added by a call by farcall */
    TYPE_LONGOP   /**< Added by pushLongOperation() */
  } frameType;

  /// Default constructor
  StackFrame(libReallive::Scenario const* s,
             const libReallive::Scenario::const_iterator& i,
             FrameType t) 
    : scenario(s), ip(i), frameType(t) {}

  StackFrame(libReallive::Scenario const* s,
             const libReallive::Scenario::const_iterator& i,
             LongOperation* op)
    : scenario(s), ip(i), longOp(op), frameType(TYPE_LONGOP) {}
};

// -----------------------------------------------------------------------
// RLMachine
// -----------------------------------------------------------------------

RLMachine::RLMachine(System& inSystem, Archive& inArchive) 
  : m_halted(false), m_haltOnException(true), archive(inArchive), 
    m_system(inSystem), m_markSavepoints(false)
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
    scenario = inArchive.scenario(archive.begin()->first);
  }

  if(scenario == 0)
    throw rlvm::Exception("Invalid scenario file");
  pushStackFrame(StackFrame(scenario, scenario->begin(), StackFrame::TYPE_ROOT));

  // Initialize the big memory block to zero
  memset(intVar, 0, sizeof(intVar));

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

void RLMachine::saveGlobalMemory()
{
  fs::path home = m_system.gameSaveDirectory() / "global.jsn";
  fs::ofstream file(home);
  if(!file)
  {
	ostringstream oss;
	oss << "Could not open global memory file.";
	throw rlvm::Exception(oss.str());
  }

  saveGlobalMemoryTo(file);
}

// -----------------------------------------------------------------------

namespace {

Json::Value buildFromString(const std::string& str)
{
  return Json::Value(str);
}

// -----------------------------------------------------------------------

Json::Value buildFromInt(const int val)
{
  return Json::Value(val);
}

}

// -----------------------------------------------------------------------

string RLMachine::makeSaveGameName(int slot)
{
  ostringstream oss;
  oss << "save" << setw(3) << setfill('0') << slot << ".jsn";
  return oss.str();
}

// -----------------------------------------------------------------------

void RLMachine::saveIntegerBanksTo(const IntegerBank_t& banks, 
								   Json::Value& root)
{
  for(IntegerBank_t::const_iterator it = banks.begin(); it != banks.end(); 
	  ++it)
  {
	Json::Value intArray;
	transform(intVar[it->first], intVar[it->first] + 2000,
			  back_inserter(intArray),
			  buildFromInt);

	ostringstream oss;
	oss << "int" << it->second;
	root[oss.str()] = intArray;
  }
}

// -----------------------------------------------------------------------

void RLMachine::saveStringBank(const std::string* strPtr,
							   char bankName, 
							   Json::Value& root)
{
  ostringstream oss;
  oss << "str" << bankName;

  Json::Value strBank(Json::arrayValue);
  transform(strPtr, strPtr + 2000, back_inserter(strBank),
			buildFromString);
  root[oss.str()] = strBank;
}

// -----------------------------------------------------------------------

void RLMachine::loadStringBank(std::string* strPtr,
							   char bankName, 
							   Json::Value& root)
{
  ostringstream oss;
  oss << "str" << bankName;

  if(!root.isMember(oss.str()))
  {
	ostringstream err;
	err << "No " << oss.str() << " memory bank in global file!";
	throw rlvm::Exception(err.str());
  }

  const Json::Value& strBank = root[oss.str()];
  if(strBank.size() != 2000)
  {
	ostringstream err;
	err << oss.str() << " memory bank.size() != 2000";
	throw rlvm::Exception(err.str());
  }

  transform(strBank.begin(), strBank.end(),
			strPtr, bind(&Json::Value::asString, _1));
}

// -----------------------------------------------------------------------

void RLMachine::markSavepoint()
{
  scenarioSavePoint = callStack.back().scenario;
  savePoint = callStack.back().ip;
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
  return savepointDecide(&Scenario::savepointMessage, "");
}

// -----------------------------------------------------------------------

bool RLMachine::shouldSetSelcomSavepoint() const
{
  return savepointDecide(&Scenario::savepointSelcom, "");
}

// -----------------------------------------------------------------------

bool RLMachine::shouldSetSeentopSavepoint() const
{
  return savepointDecide(&Scenario::savepointSeentop, "");
}

// -----------------------------------------------------------------------

void RLMachine::loadIntegerBanksFrom(const IntegerBank_t& banks, 
									 Json::Value& root)
{
  for(IntegerBank_t::const_iterator it = banks.begin(); it != banks.end(); 
	  ++it)
  {
	ostringstream oss;
	oss << "int" << it->second;
	const Json::Value& intMem = root[oss.str()];

	transform(intMem.begin(), intMem.end(), 
			  intVar[it->first], 
			  bind(&Json::Value::asInt, _1));
  }
}

// -----------------------------------------------------------------------

void RLMachine::saveGlobalMemoryTo(std::ostream& ofs)
{
  Json::Value root(Json::objectValue);
  saveStringBank(strM, 'M', root);
  saveIntegerBanksTo(GLOBAL_INTEGER_BANKS, root);

  m_system.saveGlobals(root);

  Json::StyledWriter writer;
  ofs << writer.write( root );
}

// -----------------------------------------------------------------------

void RLMachine::loadGlobalMemory()
{
  fs::path home = m_system.gameSaveDirectory() / "global.jsn";
  fs::ifstream file(home);

  // If we were able to open the file for reading, load it. Don't
  // complain if we're unable to, since this may be the first run on
  // this certain game and it may not exist yet.
  if(file)
  {
	loadGlobalMemoryFrom(file);
  }
}

// -----------------------------------------------------------------------

void RLMachine::loadGlobalMemoryFrom(std::istream& iss)
{
  string memoryContents;
  string line;
  while(getline(iss, line))
  {
	memoryContents += line;
	memoryContents += "\n";
  }

  Json::Value root;
  Json::Reader reader;
  if(!reader.parse(memoryContents, root))
  {
	ostringstream oss;
	oss << "Failed to read global memory file for game \""
		<< m_system.gameexe()("REGNAME").to_string() << "\": "
		<< reader.getFormatedErrorMessages();

	throw rlvm::Exception(oss.str());
  }

  loadStringBank(strM, 'M', root);
  loadIntegerBanksFrom(GLOBAL_INTEGER_BANKS, root);

  m_system.loadGlobals(root);  
}

// -----------------------------------------------------------------------

void RLMachine::saveGame(int slot)
{
  fs::path home = m_system.gameSaveDirectory() / makeSaveGameName(slot);
  fs::ofstream file(home);
  if(!file)
  {
	ostringstream oss;
	oss << "Could not open save game file " << home.string();
	throw rlvm::Exception(oss.str());
  }

  saveGameTo(file);
}

// -----------------------------------------------------------------------

void RLMachine::saveGameTo(std::ostream& ofs)
{
  Json::Value root(Json::objectValue);

  saveIntegerBanksTo(LOCAL_INTEGER_BANKS, root);
  saveStringBank(strS, 'S', root);

  root["title"] = system().graphics().windowSubtitle();

  // Build the save game time
  Json::Value saveTime(Json::arrayValue);
  using namespace datetime;
  saveTime.append(Json::Value(getYear()));
  saveTime.append(Json::Value(getMonth()));
  saveTime.append(Json::Value(getDay()));
  saveTime.append(Json::Value(getDayOfWeek()));
  saveTime.append(Json::Value(getHour()));
  saveTime.append(Json::Value(getMinute()));
  saveTime.append(Json::Value(getSecond()));
  saveTime.append(Json::Value(getMs()));
  root["saveTime"] = saveTime;

  Json::Value saveCallStack(Json::arrayValue);
  for(vector<StackFrame>::const_iterator it = callStack.begin();
      it != callStack.end(); ++it)
  {
    if(it->frameType == StackFrame::TYPE_LONGOP)
    {
      saveCallStack.push_back(Json::Value("PauseLongOperation"));
      break;
    }
    else
    {
      Json::Value frame(Json::arrayValue);

      int position = 0;
      if(it->scenario == scenarioSavePoint)
        position = distance(it->scenario->begin(), savePoint);
      else
        position = distance(it->scenario->begin(), it->ip);

      frame.push_back(Json::Value(it->scenario->sceneNumber()));
      frame.push_back(Json::Value(position));
      frame.push_back(Json::Value(it->frameType));

      saveCallStack.push_back(frame);
    }
  }

  root["callStack"] = saveCallStack;

  Json::StyledWriter writer;
  ofs << writer.write( root );
}

// -----------------------------------------------------------------------

void RLMachine::loadGame(const int slot)
{
  fs::path home = m_system.gameSaveDirectory() / makeSaveGameName(slot);
  fs::ifstream file(home);
  if(!file)
  {
	ostringstream oss;
	oss << "Could not open save game file " << home.string();
	throw rlvm::Exception(oss.str());
  }

  loadGameFrom(file);
}

// -----------------------------------------------------------------------

void RLMachine::loadGameFrom(std::istream& iss)
{
  using namespace Json;

  string memoryContents;
  string line;
  while(getline(iss, line))
  {
	memoryContents += line;
	memoryContents += "\n";
  }

  Value root;
  Reader reader;
  if(!reader.parse(memoryContents, root))
  {
	ostringstream oss;
	oss << "Failed to read saved game \": "
		<< reader.getFormatedErrorMessages();

	throw rlvm::Exception(oss.str());
  }

  loadIntegerBanksFrom(LOCAL_INTEGER_BANKS, root);
  loadStringBank(strS, 'S', root);

  system().graphics().setWindowSubtitle(root["title"].asString());

  const Value saveCallStack = root["callStack"];
  for(ValueConstIterator it = saveCallStack.begin(); 
      it != saveCallStack.end(); ++it)
  {
    if((*it).type() == stringValue)
    {
      if((*it).asString() == "PauseLongOperation")
        pushLongOperation(new PauseLongOperation(*this));
      else
        throw rlvm::Exception("Unknown string in callStack in save file!");
    }
    else if((*it).type() == arrayValue)
    {
      int scenarioNum = (*it)[0u].asInt();
      int offset = (*it)[1u].asInt();
      int type = (*it)[2u].asInt();

      libReallive::Scenario const* scenario = archive.scenario(scenarioNum);
      if(scenario == NULL)
      {
        ostringstream oss;
        oss << "Unknown SEEN #" << scenarioNum << " in save file!";
        throw rlvm::Exception(oss.str());
      }

      if(offset > distance(scenario->begin(), scenario->end()))
      {
        ostringstream oss;
        oss << offset << " is an illegal bytecode offset for SEEN #" 
            << scenarioNum << " in save file!";
        throw rlvm::Exception(oss.str());
      }

      Scenario::const_iterator positionIt = scenario->begin();
      advance(positionIt, offset);

      pushStackFrame(
        StackFrame(scenario, positionIt, StackFrame::FrameType(type)));
    }
    else
      throw rlvm::Exception("Unknown token in callStack in save file!");
  }

  // Reset the system
  // @todo Make this work.
//  system().reset();
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

      cout << "(SEEN" << callStack.back().scenario->sceneNumber() 
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
    if(it->ip == it->scenario->end())
      m_halted = true;
  }
}

// -----------------------------------------------------------------------

/** 
 * Helper function that throws errors for illegal memory access
 * 
 * @param location The illegal index that was accessed
 * @see RLMachine::getIntValue
 */
static void throwIllegalIndex(const IntMemRef& ref,
							  const std::string& function)
{
  ostringstream ss;
  ss << "Invalid memory access " << ref << " in " << function;
  throw rlvm::Exception(ss.str());
}

// -----------------------------------------------------------------------

/**
 *
 * @note This method was plagarized from xclannad.
 * @todo Does this allow for access like intL4[]? I don't think it does...
 */
int RLMachine::getIntValue(const IntMemRef& ref) 
{
  int type = ref.type();
  int index = ref.bank();
  int location = ref.location();

  if(index > NUMBER_OF_INT_LOCATIONS) 
      throwIllegalIndex(ref, "RLMachine::getIntValue()");

  if (type == 0) {
    // A[]..G[], Z[] を直に読む
    if (uint(location) >= 2000) 
      throwIllegalIndex(ref, "RLMachine::getIntValue()");

    return intVar[index][location];
  } else {
    // Ab[]..G4b[], Z8b[] などを読む
    int factor = 1 << (type - 1);
    int eltsize = 32 / factor;
    if (uint(location) >= (64000u / factor)) 
      throwIllegalIndex(ref, "RLMachine::getIntValue()");

    return (intVar[index][location / eltsize] >>
            ((location % eltsize) * factor)) & ((1 << factor) - 1);
  }
}

// -----------------------------------------------------------------------

/**
 *
 * @note This method was plagarized from xclannad.
 */
void RLMachine::setIntValue(const IntMemRef& ref, int value) 
{
  int type = ref.type();
  int index = ref.bank();
  int location = ref.location();

  if (index < 0 || index > NUMBER_OF_INT_LOCATIONS) {
	throwIllegalIndex(ref, "RLMachine::setIntValue()");
  }
  if (type == 0) {
    // A[]..G[], Z[] を直に書く
    if (uint(location) >= 2000) 
      throwIllegalIndex(ref, "RLMachine::setIntValue()");
    intVar[index][location] = value;
  } else {
    // Ab[]..G4b[], Z8b[] などを書く
    int factor = 1 << (type - 1);
    int eltsize = 32 / factor;
    int eltmask = (1 << factor) - 1;
    int shift = (location % eltsize) * factor;
    if (uint(location) >= (64000u / factor)) 
      throwIllegalIndex(ref, "RLMachine::setIntValue()");

    intVar[index][location / eltsize] =
      (intVar[index][location / eltsize] & ~(eltmask << shift))
      | (value & eltmask) << shift;
  }
}

// -----------------------------------------------------------------------

const std::string& RLMachine::getStringValue(int type, int location) 
{
  if(location > 1999)
      throw rlvm::Exception("Invalid range access in RLMachine::setStringValue");

  switch(type) {
  case STRK_LOCATION:
    if(location > 2)
      throw rlvm::Exception("Invalid range access on strK in RLMachine::setStringValue");
    return strK[location];
  case STRM_LOCATION: return strM[location];
  case STRS_LOCATION: return strS[location];
  default:
    throw rlvm::Exception("Invalid type in RLMachine::getStringValue");
  }
}

// -----------------------------------------------------------------------

void RLMachine::setStringValue(int type, int number, const std::string& value) {
  if(number > 1999)
      throw rlvm::Exception("Invalid range access in RLMachine::setStringValue");

  switch(type) {
  case STRK_LOCATION:
    if(number > 2)
      throw rlvm::Exception("Invalid range access on strK in RLMachine::setStringValue");
    strK[number] = value;
    break;
  case STRM_LOCATION:
    strM[number] = value;
    break;
  case STRS_LOCATION: 
    strS[number] = value;
    break;
  default:
    throw rlvm::Exception("Invalid type in RLMachine::setStringValue");
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
  libReallive::Scenario* scenario = archive.scenario(scenarioNum);
  if(scenario == 0)
    throw rlvm::Exception("Invalid scenario number in jump");

  callStack.back().scenario = scenario;
  callStack.back().ip = scenario->findEntrypoint(entrypoint);
}

// -----------------------------------------------------------------------

void RLMachine::farcall(int scenarioNum, int entrypoint) 
{
  libReallive::Scenario* scenario = archive.scenario(scenarioNum);
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
  pushStackFrame(StackFrame(callStack.back().scenario, newLocation, 
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
  pushStackFrame(StackFrame(callStack.back().scenario, callStack.back().ip,
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

int RLMachine::sceneNumber() const
{
  return callStack.back().scenario->sceneNumber();
}

// -----------------------------------------------------------------------

const Scenario& RLMachine::scenario() const
{
  return *callStack.back().scenario;
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
  return callStack.back().scenario->encoding();
}

// -----------------------------------------------------------------------

void RLMachine::performTextout(const TextoutElement& e)
{
  // Check to see if we mark savepoints on textout
  if(shouldSetMessageSavepoint())
    markSavepoint();

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
