# Main scons file

import shutil
import sys

Import('env')

########################################################## [ Root environment ]
env.Append(
  LIBS = [
    "GL",
    "GLU",
    "SDL_image",
    "SDL_mixer",
  ],

  CPPDEFINES = [
    "CASE_SENSITIVE_FILESYSTEM",
    "_THREAD_SAFE"
  ]
)

env.ParseConfig("sdl-config --cflags --libs")
env.ParseConfig("freetype-config --cflags --libs")

librlvm_files = [
  "src/MachineBase/reference.cpp",
  "src/MachineBase/LongOperation.cpp",
  "src/MachineBase/RLMachine.cpp",
  "src/MachineBase/RLMachine_intmem.cpp",
  "src/MachineBase/RLModule.cpp",
  "src/MachineBase/MappedRLModule.cpp",
  "src/MachineBase/RLOperation.cpp",
  "src/MachineBase/SerializationGlobal.cpp",
  "src/MachineBase/SerializationLocal.cpp",
  "src/MachineBase/Memory.cpp",
  "src/MachineBase/StackFrame.cpp",
  "src/MachineBase/SaveGameHeader.cpp",
  "src/MachineBase/GeneralOperations.cpp",
  "src/MachineBase/OpcodeLog.cpp",
  "src/Modules/Modules.cpp",
  "src/Modules/Module_EventLoop.cpp",
  "src/Modules/Module_Mov.cpp",
  "src/Modules/Module_Bgm.cpp",
  "src/Modules/Module_Pcm.cpp",
  "src/Modules/Module_Se.cpp",
  "src/Modules/Module_Koe.cpp",
  "src/Modules/Module_Grp.cpp",
  "src/Modules/Module_Jmp.cpp",
  "src/Modules/Module_Mem.cpp",
  "src/Modules/Module_Msg.cpp",
  "src/Modules/Module_Os.cpp",
  "src/Modules/Module_Scr.cpp",
  "src/Modules/Module_Sel.cpp",
  "src/Modules/Module_Str.cpp",
  "src/Modules/Module_Sys.cpp",
  "src/Modules/Module_Sys_Frame.cpp",
  "src/Modules/Module_Sys_Timer.cpp",
  "src/Modules/Module_Sys_Save.cpp",
  "src/Modules/Module_Sys_Syscom.cpp",
  "src/Modules/Module_Sys_Date.cpp",
  "src/Modules/Module_Sys_Name.cpp",
  "src/Modules/Module_Sys_index_series.cpp",
  "src/Modules/Module_Sys_Wait.cpp",
  "src/Modules/Module_Gan.cpp",
  "src/Modules/cp932toUnicode.cpp",
  "src/Modules/Module_Obj.cpp",
  "src/Modules/Module_ObjFgBg.cpp",
  "src/Modules/Module_ObjCreation.cpp",
  "src/Modules/Module_ObjManagement.cpp",
  "src/Modules/Module_ObjPosDims.cpp",
  "src/Modules/Module_Refresh.cpp",
  "src/Modules/Module_Debug.cpp",
  "src/Modules/Module_Shl.cpp",
  "src/Modules/Module_Shk.cpp",
  "src/Modules/TextoutLongOperation.cpp",
  "src/Modules/PauseLongOperation.cpp",
  "src/Modules/ZoomLongOperation.cpp",
  "src/Effects/Effect.cpp",
  "src/Effects/FadeEffect.cpp",
  "src/Effects/WipeEffect.cpp",
  "src/Effects/ScrollOnScrollOff.cpp",
  "src/Effects/BlindEffect.cpp",
  "src/Effects/EffectFactory.cpp",
  "src/Systems/Base/System.cpp",
  "src/Systems/Base/GraphicsSystem.cpp",
  "src/Systems/Base/GraphicsObject.cpp",
  "src/Systems/Base/GraphicsObjectData.cpp",
  "src/Systems/Base/GraphicsObjectOfFile.cpp",
  "src/Systems/Base/GraphicsTextObject.cpp",
  "src/Systems/Base/GraphicsStackFrame.cpp",
  "src/Systems/Base/Surface.cpp",
  "src/Systems/Base/Rect.cpp",
  "src/Systems/Base/Colour.cpp",
  "src/Systems/Base/ObjectSettings.cpp",
  "src/Systems/Base/FrameCounter.cpp",
  "src/Systems/Base/EventSystem.cpp",
  "src/Systems/Base/EventHandler.cpp",
  "src/Systems/Base/RLTimer.cpp",
  "src/Systems/Base/SystemError.cpp",
  "src/Systems/Base/TextWindow.cpp",
  "src/Systems/Base/TextWindowButton.cpp",
  "src/Systems/Base/TextPage.cpp",
  "src/Systems/Base/TextSystem.cpp",
  "src/Systems/Base/TextKeyCursor.cpp",
  "src/Systems/Base/GanGraphicsObjectData.cpp",
  "src/Systems/Base/AnmGraphicsObjectData.cpp",
  "src/Systems/Base/SelectionElement.cpp",
  "src/Systems/Base/SoundSystem.cpp",
  "src/Systems/Base/MouseCursor.cpp",
  "src/Systems/Base/CGMTable.cpp",
  "src/libReallive/archive.cpp",
  "src/libReallive/bytecode.cpp",
  "src/libReallive/compression.cpp",
  "src/libReallive/expression.cpp",
  "src/libReallive/filemap.cpp",
  "src/libReallive/gameexe.cpp",
  "src/libReallive/scenario.cpp",
  "src/libReallive/intmemref.cpp",
  "src/Utilities.cpp",
  "src/Utilities/dateUtil.cpp",
  "src/Utilities/findFontFile.cpp",
  "src/Utilities/StringUtilities.cpp",
  "vendor/file.cc",
  "vendor/endian.cpp",
  "vendor/wavfile.cc",
  "vendor/nwatowav.cc"
]

