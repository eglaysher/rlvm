---
layout: post
title: rlvm 0.06.2
---

Bugfix release.

- Fixes regression where G00 animations weren't being played (Seen in
  Planetarian.)
- Fixes regression where Yumemi's face in the final scene in Planetarian
  was being drawn twice because of poor clip rect handling.
- Support for English patches compiled with debugging symbols stripped.
- Temporarily disables scrollback due to crash that will be invasive to
  fix.
