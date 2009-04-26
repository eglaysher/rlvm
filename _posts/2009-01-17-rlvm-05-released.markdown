---
layout: post
title: rlvm 0.5 released!
tags: [rlvm]
---

The big new feature is rlBabel support! NDT's Kanon teaser patch is now playable to the end of the translated text. Western text is now word wrapped correctly and translation notes work (though there is a minor bug that they can only be read once). However, italicized text is not supported and italicized words will be displayed with the normal font. The text is also not kerned.

There appears to be some broken unofficial CLANNAD patches floating around. English patches that were *not* compiled with rlBabel will, obviously, not line break correctly. You can check to see if rlBabel is enabled by hitting [F1] at any time. "rlBabel" should be set to "Enabled" and "Text Encoding" should be set to "Western". rlvm can not, and will never, support these broken patches.

rlvm now has a <a href="http://eglaysher.github.com/rlvm">site on github</a> with <a href="http://eglaysher.github.com/rlvm/screenshots.html">screenshots</a> and <a href="http://eglaysher.github.com/rlvm/guide_kanon_eng.html">a guide to installing NDT's English patch</a> under Linux.

rlvm compiles on Ubuntu Intrepid Ibex (and most likely any modern Linux distribution) and Mac OSX 10.5 Leopard.

<h2>Download</h2>
<ul>
  <li>
    <a href="http://www.elliotglaysher.org/Releases/rlvm_0.5_i386.deb"
       onClick="pageTracker._trackEvent('Download', 'rlvm_0.5_i386.deb');">
      Ubuntu (Intrepid) package for x86 machines (version 0.05)
    </a>
  </li>
  <li>
    <a href="http://www.elliotglaysher.org/Releases/rlvm_0.5_amd64.deb"
       onClick="pageTracker._trackEvent('Download', 'rlvm_0.5_amd64.deb');">
      Ubuntu (Intrepid) package for amd64 machines (version 0.05)
    </a>
  </li>
  <li>
    <a href="http://www.elliotglaysher.org/Releases/rlvm_0.5.dmg"
       onClick="pageTracker._trackEvent('Download', 'rlvm_0.5.dmg');">
      Mac OSX Application (Intel Only) (version 0.06)
    </a>
  </li>
  <li>
    <a href="http://github.com/eglaysher/rlvm/tarball/release-0.05"
       onClick="pageTracker._trackEvent('Download', 'source-0.5');">
      Source code (0.05)
    </a>
  </li>
</ul>
