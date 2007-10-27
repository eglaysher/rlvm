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

// void RLMachine::saveIntegerBanksTo(const IntegerBank_t& banks, 
// 								   Json::Value& root)
// {
//   for(IntegerBank_t::const_iterator it = banks.begin(); it != banks.end(); 
// 	  ++it)
//   {
// 	Json::Value intArray;
// 	transform(intVar[it->first], intVar[it->first] + 2000,
// 			  back_inserter(intArray),
// 			  buildFromInt);

// 	ostringstream oss;
// 	oss << "int" << it->second;
// 	root[oss.str()] = intArray;
//   }
// }

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
  callStack.back().saveGameFrame = true;
  callStack.back().savePoint = callStack.back().ip;
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

// void RLMachine::loadIntegerBanksFrom(const IntegerBank_t& banks, 
// 									 Json::Value& root)
// {
//   for(IntegerBank_t::const_iterator it = banks.begin(); it != banks.end(); 
// 	  ++it)
//   {
// 	ostringstream oss;
// 	oss << "int" << it->second;
// 	const Json::Value& intMem = root[oss.str()];

// 	transform(intMem.begin(), intMem.end(), 
// 			  intVar[it->first], 
// 			  bind(&Json::Value::asInt, _1));
//               }
// }

// -----------------------------------------------------------------------

// void RLMachine::loadGlobalMemory()
// {
//   fs::path home = m_system.gameSaveDirectory() / "global.sav";
//   fs::ifstream file(home);

//   // If we were able to open the file for reading, load it. Don't
//   // complain if we're unable to, since this may be the first run on
//   // this certain game and it may not exist yet.
//   if(file)
//   {
//     Serialization::loadGlobalMemoryFrom(file, *this);
//   }
// }

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

//  saveIntegerBanksTo(LOCAL_INTEGER_BANKS, root);
//  saveStringBank(strS, 'S', root);

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

  cerr << "Stack: ";
  Json::Value saveCallStack(Json::arrayValue);
  for(vector<StackFrame>::const_iterator it = callStack.begin();
      it != callStack.end(); ++it)
  {
    if(it->frameType == StackFrame::TYPE_LONGOP)
    {
      break;
    }
    else
    {
      Json::Value frame(Json::arrayValue);

      int position = 0;
      vector<StackFrame>::const_iterator next = boost::next(it);

      if(it->saveGameFrame && 
         (next == callStack.end() || next->frameType == StackFrame::TYPE_LONGOP))
      {
        cerr << "SP";
        position = distance(it->scenario->begin(), it->savePoint);
      }
      else
      {
        cerr << "N";
        position = distance(it->scenario->begin(), it->ip);
      }

      cerr << position << "(" << it->scenario << "), ";

      frame.push_back(Json::Value(it->scenario->sceneNumber()));
      frame.push_back(Json::Value(position));
      frame.push_back(Json::Value(it->frameType));

      saveCallStack.push_back(frame);
    }
  }

  cerr << endl;

  root["callStack"] = saveCallStack;

  m_system.saveGameValues(root);

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

//  loadIntegerBanksFrom(LOCAL_INTEGER_BANKS, root);
//  loadStringBank(strS, 'S', root);

  clearCallstack();
  const Value saveCallStack = root["callStack"];
  for(ValueConstIterator it = saveCallStack.begin(); 
      it != saveCallStack.end(); ++it)
  {
    if((*it).type() == arrayValue)
    {
      int scenarioNum = (*it)[0u].asInt();
      int offset = (*it)[1u].asInt();
      int type = (*it)[2u].asInt();

      libReallive::Scenario const* scenario = m_archive.scenario(scenarioNum);
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

  m_system.loadGameValues(*this, root);
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

  callStack.back().scenario = scenario;
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

void RLMachine::clearCallstack() 
{
  while(callStack.size())
    popStackFrame();
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
