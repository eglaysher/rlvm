#!/usr/bin/python

"""
Support for building rlvm objects
"""

import os

def _MakeStaticName(lib_name):
  return 'STATIC_' + lib_name + "_LIBS"

def RlvmLibrary(env, lib_name, *args, **kwargs):
  """
  Psuedo-builder for rlvm's internal libraries to speed up development.
  """
#   if env['BUILD_TYPE'] == "Debug":
#     return env.SharedLibrary(lib_name, args)
#   else:
  return env.StaticLibrary(lib_name, *args, **kwargs)


def RlvmProgram(env, prog_name, *args, **kwargs):
  """Psuedo-builder for rlvm programs to handle special static linking on per
  platform basis.

  RlvmProgram should be invoked per this example:

    env.RlvmProgram("rlvm", [rlvm objects files], use_lib_set = ["SDL", "LUA"],
                    rlvm_libs = ['guichan_platform', 'rlvm'])

  This will build the 'rlvm' binary with component libs libguichan_platform and
  librlvm while including all the static libraries necessary for SDL and for
  LUA.

  Environment Variables Responded to:
  - FULL_STATIC_BUILD: Checks to see if this is set and will attempt to link
    the static

  - STATIC_{string in set use_lib_set}_LIBS: Passing a string in the
    use_lib_set will add the static libraries in that set to

  - BUILD_TYPE: In "Debug" mode, all the values passed into RLVM_LIBS are
    assumed to be

  Returns:
    Output node list from env.Program().
  """
  # Need to be smarter about cloning
  cloned_env = env.Clone()

  # All the objects to link into the program. Note the order by which we add
  # all these
  objects = [ ]

  # Add the actual program files
  objects.extend(args)

  # TODO: Deal with RLVM_LIBS in a shared objecty way
  if kwargs.has_key('rlvm_libs'):
    for lib_name in kwargs['rlvm_libs']:
      objects.append(env['LIBPREFIX'] + lib_name + env['LIBSUFFIX'])

  # Add all static libraries from the various categories
  if kwargs.has_key('use_lib_set'):
    for lib_set_name in kwargs['use_lib_set']:
      lib_set = cloned_env[_MakeStaticName(lib_set_name)]
      if lib_set:
        objects.extend(lib_set)

  # First, we need to see if this is a static build
  if kwargs.has_key("full_static_build") and kwargs['full_static_build'] == True:
    # We must unpack each entry in LIBS and try to locate a static library to
    old_libs = cloned_env['LIBS']
    libpaths = cloned_env['LIBPATH']
    no_static_library_libs = [ ]

    for lib in old_libs:
      found = False

      if isinstance(lib, str):
        for libpath in libpaths:
          absolute_path = libpath + "/" + env['LIBPREFIX'] + lib + env['LIBSUFFIX']
          if os.path.exists(absolute_path):
            objects.append(absolute_path)
            found = True
            break

      if found == False:
        no_static_library_libs.append(lib)

    # Put libraries with no static version back into the path
    cloned_env['LIBS'] = no_static_library_libs

  out_nodes = cloned_env.Program(prog_name, objects)

  return out_nodes

def AddStaticLibraryTo(env, name, type):
  """
  Adds a library to one of the internal STATIC_name_LIBS tables.
  """
  path = env['LIBRARY_DIR'] + "/" + env['LIBPREFIX'] + name + env['LIBSUFFIX']
  env[_MakeStaticName(type)].insert(0, path)


def BuildSubcomponent(env, component_name):
  """
  Builds the subcomponent |name| and puts the resultant libraries in
  build/libraries/|name| and exposes that subcomponent's headers to |env|.
  """
  component_env = env.Clone()
  component_env.Append(
    CPPFLAGS = [
      "-Os"
    ]
  )

  component_env.SConscript("vendor/" + component_name + "/SConscript",
                           variant_dir="build/libraries/" + component_name,
                           duplicate=0,
                           exports=["component_env", "env"])

  # Make sure the main compilation can see the includes to these files
  env.Append(CPPPATH = [ "#vendor/" + component_name + "/include/" ])


def generate(env, **kw):
  env.Append(
    # A list of absolute paths to static sdl libraries to make things that need
    # SDL work.
    STATIC_SDL2_LIBS = [ ],

    # A list of absolute paths to static lua objects to make that subsystem
    # work.
    STATIC_LUA_LIBS = [ ],

    # A list of absolute paths to static google testing libraries.
    STATIC_TEST_LIBS = [ ]
  )

  env.AddMethod(RlvmLibrary)
  env.AddMethod(RlvmProgram)
  env.AddMethod(AddStaticLibraryTo)
  env.AddMethod(BuildSubcomponent)

def exists(env):
  return True
