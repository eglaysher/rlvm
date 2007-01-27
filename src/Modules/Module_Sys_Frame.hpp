#ifndef __Module_Sys_Frame_hpp__
#define __Module_Sys_Frame_hpp__

class RLModule;

/** 
 * Public access to the frame counter operations. Meant to be called
 * from the constructor of SysModule.
 * 
 * @see SysModule
 */
void addSysFrameOpcodes(RLModule& module);

#endif
