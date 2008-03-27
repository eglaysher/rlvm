-------------------------------------------------------------------------
rlvm: A RealLive interpreter clone for Linux, OSX and other UNIX-likes.
-------------------------------------------------------------------------

Table of Contents:
1) INTRODUCTION
2) STATUS
3) COMPILING RLVM
4) KNOWN ISSUES

1) INTRODUCTION:
-------------------------------------------------------------------------
rlvm is a Free Software reimplementation of the VisualArt's KK's
RealLive interpreter. It is meant to provide Linux and Apple users
with a compatible, portable interpreter to play VisualArts games,
specifically those released by Key. It is licensed to you under
"version 2 or later" of the GNU General Public License as published by
the Free Software Foundation. (Later versions may explicitly be
licensed under GPLv3).

rlvm is meant to be a compatibility utility; it is not an excuse for
copyright infringement. Please do not acquire games from bittorrent /
Share / {insert name of popular P2P app in your locale}. 

Special thanks to Jagarl for the first work on reverse engineering the
RealLive interpreter and for Free software licensed source code for
decoding their file formats, to Haeleth for creating rldev along with
answering all my dumb questions about the internal workings of the
RealLive system, and to my current roomate for goading me on by
claiming I'd abandon this project a week after I started. (He claimed
this two and half years ago.)

2) STATUS
-------------------------------------------------------------------------
This beta is currently only compatible with the Planetarian CD
edition[1]. Planetarian is playable through the end, though rlvm
currently doesn't implement all features used in the game (see known
issues).

This is the first public release of rlvm. Please report bugs and
discrepancies in Planetarian (other then the opening scene not showing
and the lack of sound) to <glaysher at umich dot edu>.

The games CLANNAD and Kanon SE are not playable through the
end. Haeleth's unofficial "ONE" demo does not render correctly. No
other games have been tested at this time.

3) COMPILING RLVM
-------------------------------------------------------------------------
You will need the following libraries/utilities:

- The omake build system
- Haeleth's rldev package (which also needs omake)
- boost 1.34.1
- SDL and SDL_image
- freetype

Make sure that freetype-config and sdl-config are in your $PATH. The
current OMakefiles are fairly naive and may need editing to point to
the correct location of some header files. Future versions will have
the build system refined. You should be able to type:

$ omake

in the root and have a complete build. The unit tests can be run by typing

$ test/rlvmTest 

They should all pass.

You should copy the contents of the Planetarian CD to a directory on
your hard drive for speed reasons. Playing directly from the CD is
discouraged. Once done, you should be able to run:

$ src/rlvm /path/to/PlanetarianCopy

rlvm is currently not meant to be installed to /usr/bin, and should be
run out of the unpacked source folder.

Future versions may transition to CMake to provide native build files
for whichever system. rlvm will most likely never use GNU autoconf.

4) KNOWN ISSUES:
-------------------------------------------------------------------------
The interpreter currently lacks the following features:

 - Haikei / HIK animations. To my knowledge, the file format has not
   been reverse engineered.
 - TCC shading data used to give character images a certain tint in
   some scenes. To my knowledge, the file format has not been reverse
   engineered.
 - Sound
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

-------------------------------------------------------------------------

[1]  The Planetarian download edition is not supported and never
     will be. I can not write, nor can I accept, code to run the
     download edition or other DRMed Kinetic games because of the
     anticircumvention provisions of the United States Digital
     Millennium Copyright Act.)
