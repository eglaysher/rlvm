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
    std::cout << "Usage: example [regression] | [list] | [ group] [test]" << std::endl;
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
