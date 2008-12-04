
import os

############################################################# [ World options ]

AddOption('--release', action='store_true',
          help='Builds an optimized release for the platform.')

# Set libraries used by all configurations and all binaries in rlvm.
env = Environment(
  LIBS = [
    "boost_program_options",
    "boost_serialization",
    "boost_iostreams",
    "boost_filesystem",
    "boost_date_time",
    "boost_signals",
    "boost_thread-mt",
    "z"
  ],

  LOCAL_LIBS = [],

  CPPPATH = [
    "#/build",
    "#/src",
    "#/vendor",
  ],

  CPPFLAGS = [
    "--ansi",
    "-funsigned-char"
  ],

  CPPDEFINES = [
    "HAVE_CONFIG_H"
  ],

  # Where the final binaries should be put.
  OUTPUT_DIR = "#/build/",

  # Where under build/ to build.
  BUILD_DIR = "#/build/debug/",

  # Debug or Release.
  BUILD_TYPE = "Debug",

  # Whether we build the tests that require rlc to compile them.
  BUILD_RLC_TESTS = False,

  # Whether we build the test binary that requires lua.
  BUILD_LUA_TESTS = False
)

# Auto select the number of processors
if os.path.exists('/proc'):
  cpus = len([l for l in open('/proc/cpuinfo') if l.startswith('processor\t')])
else:
  cpus = 1
env.SetOption('num_jobs', cpus + 1)

# Use timestamps change, followed by MD5 for speed
env.Decider('MD5-timestamp')

if ARGUMENTS.get('VERBOSE') != '1':
  env['CCCOMSTR'] = 'Compiling $TARGET ...'
  env['CXXCOMSTR'] = 'Compiling $TARGET ...'
  env['SHCCCOMSTR'] = 'Compiling $TARGET ...'
  env['SHCXXCOMSTR'] = 'Compiling $TARGET ...'
  env['LINKCOMSTR'] = 'Linking $TARGET ...'
  env['SHLINKCOMSTR'] = 'Linking $TARGET ...'

############################################ [ Configuration (Think autoconf) ]

def CheckBoost(context, version):
  # Boost versions are in format major.minor.subminor
  v_arr = version.split(".")
  version_n = 0
  if len(v_arr) > 0:
    version_n += int(v_arr[0])*100000
    if len(v_arr) > 1:
      version_n += int(v_arr[1])*100
    if len(v_arr) > 2:
      version_n += int(v_arr[2])

  context.Message('Checking for Boost version >= %s... ' % (version))
  ret = context.TryRun("""
    #include <boost/version.hpp>

    int main()
    {
        return BOOST_VERSION >= %d ? 0 : 1;
    }
    """ % version_n, '.cpp')[0]
  context.Result(ret)
  return ret

def VerifyLibrary(config, library, header):
  if not config.CheckLibWithHeader(library, header, "cpp"):
    if config.CheckLib(library):
      print "You have " + library + " installed, but the development headers aren't installed."
    else:
      print "You need " + library + " to compile this program!"
    Exit(1)

config = env.Configure(custom_tests = { 'CheckBoost' : CheckBoost },
                       config_h="build/config.h")
if not config.CheckBoost('1.35'):
  print "Boost version >= 1.35 needed to compile rlvm!"
  Exit(1)

VerifyLibrary(config, 'SDL', 'SDL/SDL.h')
VerifyLibrary(config, 'SDL_image', 'SDL/SDL_image.h')
VerifyLibrary(config, 'SDL_mixer', 'SDL/SDL_mixer.h')
VerifyLibrary(config, 'ogg', 'ogg/ogg.h')
VerifyLibrary(config, 'vorbis', 'vorbis/vorbisfile.h')

# Building the luaRlvm test harness requires having lua installed;
if config.CheckLibWithHeader('lua5.1', 'lua5.1/lua.h', 'cpp'):
  env['BUILD_LUA_TESTS'] = True

# Really optional libraries that jagarl's file loaders take advantage of if on
# the system.
config.CheckLibWithHeader('png', 'png.h', "cpp")
config.CheckLibWithHeader('jpeg', 'jpeglib.h', "cpp")
config.CheckLibWithHeader('mad', 'mad.h', "cpp")

if config.TryAction('rlc --version'):
  env['BUILD_RLC_TESTS'] = True

env = config.Finish()

#########################################################################
## Building things
#########################################################################

if env['BUILD_RLC_TESTS'] == True:
  # Build the platform independent SEEN.TXT files.
  env.SConscript("test/SConscript.rlc",
                 build_dir="build/test",
                 duplicate=0,
                 exports='env')

# Hacky Temporary Differentiation between release and debug:
if GetOption('release'):
  env["BUILD_DIR"] = "#/build/release"
  env['BUILD_TYPE'] = "Release"

  # Now add release optimizations to the environment
  env.Append(
    CPPFLAGS = [
      "-O2"
    ]
  )
else:
  # Add debugging flags to all binaries here
  env.Append(
    CPPFLAGS = [
      "-g",
      "-O0"
    ]
  )


if env['BUILD_LUA_TESTS'] == True:
  # Only build our internal copy of luabind if there's a copy of lua on the
  # system.
  env.SConscript("vendor/luabind/SConscript",
                 build_dir="$BUILD_DIR/luabind",
                 duplicate=0,
                 exports='env')

# Need to generalize these so that maybe we use them if they're installed on
# the system. Then we could get rid of glew and SDL_ttf doing the same thing...
env.SConscript("vendor/guichan/SConscript",
               build_dir="$BUILD_DIR/guichan",
               duplicate=0,
               exports='env')

env.SConscript("SConscript",
               build_dir="$BUILD_DIR/",
               duplicate=0,
               exports='env')

