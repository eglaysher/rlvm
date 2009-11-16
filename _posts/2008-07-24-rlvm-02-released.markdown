---
layout: post
title: rlvm 0.02
tags: [rlvm]
---

rlvm 0.2 adds music and sound effect support, along with custom mouse cursors included in RealLive games.

While rlvm plays music and sounds, rlvm still does not support voices. nwatowav does not support the voice format used on the Planetarian CD edition. If anyone knows anything about the voice file format used in Planetarian, please contact me at <em>glaysher at umich dot edu</em>.

This version is not backwards compatible with rlvm 0.1's save files. I've added versioning information to the save files so hopefully future versions of rlvm will be compatible with the save game files produced by version 0.2. No user action is required to upgrade; save files are written in a different location and are now compressed with zlib.

rlvm compiles on Ubuntu Hardy Heron (and most likely any modern Linux distribution), Mac OSX 10.4 Tiger, and Mac OSX 10.5 Leopard.

<h2>Downloads</h2>
<ul>
  <li class="sourceicon">
    <a href="http://github.com/eglaysher/rlvm/tarball/release-0.02"
       onClick="pageTracker._trackEvent('Download', 'source-0.2');">
      Source code (0.02)
    </a>
  </li>
</ul>
