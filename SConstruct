
import os
from SCons.Conftest import *

# 0.98.3 had a bug where it wasn't generating config.h files correctly. :(
EnsureSConsVersion(0, 98, 5)

############################################################# [ World options ]

AddOption('--release', action='store_true',
          help='Builds an optimized release for the platform.')
AddOption('--coverage', action='store_true',
          help='Builds the unit tests for running through gcov, runs them, '
          'runs gcov, and then generates an html report with lcov.')
AddOption('--pprof', action='store_true',
          help='Build with Google\'s performance tools.')
AddOption('--fullstatic', action='store_true',
          help='Builds a static binary, linking in all libraries.')

# Set libraries used by all configurations and all binaries in rlvm.
env = Environment(
  tools = ["default", "rlvm"],

  LIBS = ["z"],

  LOCAL_LIBS = [],

  CPPPATH = [
    "#/build",
    "#/src",
    "#/vendor",
  ],

  CPPFLAGS = [
    "-funsigned-char"
  ],

  CPPDEFINES = [
    "HAVE_CONFIG_H",

    # We use gettext for translations.
    "ENABLE_NLS",

    # Use glext.h
    "GL_GLEXT_PROTOTYPES"
  ],

  # Where the final binaries should be put.
  OUTPUT_DIR = "#/build/",

  # Where to output libraries that we've built because they aren't installed on
  # the system
  LIBRARY_DIR = "#/build/libraries",

  # Where under build/ to build.
  BUILD_DIR = "#/build/debug/",

  # Debug or Release.
  BUILD_TYPE = "Debug",

  # Whether we build the test binary that requires lua.
  BUILD_LUA_TESTS = False,
)

if env['PLATFORM'] == "darwin":
  env.Append(
    LIBS = [
      "png",
      "bz2",
      "boost_program_options-mt",
      "boost_serialization-mt",
      "boost_iostreams-mt",
      "boost_filesystem-mt",
      "boost_date_time-mt",
      "boost_thread-mt",
      "boost_system-mt"])
else:
  env.Append(
    LIBS = [
      "boost_program_options",
      "boost_serialization",
      "boost_iostreams",
      "boost_filesystem",
      "boost_date_time",
      "boost_thread",
      "boost_system"])

if GetOption("fullstatic"):
  env["FULL_STATIC_BUILD"] = True

# Auto select the number of processors
if os.path.exists('/proc/cpuinfo'):
  cpus = len([l for l in open('/proc/cpuinfo') if l.startswith('processor\t')])
  env.SetOption('num_jobs', cpus + 1)

# Use timestamps change, followed by MD5 for speed
env.Decider('MD5-timestamp')

if ARGUMENTS.get('VERBOSE') != '1':
  env['CCCOMSTR'] = 'Compiling $TARGET ...'
  env['CXXCOMSTR'] = 'Compiling $TARGET ...'
  env['SHCCCOMSTR'] = 'Compiling $TARGET ...'
  env['SHCXXCOMSTR'] = 'Compiling $TARGET ...'
  env['LINKCOMSTR'] = 'Linking $TARGET ...'
  env['ARCOMSTR'] = 'Archiving $TARGET ...'
  env['RANLIBCOMSTR'] = 'Indexing $TARGET ...'
  env['SHLINKCOMSTR'] = 'Linking $TARGET ...'

#########################################################################
## Platform Specific Locations
#########################################################################
if env['PLATFORM'] == "darwin":
  # Fink based mac
  if os.path.exists("/sw/"):
    env['ENV']['PATH'] += ":/sw/bin"
    env.Append(
      CPPPATH = ["/sw/include/"],
      LIBPATH = ["/sw/lib/"]
    )
  # Darwinports based mac
  if os.path.exists("/opt/local"):
    env['ENV']['PATH'] += ":/opt/local/bin"
    env.Append(
      CPPPATH = ["/opt/local/include/"],
      LIBPATH = ["/opt/local/lib/"]
    )
  # Manually installed / homebrew
  if os.path.exists("/usr/local/"):
    env['ENV']['PATH'] += ":/usr/local/bin/"
    env.Append(
      CPPPATH = ["/usr/local/include/"],
      LIBPATH = ["/usr/local/lib/"]
    )
  if os.path.exists('/usr/local/opt/gettext/'):
    # Homebrew does this bogus thing where installing the gettext bottle doesn't
    # actually symlink the headers and libraries into /usr/local/.
    env.Append(
      CPPPATH = ["/usr/local/opt/gettext/include/"],
      LIBPATH = ["/usr/local/opt/gettext/lib/"]
    )

