
import os

############################################################# [ World options ]

# Set libraries used by all configurations and all binaries in rlvm.
env = Environment(
  LIBS = [
    "boost_program_options",
    "boost_serialization",
    "boost_iostreams",
    "boost_filesystem",
    "boost_date_time",
    "boost_thread-mt",
    "z"
  ],

  CPPPATH = [
    "#/src",
    "#/vendor",
  ],

  CPPFLAGS = [
    "--ansi",
    "-funsigned-char"
  ]
)

Progress('$TARGET\r', overwrite=True, file=open('con', 'w'))

# Auto select the number of processors
if os.path.exists('/proc'):
  cpus = len([l for l in open('/proc/cpuinfo') if l.startswith('processor\t')])
else:
  cpus = 1
env.SetOption('num_jobs', cpus + 1)

# Use timestamps change, followed by MD5 for speed
env.Decider('MD5-timestamp')

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
VerifyLibrary(config, 'SDL', 'SDL/SDL.h')
VerifyLibrary(config, 'SDL_image', 'SDL/SDL_image.h')
VerifyLibrary(config, 'SDL_mixer', 'SDL/SDL_mixer.h')
VerifyLibrary(config, 'ogg', 'ogg/ogg.h')
VerifyLibrary(config, 'libvorbis', 'vorbis/vorbisfile.h')
if not config.CheckBoost('1.35'):
  print "Boost version >= 1.35 needed to compile rlvm!"
  Exit(1)

env = config.Finish()
SConscript("SConscript", build_dir="build", duplicate=0, exports='env')
