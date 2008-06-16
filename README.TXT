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
RealLive interpreter. It is meant to provide Linux and Apple users
with a compatible, portable interpreter to play VisualArts games,
specifically those released by Key. It is licensed to you under
version 3 or later of the GNU General Public License as published by
the Free Software Foundation.

rlvm is meant to be a compatibility utility; it is not an excuse for
copyright infringement. Please do not acquire games from BitTorrent /
Share / {insert name of popular P2P app in your locale}.

Special thanks to Jagarl for the first work on reverse engineering the
RealLive interpreter and for Free software licensed source code for
decoding their file formats, to Haeleth for creating rldev along with
answering all my dumb questions about the internal workings of the
RealLive system.

2) STATUS
-------------------------------------------------------------------------
This beta is currently only compatible with the Planetarian CD
edition[1]. Planetarian is playable through the end, though rlvm
currently doesn't implement all features used in the game (see known
issues).

This is the second public release of rlvm. Please report bugs and
discrepancies in Planetarian (other then the opening scene not
showing) to <glaysher at umich dot edu>.

The games CLANNAD and Kanon SE are not playable through the
end. Haeleth's unofficial "ONE" demo does not render correctly. No
other games have been tested at this time.

3) COMPILING RLVM
-------------------------------------------------------------------------
You will need the following libraries/utilities:

- The omake build system when building under Linux or OSX
- boost 1.34.1 or higher
- SDL, SDL_image, and SDL_mixer
- freetype

Optionally, you may want:
- Haeleth's rldev package (which builds the unit tests)

3a) Compiling under Linux
-------------------------------------------------------------------------

Make sure that freetype-config and sdl-config are in your $PATH. The
current OMakefiles are fairly naive and may need editing to point to
the correct location of some header files. Future versions will have
the build system refined. You should be able to type:

 $ omake

in the root and have a complete build. If you have rldev installed,
the unit tests will automatically be built and can be run by typing:

 $ test/rlvmTest 

They should all pass.

You should copy the contents of the Planetarian CD to a directory on
your hard drive for speed reasons. Playing directly from the CD is
discouraged. Once done, you should be able to run:

 $ src/rlvm [--font=/path/to/JapaneseTTF] /path/to/PlanetarianCopy

rlvm is currently not meant to be installed to /usr/bin, and should be
run out of the unpacked source folder.

If you don't have the file "msgothic.ttc" in either the game directory
nor your home directory, please specify a Japanese font on the command
line with --font.

3b) Compiling under OSX
-------------------------------------------------------------------------
In addition to freetype-config and sdl-config needing to be in your
$PATH, (like when compiling under Linux), the OMakefiles assume you
use fink to manage your unix programs and that fink is installed at
/sw/. If you are using Darwin Ports or are just compiling libraries
manually and installing them into /usr/local/, you will need to edit
the paths in the apple specific section at the end of src/Omakefile.

To build an OSX application, type:

 $ omake bundle

which will build rlvm.app in the src directory. The resulting OSX
application will ask you for the location of the game with a native
dialog.

You can also build rlvm as a command line program, as in Linux:

 $ omake

The result is run like this:

 $ src/rlvm /path/to/PlanetarianCopy

3c) Compiling under Windows
-------------------------------------------------------------------------
rlvm works under Windows with a caveat. While paths with characters
other then lower ASCII are correctly handled under Linux and OSX,
windows will refuse to handle them correctly. I am not entirely sure
of why this is. Patches are welcome; I wrote rlvm in part to get away
from Windows problems and have already lost a week trying to figure
out Windows nonstandard behaviour. 

Unlike Linux and OSX, rlvm is built with Microsoft Visual Studio under
Windows. Solution and vcproj files are provided. You will most likely
need to adjust the include and lib paths in the various vcproj files
for your local system.

(Note: If compiling with MSVC++ v8.0, you will probably need to build
boost from source. I couldn't find any prebuilt binary packages.)

4) KNOWN ISSUES:
-------------------------------------------------------------------------
The interpreter currently lacks the following features:

 - Haikei / HIK animations. To my knowledge, the file format has not
   been reverse engineered.
 - TCC shading data used to give character images a certain tint in
   some scenes. To my knowledge, the file format has not been reverse
   engineered.
 - Koe / Narration
 - Right click configuration menus / most SYSCOM commands.

In addition, the current indentation system is extremely naive and
does not produce the same results as the official interpreter, though
it's sufficient for Japanese text as line breaks can occur essentially
anywhere. Using rlvm with insani's English translation of Planetarian
is currently not recommended as the indentation behaviour *will* break
in the middle of words.

Many graphics manipulation commands not required by Planetarian are
currently unimplemented. Kanon SE, Haeleth's unofficial ONE demo and
CLANNAD are not playable/do not render correctly because of this.

rlvm will enter an infinite loop while interpreting Kanon SE's opening
movie because the command index_series is currently unimplemented and
not enough is known about what it does. For more information, see
section 5.6.1 of Haeleth's rldev manual.

User selection code has not been rigoursly tested; I've only tried the
first few decision points in CLANNAD. While holding control down to
see if I could get to the ending in CLANNAD, I noticed that the
interpreter would randomly hang, both before and after a decision was
presented to the user.

rlvm does not contain an implementation of Haeleth's rlBabel and
Textout extensions. Fan translations that use something other than the
lower ASCII set, specifically Chinese or Korean ones, will not work.

5) GETTING INVOLVED
-------------------------------------------------------------------------
rlvm needs several file formats reverse engineered:

- rlvm contains jagarl's nwatowav, which doesn't currently decode the
  NWK voice files included with the Planetarian CD edition. I need a
  working decoding library before I can start work on voice support
  and I'm sure jagarl would like nwatowav to be able to read even more
  files.
- The HIK file format hasn't been reverse engineered at all. Nothing
  is known about this format, except that the commands to display and
  manipulate these files are in module<1:40>. These files are used in
  Planetarian to control the opening animation.
- The TCC file format hasn't been reverse engineered. TCC files
  (called "tonecurve" files in the GAMEEXE file) are tint definitions
  that are applied to graphics objects. Tones are applied to objects
  by loading "filename?[integer]" where [integer] appears to be an
  index. Any information about this file format would be appreciated.

rlvm needs a few RealLive commands documented more thouroughly:

- index_series: The rldev documentation acknowledges that it is
  incomplete for the most common case, which is needed to support
  Kanon. This is the current blocker for Kanon support.

-------------------------------------------------------------------------

[1]  The Planetarian download edition is not supported and never
     will be. I can not write, nor can I accept, code to run the
     download edition or other DRMed Kinetic games because of the
     anticircumvention provisions of the United States Digital
     Millennium Copyright Act.)

