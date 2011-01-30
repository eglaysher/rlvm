-------------------------------------------------------------------------
rlvm: A RealLive interpreter clone for Linux, OSX and other UNIX-likes.
-------------------------------------------------------------------------

Table of Contents:
1) INTRODUCTION
2) STATUS
3) COMPILING RLVM
4) KNOWN ISSUES
5) GETTING INVOLVED

1) INTRODUCTION:
-------------------------------------------------------------------------
rlvm is a Free Software reimplementation of the VisualArt's KK's
RealLive interpreter. It is meant to provide Linux and Apple users with
a compatible, portable interpreter to play VisualArts games,
specifically those released by Key. It is licensed to you under version
3 or later of the GNU General Public License as published by the Free
Software Foundation.

rlvm is meant to be a compatibility utility; it is not an excuse for
copyright infringement. Please do not acquire games from BitTorrent /
Share / {insert name of popular P2P app in your locale}.

Special thanks to Jagarl for the first work on reverse engineering the
RealLive interpreter and for Free software licensed source code for
decoding their file formats, and to Haeleth for creating rldev along
with answering all my dumb questions about the internal workings of the
RealLive system.

2) STATUS
-------------------------------------------------------------------------

rlvm currently plays the following games to their completion:

|------------------------+--------------------+---------------------|
| Game                   | Original Japanese  |  English Fan Patch  |
|------------------------+--------------------+---------------------|
| Kanon Standard Edition | OK                 | Get from NDT        |
| Air Standard Edition   | OK                 | (None)              |
| CLANNAD                | OK                 | (Not supported)     |
| CLANNAD (Full Voice)   | OK                 | Get from Baka-Tsuki |
| Planetarian CD         | OK                 | (Broken)            |
| Tomoyo After           | OK                 | (None)              |
| Little Busters         | OK (Some glitches) | (Broken)            |
| Princess Bride         | Mostly works       | (None)              |
|------------------------+--------------------+---------------------|
| Princess Brave         | Works?             | (None)              |
| ALMA ~zutto soba ni~   | Reported to work   | (None)              |
| SNOW                   | (Broken)           | (None)              |
|------------------------+--------------------+---------------------|

For more details, please read STATUS.TXT.

rlvm is now at the point where enough commands are implemented that other games
*may* work. The above is only a list of what I've tested first hand or what
I've been told.

rlvm has an implementation of rlBabel; English patches compiled with Haeleth's
rlBabel should line break correctly. I've successfully tested it with the Kanon
patch from NDT and the beta CLANNAD patches from Baka-Tsuki's Assembla page.

rlvm supports KOE, NWK and OVK archives for voices, along with ogg vorbis voice
patches which follow the convention <packnumber>/z<packnumber><sampleid>.ogg.

Fullscreen can be entered by pressing Alt+{F,Enter} on Linux and
Command+{F,Enter} on Mac.

Please report bugs and discrepancies to <glaysher at umich dot edu>.

3) COMPILING RLVM
-------------------------------------------------------------------------
You will need the following libraries/utilities:

- The scons build system
- boost 1.40 or higher (1.35 has a subtle corruption bug; if you are
  upgrading from a copy of rlvm 0.7 that was compiled with 1.35, you may
  need to delete your save files. My Ubuntu packages and Mac applications
  did not use 1.35 for rlvm 0.7.)
- SDL (and optionally, SDL_image, and SDL_mixer, though there are copies
  included in the source distribution if you don't already have them
  installed.)
- freetype

3a) Compiling under Linux
-------------------------------------------------------------------------

Make sure that freetype-config and sdl-config are in your $PATH. The
current SConscripts are fairly naive and may need editing to point to the
correct location of some header files. Future versions will have the
build system refined. You should be able to type:

 $ scons --release

in the root and have a complete build. The unit tests will automatically
be built and can be run by typing:

 $ ./build/rlvmTest

They should all pass.

You should copy the contents of the Planetarian CD to a directory on
your hard drive for speed reasons. Playing directly from the CD is
discouraged. Once done, you should be able to run:

 $ ./build/rlvm [--font=/path/to/JapaneseTTF] /path/to/PlanetarianCopy

The rlvm binary should be self contained and movable anywhere, though
the build system does not have an install target.

If you don't have the file "msgothic.ttc" in either the game directory
nor your home directory, please specify a Japanese font on the command
line with --font.

3b) Compiling under OSX
-------------------------------------------------------------------------
In addition to freetype-config and sdl-config needing to be in your
$PATH, (like when compiling under Linux), the Sconscripts assume you use
fink to manage your unix programs and that fink is installed at /sw/. If
you are using Darwin Ports or are just compiling libraries manually and
installing them into /usr/local/, you may need to edit the SConscript
files.

To build an OSX application, type:

 $ scons --release --fullstatic

which will build rlvm.app in the build directory. The resulting OSX
application will ask you for the location of the game with a native
dialog.

You can also build rlvm as a command line program, as in Linux:

 $ scons --release

The result is run like this:

 $ ./build/rlvm /path/to/PlanetarianCopy

4) KNOWN ISSUES:
-------------------------------------------------------------------------
The interpreter currently lacks the following features:

 - TCC shading data used to give character images a certain tint in some
   scenes. To my knowledge, the file format has not been reverse
   engineered.

Using rlvm with insani's English translation of Planetarian is
currently not recommended as the indentation behaviour *will* break in
the middle of words.

5) GETTING INVOLVED
-------------------------------------------------------------------------
rlvm needs several file formats reverse engineered:

- The TCC file format hasn't been reverse engineered. TCC files (called
  "tonecurve" files in the GAMEEXE file) are tint definitions that are
  applied to graphics objects. Tones are applied to objects by loading
  "filename?[integer]" where [integer] appears to be an index. Any
  information about this file format would be appreciated.

-------------------------------------------------------------------------

;; Local Variables: **
;; fill-column: 72 **
;; End: **
