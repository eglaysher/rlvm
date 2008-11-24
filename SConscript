# Main scons file

import os
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
  ],

  CPPFLAGS = [
    "-g",
    "-O0"
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

def build_rlc_simple(target, source, env, for_signature):
  target_build_directory = os.path.splitext(target[0].path)[0]
  intermediate = target_build_directory + "/seen0001.TXT"
  return env.Command(target, source,
                     Action([Mkdir(target_build_directory),
                             "rlc ${SOURCES[0]} -o seen0001 -d " + target_build_directory,
                             "kprl -a ${TARGETS[0]} " + intermediate,
                             Delete(target_build_directory)],
                            "Building ${TARGET}..."))

def build_rlc_complex(target, source, env, for_signature):
  '''Builds all SEENXXXX.ke files in the source directory into a resulting .TXT
  file.

   TODO: I'm not sure this works with dependencies correctly.
   source_scanner=DirScanner should work, but If I ever modify any of these
   test cases' source files and things don't appear to be rebuilding properly
   THIS IS THE FIRST PLACE TO LOOK!'''

  target_build_directory = os.path.splitext(target[0].path)[0]

  commands = [
    Mkdir(target_build_directory)
  ]

  source_file_names = os.listdir(source[0].srcnode().path)
  for file_name in source_file_names:
    if os.path.splitext(file_name)[1] == ".ke":
      input_path = os.path.join(source[0].srcnode().path, file_name)
      base_name = os.path.splitext(os.path.basename(file_name))[0]
      commands += ["rlc " + input_path + " -o " + base_name + " -d " +
                   target_build_directory]
  commands += [
     "kprl -a ${TARGETS[0]} " + target_build_directory + "/*",
     Delete(target_build_directory)
  ]

  return env.Command(target, source, Action(commands, "Building ${TARGET}..."))

builder_rlc = Builder(generator = build_rlc_simple,
                      suffix = ".TXT",
                      src_suffix = ".ke")
builder_rlc_complex = Builder(generator = build_rlc_complex,
                              suffix = ".TXT",
                              source_scanner=DirScanner,
                              source_factory=Dir)

test_env = env.Clone()
test_env.Append(CPPPATH = ["#/test"],
                BUILDERS = {'RlcSimple' : builder_rlc,
                            'RlcComplex' : builder_rlc_complex})

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

simple_test_cases = [
  'ExpressionTest_SEEN/basicOperators',
  'ExpressionTest_SEEN/comparisonOperators',
  'ExpressionTest_SEEN/logicalOperators',
  'ExpressionTest_SEEN/previousErrors',
  'Module_Jmp_SEEN/fibonacci',
  'Module_Jmp_SEEN/gosub_0',
  'Module_Jmp_SEEN/gosub_case_0',
  'Module_Jmp_SEEN/gosub_if_0',
  'Module_Jmp_SEEN/gosub_unless_0',
  'Module_Jmp_SEEN/gosub_with_0',
  'Module_Jmp_SEEN/goto_0',
  'Module_Jmp_SEEN/goto_case_0',
  'Module_Jmp_SEEN/goto_if_0',
  'Module_Jmp_SEEN/goto_on_0',
  'Module_Jmp_SEEN/goto_unless_0',
  'Module_Jmp_SEEN/graphics',
  'Module_Jmp_SEEN/graphics2',
  'Module_Jmp_SEEN/jump_0',
  "Module_Mem_SEEN/cpyrng_0",
  "Module_Mem_SEEN/cpyvars_0",
  "Module_Mem_SEEN/setarray_0",
  "Module_Mem_SEEN/setarray_stepped_0",
  "Module_Mem_SEEN/setrng_0",
  "Module_Mem_SEEN/setrng_1",
  "Module_Mem_SEEN/setrng_stepped_0",
  "Module_Mem_SEEN/setrng_stepped_1",
  "Module_Mem_SEEN/sum_0",
  "Module_Mem_SEEN/sums_0",
  "Module_Str_SEEN/atoi_0",
  "Module_Str_SEEN/digit_0",
  "Module_Str_SEEN/digits_0",
  "Module_Str_SEEN/hantozen_0",
  "Module_Str_SEEN/hantozen_1",
  "Module_Str_SEEN/itoa_0",
  "Module_Str_SEEN/itoa_s_0",
  "Module_Str_SEEN/itoa_w_0",
  "Module_Str_SEEN/itoa_ws_0",
  "Module_Str_SEEN/lowercase_0",
  "Module_Str_SEEN/lowercase_1",
  "Module_Str_SEEN/strcat_0",
  "Module_Str_SEEN/strcharlen_0",
  "Module_Str_SEEN/strcharlen_1",
  "Module_Str_SEEN/strclear_0",
  "Module_Str_SEEN/strclear_1",
  "Module_Str_SEEN/strcmp_0",
  "Module_Str_SEEN/strcpy_0",
  "Module_Str_SEEN/strcpy_1",
  "Module_Str_SEEN/strlen_0",
  "Module_Str_SEEN/strlpos_0",
  "Module_Str_SEEN/strpos_0",
  "Module_Str_SEEN/strrsub_0",
  "Module_Str_SEEN/strrsub_1",
  "Module_Str_SEEN/strsub_0",
  "Module_Str_SEEN/strsub_1",
  "Module_Str_SEEN/strsub_2",
  "Module_Str_SEEN/strsub_3",
  "Module_Str_SEEN/strtrunc_0",
  "Module_Str_SEEN/strtrunc_1",
  "Module_Str_SEEN/strused_0",
  "Module_Str_SEEN/uppercase_0",
  "Module_Str_SEEN/uppercase_1",
  "Module_Str_SEEN/zentohan_0",
  "Module_Str_SEEN/zentohan_1",
  "Module_Sys_SEEN/builtins",
  "Module_Sys_SEEN/dumb",
]

for test_case in simple_test_cases:
  test_env.RlcSimple('test/' + test_case)

complex_test_cases = [
  "Module_Jmp_SEEN/farcallTest_0",
  "Module_Jmp_SEEN/farcall_withTest",
  "Module_Jmp_SEEN/jumpTest",
  "Module_Sys_SEEN/SceneNum"
]

for test_case in complex_test_cases:
  test_env.RlcComplex('test/' + test_case)

files_to_copy = [
  "Gameexe_data/Gameexe.ini",
  "Gameroot/g00/doesntmatter.g00"
]

for file_name in files_to_copy:
  test_env.Command('#/build/test/' + file_name,
                   '#/test/' + file_name,
                   Copy('$TARGET', '$SOURCE'))

#########################################################################

test_env.Program("luaRlvm", ['test/luaRlvm.cpp', 
                             script_machine_files, 'luabind/libluabind.a', 
                             'libsystem_sdl.a', 'librlvm.a'])