env.StaticLibrary('rlvm', librlvm_files)

libsystemsdl_files = [
  "src/Systems/SDL/SDLAudioLocker.cpp",
  "src/Systems/SDL/SDLSystem.cpp",
  "src/Systems/SDL/SDLGraphicsSystem.cpp",
  "src/Systems/SDL/SDLSurface.cpp",
  "src/Systems/SDL/SDLSoundSystem.cpp",
  "src/Systems/SDL/SDLSoundChunk.cpp",
  "src/Systems/SDL/SDLMusic.cpp",
  "src/Systems/SDL/SDLEventSystem.cpp",
  "src/Systems/SDL/SDLUtils.cpp",
  "src/Systems/SDL/SDLRenderToTextureSurface.cpp",
  "src/Systems/SDL/SDLTextSystem.cpp",
  "src/Systems/SDL/SDLTextWindow.cpp",
  "src/Systems/SDL/Texture.cpp",
  "vendor/glew.c",
  "vendor/alphablit.cc",
  "vendor/SDL_ttf.c"
]

env.StaticLibrary('system_sdl', libsystemsdl_files)

env.Program('rlvm', ["src/rlvm.cpp", 'libsystem_sdl.a', 'librlvm.a'])

#########################################################################

test_env = env.Clone()
test_env.Append(CPPPATH = ["#/test"])

test_case_files = [
  "test/testUtils.cpp",
  "test/RLMachine_TUT.cpp",
  "test/Module_Jmp_TUT.cpp",
  "test/Module_Str_TUT.cpp",
  "test/Module_Mem_TUT.cpp",
  "test/Module_Sys_TUT.cpp",
  "test/ExpressionTest_TUT.cpp",
  "test/TextSystem_TUT.cpp",
  "test/Gameexe_TUT.cpp",
  "test/LazyArray_TUT.cpp",
  "test/GraphicsObject_TUT.cpp",
  "test/Effect_TUT.cpp"
]

null_system_files = [
  "test/NullSystem/NullSystem.cpp",
  "test/NullSystem/NullEventSystem.cpp",
  "test/NullSystem/NullTextSystem.cpp",
  "test/NullSystem/NullTextWindow.cpp",
  "test/NullSystem/NullGraphicsSystem.cpp",
  "test/NullSystem/NullSoundSystem.cpp",
  "test/NullSystem/NullSurface.cpp",
  "test/NullSystem/MockLog.cpp"
]

script_machine_files = [
  "test/ScriptMachine/ScriptWorld.cpp",
  "test/ScriptMachine/ScriptMachine.cpp",
  "test/ScriptMachine/luabind_Machine.cpp",
  "test/ScriptMachine/luabind_System.cpp",
  "test/ScriptMachine/luabind_EventSystem.cpp",
  "test/ScriptMachine/luabind_GraphicsSystem.cpp",
  "test/ScriptMachine/luabind_GraphicsObject.cpp",
  "test/ScriptMachine/luabind_utility.cpp"
]

test_env.Program('rlvmTests', ["test/rlvmTest.cpp", null_system_files,
                               test_case_files, 'librlvm.a'])
