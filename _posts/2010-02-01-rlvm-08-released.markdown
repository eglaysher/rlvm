---
layout: post
title: rlvm 0.08
---

rlvm 0.8 is a major update focusing on supporting previously unsupported
features of RealLive. rlvm now can read HIK animation files, such as those used
in the Planetarian opening sequence. rlvm can also now do weather effects:
snow, sakura petals, balls of light, etc. These effects are used in pretty much
every game I can find. rlvm also now does a better job line-breaking Japanese text. There are certain rules about certain characters not being the first character on a line that rlvm previously failed to implement correctly.

This version contains a fairly serious fix where, on certain graphics card
drivers, some (but not all) images would have their blue and red color
components swapped.

Due to a combination of my G4 laptop dying, Apple no longer supporting PPC
chips with the release of Snow Leopard, and difficulties with several libraries
I use on the PPC platform, rlvm 0.8 and future releases will be Intel only on
the Mac.

<div class="downloadinfo">
<h2>Downloads</h2>
<ul>
  <li class="sourceicon">
    <a href="http://github.com/eglaysher/rlvm/tarball/release-0.08"
       onClick="_gaq.push(['_trackEvent', 'Download', 'source-0.8']);">
      Source code (0.08)
    </a>
  </li>
</ul>
</div>