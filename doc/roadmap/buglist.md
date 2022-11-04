# Bug fixes
## New
- [ ] Item pickups sometimes don't draw when overlapping with weapon switch hud element
- [ ] Viewpoint flicker on Feetraise
- [ ] `/map_restart` doesn't reset timer
- [ ] (CPM) Correct deceleration values
- [ ] double-ramp-boost bug on `r7-climbworld` (found on VQ1, check on other physics)
- [ ] Wall-stop bug
- [ ] sound bug on some systems (potentially SDL non-static linking or version)  
## Pre-existing
- [ ] `g_synchronousClients 1` by default for offline
- [ ] 1ups overbounce fix ?
- [ ] Make target_speaker loop globally (currently can either loop or global, but not both)
- [ ] Solve issues with clip/slick/trigger brush rendering. Transparency, render limit, impossible to hide some brushes from gameplay
