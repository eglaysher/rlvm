---
layout: post
title: rlvm 0.07
---

rlvm 0.7 is a major update. rlvm now supports AIR, along with several other
non-KEY games (see [STATUS.TXT][status]). rlvm is now at the point where random older RealLive games *may* work. It can now read KOE voice archives used by older games, as well as voice patches that are loose [Ogg Vorbis][ogg] files (such as the one for KANON). In addition, there's been a large number of small, minor graphical fixes, including faces in text boxes, text shadow in text boxes, visual appearance of #SELBTNs (such as in Fuuko's route), and various shading effects.

Because of problems in previous versions of the boost::serialization library,
preferences such as text box color and text speed will not be read with rlvm
0.7. Saved games were not subject to the corruption bugs and should continue to
work.

[ogg]: http://en.wikipedia.org/wiki/Vorbis
[status]: http://github.com/eglaysher/rlvm/blob/master/STATUS.TXT