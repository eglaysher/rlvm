---
layout: post
title: rlvm 0.3 released!
tags: [rlvm]
---

Work has been started on getting CLANNAD playable under rlvm. Nagisa's route is playable to the end, minus the MPEG video. rlvm now remembers the CG viewed.  User selections are now usable and don't lock up the interpreter. Despite all this, CLANNAD support is far from complete. There are still several graphical glitches and unimplemented opcodes.

In addition, there are several stability fixes. The race condition in the BGM system has been fixed. Scrolling through Planetarian's backlog no longer crashes.

<h2>Download</h2>
<ul>
  <li class="ubuntuicon">
    <a href="http://www.elliotglaysher.org/Releases/rlvm_0.3_i386.deb"
       onClick="pageTracker._trackEvent('Download', 'rlvm_0.3_i386.deb');">
      Ubuntu (Hardy) package for x86 machines (version 0.03)
    </a>
  </li>
  <li class="macicon">
    <a href="http://www.elliotglaysher.org/Releases/rlvm_0.3.dmg"
       onClick="pageTracker._trackEvent('Download', 'rlvm_0.3.dmg');">
      Mac OSX Application (Intel Only) (version 0.03)
    </a>
  </li>
  <li class="sourceicon">
    <a href="http://github.com/eglaysher/rlvm/tarball/release-0.03"
       onClick="pageTracker._trackEvent('Download', 'source-0.3');">
      Source code (0.03)
    </a>
  </li>
</ul>

