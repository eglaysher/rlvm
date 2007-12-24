// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

/* tut.cpp: main() for all unit testing.  *
 */
 
#include "tut.hpp"
#include "tut_reporter.hpp"

namespace tut
{
test_runner_singleton runner;
};

int main(int argc,const char* argv[])
{
	tut::reporter visi;

  if( argc < 2 || argc > 4 )
  {
  }
  
  tut::runner.get().set_callback(&visi);

  try
  {
    if( argc == 1 || (argc == 2 && std::string(argv[1]) == "regression") )
    {
      tut::runner.get().run_tests();
    }
    else if(argc >= 4 || (argc == 2 && std::string(argv[1]) == "help" ))
    {
      std::cout << "TUT example test application." << std::endl;
      std::cout << "Usage: example [regression] | [list] | [ group] [test]" 
                << std::endl;
      std::cout << "       List all groups: example list" << std::endl;
      std::cout << "       Run all tests: example regression" << std::endl;
      std::cout << "       Run one group: example std::auto_ptr" << std::endl;
      std::cout << "       Run one test: example std::auto_ptr 3" << std::endl;;	
    }
    else if( argc == 2 && std::string(argv[1]) == "list" )
    {
      std::cout << "registered test groups:" << std::endl;
      tut::groupnames gl = tut::runner.get().list_groups();
      tut::groupnames::const_iterator i = gl.begin();
      tut::groupnames::const_iterator e = gl.end();
      while( i != e )
      {
        std::cout << "  " << *i << std::endl;
        ++i;
      }
    }
    else if( argc == 2 && std::string(argv[1]) != "regression" )
    {
      tut::runner.get().run_tests(argv[1]);
    }
    else if( argc == 3 )
    {
      tut::runner.get().run_test(argv[1],::atoi(argv[2]));
    }
  }
  catch( const std::exception& ex )
  {
    std::cerr << "tut raised ex: " << ex.what() << std::endl;
  }

  return 0;
}
