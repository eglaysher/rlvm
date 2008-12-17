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

This beta is currently only compatible with the Planetarian CD
edition[1], the non-voiced version of CLANNAD and the Standard Edition
of Kanon. Planetarian is playable through the end, though rlvm currently
doesn't implement all features used in the game (see known issues). All
of the main routes necessary to get to the After Story True End work are
playable, minus some problems with graphics and the Megumeru video not
playing. Kanon SE is playable in its entirety, with slightly less minor
graphical glitches than Planetarian or CLANNAD.

This is the fourth public release of rlvm. Please report bugs and
discrepancies to <glaysher at umich dot edu>.

3) COMPILING RLVM
-------------------------------------------------------------------------
You will need the following libraries/utilities:

- The scons build system
- boost 1.35 or higher
- SDL (and optionally, SDL_image, and SDL_mixer, though there are copies
  included in the source distribution if you don't already have them
  installed.)
- freetype

Optionally, you may want:
- Haeleth's rldev package (which builds the unit tests)

3a) Compiling under Linux
-------------------------------------------------------------------------

Make sure that freetype-config and sdl-config are in your $PATH. The
current OMakefiles are fairly naive and may need editing to point to the
correct location of some header files. Future versions will have the
build system refined. You should be able to type:

 $ scons --release

in the root and have a complete build. If you have rldev installed, the
unit tests will automatically be built and can be run by typing:

 $ cd build && ./rlvmTest 

They should all pass.

You should copy the contents of the Planetarian CD to a directory on
your hard drive for speed reasons. Playing directly from the CD is
discouraged. Once done, you should be able to run:

 $ build/rlvm [--font=/path/to/JapaneseTTF] /path/to/PlanetarianCopy

The rlvm binary should be self contained and movable anywhere, though
the build system does not have an install target.

If you don't have the file "msgothic.ttc" in either the game directory
nor your home directory, please specify a Japanese font on the command
line with --font.

3b) Compiling under OSX
-------------------------------------------------------------------------
In addition to freetype-config and sdl-config needing to be in your
$PATH, (like when compiling under Linux), the OMakefiles assume you use
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

 $ build/rlvm /path/to/PlanetarianCopy

4) KNOWN ISSUES:
-------------------------------------------------------------------------
The interpreter currently lacks the following features:

 - Haikei / HIK animations. To my knowledge, the file format has not
   been reverse engineered.
 - TCC shading data used to give character images a certain tint in some
   scenes. To my knowledge, the file format has not been reverse
   engineered.
 - Koe / Narration

In addition, the current indentation system is extremely naive and does
not produce the same results as the official interpreter, though it's
sufficient for Japanese text as line breaks can occur essentially
anywhere. Using rlvm with insani's English translation of Planetarian is
currently not recommended as the indentation behaviour *will* break in
the middle of words.

Many graphics manipulation commands not required by Planetarian are
currently unimplemented. Kanon SE, Haeleth's unofficial ONE demo and
CLANNAD do not always render correctly because of this.

rlvm does not contain an implementation of Haeleth's rlBabel and Textout
extensions. Fan translations that use something other than the lower
ASCII set, specifically Chinese or Korean ones, will not work and ones
that are restricted to the lower ASCII set will look ugly.

5) GETTING INVOLVED
-------------------------------------------------------------------------
rlvm needs several file formats reverse engineered:

- rlvm contains jagarl's nwatowav, which doesn't currently decode the
  NWK voice files included with the Planetarian CD edition. I need a
  working decoding library before I can start work on voice support and
  I'm sure jagarl would like nwatowav to be able to read even more
  files.
- The HIK file format hasn't been reverse engineered at all. Nothing is
  known about this format, except that the commands to display and
  manipulate these files are in module<1:40>. These files are used in
  Planetarian to control the opening animation.
- The TCC file format hasn't been reverse engineered. TCC files (called
  "tonecurve" files in the GAMEEXE file) are tint definitions that are
  applied to graphics objects. Tones are applied to objects by loading
  "filename?[integer]" where [integer] appears to be an index. Any
  information about this file format would be appreciated.

-------------------------------------------------------------------------

[1]  The Planetarian download edition is not supported and never
     will be. I can not write, nor can I accept, code to run the
     download edition or other DRMed Kinetic games because of the
     anticircumvention provisions of the United States Digital
     Millennium Copyright Act.

;; Local Variables: **
;; fill-column: 72 **
;; End: **
