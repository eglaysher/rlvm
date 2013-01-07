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
specifically those released under the Key label. It is licensed to you
under version 3 or later of the GNU General Public License as published
by the Free Software Foundation.

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
| SNOW                   | Works?             | (None)              |
|------------------------+--------------------+---------------------|

For more details, please read STATUS.TXT.

rlvm is now at the point where enough commands are implemented that
other games *may* work. The above is only a list of what I've tested
first hand or what I've been told.

rlvm has an implementation of rlBabel; English patches compiled with
Haeleth's rlBabel should line break correctly. I've successfully tested
it with the Kanon patch from NDT and the beta CLANNAD patches from
Baka-Tsuki's Assembla page.

rlvm supports KOE, NWK and OVK archives for voices, along with ogg
vorbis voice patches which follow the convention
<packnumber>/z<packnumber><sampleid>.ogg.

Fullscreen can be entered by pressing Alt+{F,Enter} on Linux and
Command+{F,Enter} on Mac.

Please report bugs and discrepancies to <glaysher at umich dot edu>.


3) USING RLVM
-------------------------------------------------------------------------

By default, rlvm is a GUI program. Start it by double clicking on the
icon [OSX] or selecting it from your Applications menu [Linux]. You will
then want to select the folder that contains the SEEN.TXT of the game
you want to play.

If you are using Ubuntu or OSX, rlvm makes an effort to find a system
Japanese Gothic font. If rlvm is complaining that it can't find a font,
it will also look for the file "msgothic.ttc" in either the game
directory nor your home directory. If you are using Linux, you can
manually specify a font with the --font option.

On Linux, you can also manually start rlvm from the command line,
optionally passing in the directory containing the SEEN.TXT file:

 $ rlvm [/path/to/GameDirectory/]

The rlvm binary should be self contained and movable anywhere, though
the build system does not have an install target.

If you don't have the file "msgothic.ttc" in either the game directory
nor your home directory, please specify a Japanese font on the command
line with --font.


3) COMPILING RLVM
-------------------------------------------------------------------------
You will need the following libraries/utilities:

- The scons build system (Note: the scons in MacPorts has been reported
  to not work)
- boost 1.40 or higher
- SDL (and optionally, SDL_image, and SDL_mixer, though there are copies
  included in the source distribution if you don't already have them
  installed.)
- freetype
- GNU gettext

3a) Compiling under Linux
-------------------------------------------------------------------------

Make sure that freetype-config and sdl-config are in your $PATH. The
current SConscripts are fairly naive and may need editing to point to
the correct location of some header files. Future versions will have the
build system refined. You should be able to type:

 $ scons --release

in the root and have a complete build. The unit tests will automatically
be built and can be run by typing:

 $ ./build/rlvmTest

They should all pass.

There is no convenient install feature (I am not sure how to do that in
scons). rlvm will work (without icons or localization) in place. If you
wish to use rlvm in place, the binary is in:

 $ ./build/rlvm

For packagers, the debian/install file contains a mapping of where files
should be copied to. I agree that there should be a better long term
solution, though it currently escapes me.

3b) Compiling under OSX
-------------------------------------------------------------------------
In addition to freetype-config and sdl-config needing to be in your
$PATH, (like when compiling under Linux), the Sconscripts assume you use
fink to manage your unix programs and that fink is installed at /sw/,
though they also try to check for Darwin Ports. You may need to edit the
SConscript files if using something else.

To build an OSX application, type:

 $ scons --release --fullstatic

which will build rlvm.app in the build directory. The resulting OSX
application will ask you for the location of the game with a native
dialog.

To build a dmg file for distribution, type:

 $ scons --release --fullstatic --dmgname="rlvm <version>"


4) KNOWN ISSUES:
-------------------------------------------------------------------------
Using rlvm with insani's English translation of Planetarian is currently
not recommended as the indentation behaviour *will* break in the middle
of words.

;; Local Variables: **
;; fill-column: 72 **
;; End: **
