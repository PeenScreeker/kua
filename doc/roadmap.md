# Roadmap

# Code
PlayerState rework to generic rpg

# Standalone Freedom
## UI
## Game: Client
## Game: Server

# UX
- [ ] Launcher & Autoupdater app (probably with nuklear ui)
- [ ] Steam integration (requires Standalone game)

# GUI
## UI
- [ ] Font support
- [ ] UI q3ui
- [ ] UI revamp
- [ ] Map loader UI  
- [ ] Improved Loading screen:
  - [ ] Better map info distribution
  - [ ] Add map description text field
- [ ] `/varcommand`

## HUD
- [ ] Hud element: Current map & Internal version  
- [ ] ... wsw hud system (how does it work? can it be ported?)


## Racing HUD
- [ ] New hud for Player state configuration (health, ammo, powerups, etc)  
- [ ] Proxymod support  
  - [ ] strafehud
    - [ ] accel
    - [ ] snaps
  - [ ] pitch
  - [ ] compass
  - [ ] jump
  - [ ] RL
  - [ ] GL
  - [ ] bbox
  - [ ] Port [examples](github.com/Jelvan1/cgame_proxymod#examples)

---

# Gameplay
## Entities
### New
- [ ] Velocity pads  
- [ ] Q1 barrel
- [ ] USE buttons
- [ ] Port Warp entities

### Change
- [ ] Teleport
  - [ ] 0ups SpawnFlag
  - [ ] No reset speed SpawnFlag
- [ ] Map entities reset on `ClientSpawn()`
- [ ] Support for all defrag Entities
  - [ ] trigger_ entities
    - [ ] trigger_push_velocity
  - [ ] target_ entities
    - [ ] target_speed
    - [x] target_startTimer
    - [x] target_stopTimer
    - [ ] target_checkpoint
    - [ ] target_fragsFilter
    - [ ] target_init
    - [ ] target_smallprint
    - [ ] target_print
    - [ ] target_multimanager
  - [ ] shooter_ entities
    - [ ] shooter_grenade_targetplayer
    - [ ] shooter_plasma_targetplayer
    - [ ] shooter_rocket_targetplayer
    - [ ] shooter_bfg
  - [ ] weapon_grapplinghook_types (support for all df hook types)
- [ ] Entity filters:
  - [ ] notcpm
  - [ ] notvq3
  - [ ] notsp
  - [ ] notmp
  - [ ] notdf
  - [ ] nottm
  - [ ] notfc
  - [ ] notdefrag

## Physics
- [ ] CPM rework (speed)
  - [ ] New name
  - [ ] Autojump
    - [ ] no-boost on hold
    - [ ] boost on manual
  - [ ] Halfjump
  - [ ] SBJ
  - [ ] No W-turn
  - [ ] Crouchslide

- [ ] VQ2 rework (tech)
  - [ ] Autojump
    - [ ] no-boost on hold
    - [ ] boost on manual
  - [ ] Crouchslide with vq3 slick turnrate
  - [ ] Additive multijumps
  - [ ] Rampslides
  - [ ] Rampjumps (even during rampslide, if below dj-time)

- [x] vq1 movement (qw/ag)
  - [x] AD movement
    - [x] QW balanced
    - [x] AG balanced
  - [x] Half Jumps (aka Duckroll)
  - [ ] Feetraise (SBJ)
    - [x] Basic implementation
    - [ ] Fix crouch-jumping
  - [ ] Q1 rocket launcher
  - [ ] Gaus (needs new name)
  - [ ] ...
- [x] vq4 movement
- [x] vjk movement
  - [x] holdboost mechanic
- [ ] New mechanics
  - [ ] Airjump (Powerup and WW passive)
  - [ ] Walljumps (urt insp) (optional wsw-like powerup, to boost its power)
  - [ ] Dash
  - [ ] Blink
  - [ ] Charge
  - [ ] Bow
  - [ ] ...
- [ ] W based physics (WW)
  - [ ] W turning affected by phy_ground_accel and phy_speed (aka also by haste)
  - [ ] W accelerate (how to balance)
  - [ ] AirJump by default
  - [ ] Dash by default (ground and air)
  - [ ] Blink by default (MC-EoE style?)
  - [ ] Wallkick by default
- [ ] Vortex weapons (implosion/pull instead of explosion/knockback)
- [ ] Portals

## Multiplayer
- [ ] Multiplayer
  - [ ] Remove player interaction (`df_mp_interferenceOff`)
  - [ ] Per client entity state (timers, weapons, etc)
  - [ ] Scoreboard
- [ ] Better timer
  - [ ] Independent timer score (separate to fragfilters/score)
  - [ ] Revert hack for `trigger_multiple->wait -1` being hardcoded to `0.5`
  - [ ] Per-client activation of triggers
  - [ ] Checkpoints and comparison to best times

---

# Bug fixes
- [ ] `/map_restart` doesn't reset timer
- [ ] (CPM) Correct deceleration values
- [ ] double-ramp-boost bug on `r7-climbworld` (found on VQ1, check on other physics)
- [ ] Wall-stop bug
- [ ] sound bug on some systems (potentially SDL non-static linking or version)  
- [ ] 1ups overbounce fix
- [ ] `g_synchronousClients 1` by default for offline

---

### Postponed, but wanted as Core 
_(Features delayed due to scope control. Could take too long, shipping is priority)_  
_(All of them are very wanted, and will become Core eventually)_  
- [ ] CGLTF library integration
- [ ] Refactored engine
  - [ ] OpenGL3 renderer
  - [ ] GLFW, remove SDL
  - [ ] Nuklear UI integration
- [ ] Unlock 1000 maxfps. Possible with opengl1?

---

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