#########################################################################
## CheckBoost
#########################################################################
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

# def CheckGuichan(context):
#   # We specifically check for 0.8 because the authors have said they'll do
#   # sweeping, API breaking changed between major releases. gcnGuichanVersion()
#   # doesn't change during minor releases.
#   context.Message('Checking for guichan 0.8 with OpenGL and SDL support...')
#   lastLIBS = context.env['LIBS']
#   context.env.Append(LIBS = ['guichan', 'guichan_opengl', 'guichan_sdl'])
#   ret = context.TryRun("""
# #include <guichan.hpp>
# #include <cstring>

# int main(int argc, char **argv) {
#   return std::strcmp(gcnGuichanVersion(), "0.8") != 0;
# }
# """, ".cc")[0]
#   if not ret:
#     context.env.Replace(LIBS = lastLIBS)
#   context.Result( ret )
#   return ret

def VerifyLibrary(config, library, header):
  if not config.CheckLibWithHeader(library, header, "c"):
    if config.CheckLib(library):
      print "You have " + library + " installed, but the development headers aren't installed."
    else:
      print "You need " + library + " to compile this program!"
    Exit(1)

def CheckForSystemLibrary(config, library_dict, componentlist):
  """
  Configure check to see if a certain library is installed, falling back on an
  included version. Most of this check is ripped off from CheckLib, but with
  custom handling on build failure, where instead we add the source to the
  dependency graph instead.
  """

  res = None
  if not GetOption("fullstatic"):
      res = config.CheckLibWithHeader(library_dict['library'],
                                      library_dict['include'],
                                      'cpp',
                                      call = library_dict['function'])

  if not res:
    lib_name = library_dict['library']
    print "(Using included version of %s)" % lib_name
    componentlist.append(lib_name)
    config.Define("HAVE_LIB" + lib_name.replace("-", "_"), 1,
                  "Define to 1 if you have the `%s' library." % lib_name)


#########################################################################
## Configuration
#########################################################################
subcomponents = [ ]

config = env.Configure(custom_tests = {'CheckBoost'   : CheckBoost,
#                                       'CheckGuichan' : CheckGuichan
                                      },
                       config_h="build/config.h")
if not config.CheckBoost('1.40'):
  print "Boost version >= 1.40 needed to compile rlvm!"
  Exit(1)

VerifyLibrary(config, 'pthread', 'pthread.h')
VerifyLibrary(config, 'ogg', 'ogg/ogg.h')
VerifyLibrary(config, 'vorbis', 'vorbis/codec.h')
VerifyLibrary(config, 'vorbisfile', 'vorbis/vorbisfile.h')

VerifyLibrary(config, 'sndfile', 'sndfile.h')

# In short, we do this because the SCons configuration system doesn't give me
# enough control over the test program. Even if the libraries are installed,
# they won't compile because SCons outputs "int main()" instead of "int
# main(int argc, char** argv)", which SDL macros into its own special SDL_main
# entrypoint, and the CheckXXX tests don't allow me a way to inject "#undef
# main" before I declare the main() function.
if env['PLATFORM'] != 'darwin':
  VerifyLibrary(config, 'SDL2', 'SDL2/SDL.h')
else:
  print "Can't properly detect SDL under OSX. Assuming you have the libraries."

# Libraries we need, but will use a local copy if not installed.
local_sdl_libraries = [
  {
    'include'  : 'zita-resampler/resampler.h',
    'library'  : 'zita-resampler',
    'function' : '',
  },
  {
    'include'  : 'SDL2/SDL_ttf.h',
    'library'  : 'SDL2_ttf',
    'function' : 'TTF_Init();'
  },
  {
    'include'  : 'SDL2/SDL_mixer.h',
    'library'  : 'SDL2_mixer',
    'function' : ''
  },
  {
    'include'  : 'SDL2/SDL_image.h',
    'library'  : 'SDL2_image',
    'function' : ''
  }
]

