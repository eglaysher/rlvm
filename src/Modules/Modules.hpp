#ifndef __Modules_hpp__
#define __Modules_hpp__

/** 
 * Convenience function to add all known module to a certain machine;
 * This keeps us from having to recompile rlvm.cpp all the time.
 * 
 * @param machine 
 */
void addAllModules(RLMachine& machine);

#endif 
