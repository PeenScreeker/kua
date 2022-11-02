# Done:
## Works in default q3a
- [x] Surface flags (uses shader properties)
- [x] Vq3 movement (`pmove_fixed 1`)
- [x] Vq3 Teleports
- [x] vq3 Hurt Trigger
- [x] target score

## Minimal working product (0.0.1)
- [x] Initial setup & compilation (manual)
- [x] Map loading
- [x] Changes to the code actually affect gameplay through `.qvm` files

## Automated compilation on bash shell (0.0.2)
- [x] Automated compilation from makefile
- [x] Automated packing

## Minimal viable product (0.1.0)
- [x] Full vq3 movement
- [x] Basic vq3 Weapons (should be full by default)  
- [x] Basic cpm strafing (Correct accel, aircontrol and A/D strafing)
- [x] Instant respawn (g_combat.c/player_die()/line608  & some other spots to switch timer to ms and make 1 mean immediate )
- [x] Gamemode "run"
- [x] Timer (local, best per session, all maps have TimeReset)
- [x] Gamemode "run" (basic)
  - [x] Definition in code (replaced FFA, g_gametype 0)
- [x] First cpm/vq3 alpha/dev release version

## CPM and Physics selection (0.2.0)
- [x] Physics type Selection (phy_movetype NUMBER :: 0=CPM, 3=VQ3, 1:Q1-ag)
- [x] Full cpm strafing (double jumps, slick and other specific mechanics)
- [x] Full CPM weapons
  - [x] Instant weapon switch
  - [x] Rocket Launcher speed increased from 900 to 1000
  - [x] Rocket Launcher knockback increased from 1 to 1.2
  - [x] Item pickup size increased (from 36 to 66)
  - [x] CPMA changes: Need side-by-side comparison of csdf-cpm and osdf-cpm
    - [x] (false) Shotgun damage decreased from 10 to 7
    - [x] (false) Shotgun spread increased from 700 to 900
    - [x] (false) Shotgun knockback increased from 1 to 1.35
    - [x] (false) Plasma Gun damage reduced from 20 to 15
    - [x] (false) Plasma Gun knockback reduced from 1 to 0.5
    - [x] (false) Grenade Launcher reload reduced from 800 to 600
    - [x] (false) Gauntlet knockback changed from 1 to 0.5
    - [x] (false) Machine Gun damage changed from 7 to 5
    - [x] (false) Lightning Gun knockback increased from 1 to 1.55
    - [x] (false) Railgun cooldown decreased from 1500 to 1000 for weapon switch
- [x] no-ob
  - [x] Rough implementation (cvar phy_overbounce_scale = 1.000f) //TODO Code is created. cvar is currently disconnected
  - [x] Robust fix for random overbounces only, while keeping the good ones.

## New Physics (0.3.0)
- [x] Physics: VJK
- [x] Physics: VQ4
- [x] Physics: CQ3
- [x] Physics: New meachanics for VQ1
- [x] HUD: Skim timer
- [x] Sound: Event limit system

## OSDF to KUA (0.35a)
- [x] New Users UX: Basic Launcher & Autoupdater app
- [x] Font support: cgame hud
- [x] Aspect correct hud