for library_dict in local_sdl_libraries:
  CheckForSystemLibrary(config, library_dict, subcomponents)

# if not config.CheckGuichan():
#   print "(Using included copy of guichan)"
#   subcomponents.append("guichan")

# Get the configuration from sdl and freetype
env.ParseConfig("sdl2-config --cflags")
env.ParseConfig("freetype-config --cflags --libs")

env = config.Finish()

### HACK! Until I make my own version of CheckLibWithHeader, just assume that
### we have the right libraries. This needs to be done after config.Finish() is
### called or else we get a really confusing error.
if env['PLATFORM'] == 'darwin':
  env.Append(LIBS=["SDL2", "intl", "iconv"])

# Get the configuration from sdl and freetype
env.ParseConfig("sdl2-config --cflags")
env.ParseConfig("freetype-config --cflags --libs")

#########################################################################
## Building subcomponent functions
#########################################################################
for component in subcomponents:
  env.BuildSubcomponent(component)

#########################################################################
## Building things
#########################################################################

duplicate_files = 0
# Hacky Temporary Differentiation between release and debug:
if GetOption('release'):
  env["BUILD_DIR"] = "#/build/release"
  env['BUILD_TYPE'] = "Release"

  # Now add release optimizations to the environment
  env.Append(
    CPPFLAGS = [
      "-Os",
      "-DNDEBUG",
      "-DBOOST_DISABLE_ASSERTS"
    ]
  )

  # I would love to enable gc-sections everywhere, but it isn't enabled on OSX?
  if env['PLATFORM'] != "darwin":
    env.Append(
      CPPFLAGS = [
        "-ffunction-sections",
        "-fdata-sections",
        ],
  
      LINKFLAGS = [
        "-Wl,--gc-sections"
        ],
      )
elif GetOption('coverage'):
  env["BUILD_DIR"] = "#/build/coverage"
  env['BUILD_TYPE'] = "Release"

  env.Append(
    CPPFLAGS = [
      "-O0",
      "-g",
      "-fprofile-arcs",
      "-ftest-coverage"
    ],

    LIBS = [ "gcov" ]
  )
elif GetOption('pprof'):
  env["BUILD_DIR"] = "#/build/pprof"
  env['BUILD_TYPE'] = "Release"

  env.Append(
    CPPFLAGS = [
      "-O0",
      "-g",
    ],

    LIBS = [ "tcmalloc",
             "profiler"]
  )
else:
  # Add debugging flags to all binaries here
  env.Append(
    CPPFLAGS = [
      "-g",
      "-O0"
    ]
  )

# Cross platform core of rlvm. Produces librlvm.a and libsystem_sdl.a
env.SConscript("SConscript",
               variant_dir="$BUILD_DIR/",
               duplicate=0,
               exports='env')

# Run the correct port script. If we're on darwin, we'll run the cocoa version,
# everywhere else we assume GTK+.
if env['PLATFORM'] == 'darwin':
  env.SConscript("SConscript.cocoa",
                 variant_dir="$BUILD_DIR/",
                 duplicate=0,
                 exports='env')
else:
  env.SConscript("SConscript.gtk",
                 variant_dir="$BUILD_DIR/",
                 duplicate=0,
                 exports='env')

# Copy the platform independent SEEN.TXT files to output (we no longer depend
# on rldev because I can no longer reliably compile it).
env.SConscript("test/SConscript.rlc",
               variant_dir="build/test",
               duplicate=0,
               exports='env')

# Build the rlvmTests binary that uses those SEEN.TXT files.
env.SConscript("SConscript.test",
               variant_dir="$BUILD_DIR/",
               duplicate=0,
               exports='env')

env.SConscript("SConscript.luarlvm",
               variant_dir="$BUILD_DIR/",
               duplicate=0,
               exports='env')

if GetOption("coverage"):
  env.SConscript("SConscript.coverage",
                 variant_dir="$BUILD_DIR/",
                 duplicate=0,
                 exports='env')

# In addition to all that, we also want to build the translation files.
env.Command("build/locale/ja/LC_MESSAGES/rlvm.mo",
            "po/ja.po",
            ["mkdir -p build/locale/ja/LC_MESSAGES/",
             "msgfmt -o build/locale/ja/LC_MESSAGES/rlvm.mo po/ja.po"])
