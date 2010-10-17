---
layout: post
title: rlvm 0.09
---

As of rlvm 0.9, there is now preliminary Little Busters! support. Currently, only the original version of Little Busters is supported (There are some additional features required to support Little Busters Ecstasy!). While the entire game is completable, there are major graphics artifacts and the majority of ending sequences are garbled or just blank.

In addition, new saved games should save the current state of the screen properly. Previous versions of rlvm didn't know how to save some commonly used graphics effects, leading to a blank background on game load. Hopefully, the new system is future proof to new graphics commands that I'll implement.

Several other regressions were also fixed.

<div class="downloadinfo">
<h2>Downloads</h2>
<ul>
  <li class="ubuntuicon">
    <a href="http://www.elliotglaysher.org/Releases/rlvm_0.9_i386.deb"
     onClick="_gaq.push(['_trackEvent', 'Download', 'rlvm_0.9_i386.deb']);">
      Ubuntu (Lucid) package for x86 machines (version 0.09)
    </a>
  </li>
  <li class="ubuntuicon">
    <a href="http://www.elliotglaysher.org/Releases/rlvm_0.9_amd64.deb"
     onClick="_gaq.push(['_trackEvent', 'Download', 'rlvm_0.9_amd64.deb']);">
      Ubuntu (Lucid) package for amd64 machines (version 0.09)
    </a>
  </li>
  <li class="macicon">
    <a href="http://www.elliotglaysher.org/Releases/rlvm_0.9.dmg"
       onClick="_gaq.push(['_trackEvent', 'Download', 'rlvm_0.9.dmg']);">
      Mac OSX Application (Intel) (version 0.09)
    </a>
  </li>
  <li class="sourceicon">
    <a href="http://github.com/eglaysher/rlvm/tarball/release-0.09"
       onClick="_gaq.push(['_trackEvent', 'Download', 'source-0.9']);">
      Source code (0.09)
    </a>
  </li>
</ul>
</div>
