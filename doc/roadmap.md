# Roadmaps  
[ktt](./roadmap/ktt.md)
[kmp](./roadmap/kmp.md)
[done](./roadmap/done.md)
[wishlist](./roadmap/wishlist.md)
[buglist](./roadmap/buglist.md)

---
# Code
## Engine+Gamecode
- [ ] PlayerState rework 
  - [ ] Generic rpg
    - [ ] Attributes
    - [ ] State (health/armor)
  - [ ] Physics info
    - [ ] Gravity
    - [ ] Speed
    - [ ] ...
## Gamecode
## Engine
- [ ] Default to good visual config
- [ ] Port font support to the engine, and use callbacks to draw text

# Standalone Freedom
_Main Idea:      Disable loading of all media cache assets_
_Realistically:  Does this just work? Is it that "simple"? Need testing_
## UI
## Game: Client
## Game: Server

# UX
## New Users
- [ ] Steam integration (requires Standalone game)
## QoL
- [ ] cg_drawgun 2
- [ ] cg_nodamagekick
- [ ] Separate respawn command
- [ ] `/varcommand`
- [ ] Purple clearscreen color only on debug build

# GUI
## UI
- [ ] Font support
- [ ] UI q3ui
- [ ] UI revamp
- [ ] Menus
  - [ ] Map loader UI  
  - [ ] Setup
- [ ] Improved Loading screen:
  - [ ] Fix: Image size
  - [ ] Better map info distribution
  - [ ] Add map description text field
- [ ] Story-telling (? both same system ?)
  - [ ] Dialogue
  - [ ] Narration
- [ ] Cvar control
  - [ ] Serialize them

## HUD
- [ ] Skill bar
  - [ ] Basic version: Timer on top of icon (grayscale:inactive, colored:active)
- [ ] New icons : Warsow insp (vector style, simple shading, 3 tone range)
- [ ] New design: Player state (health, ammo, powerups, etc)  
  - [x] Basic: Q3 hud, aspect correct, widescreen, smaller
  - [ ] Rework (What's needed?)
  - [ ] Customization with cvars

---

# Gameplay
## Entities
### New
- [ ] Velocity pads  
- [ ] Q1 barrel  (model from that default q3 map)
- [ ] USE buttons
- [ ] Port Warp entities
- [ ] Client sided logic (EntityPlus and SourceEngine-I/O inspired)

### Change
- [ ] Teleport
  - [ ] Keep speed SpawnFlag
  - [ ] 0ups SpawnFlag
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
  - [ ] Separate to new file
  - [ ] Differential strafing
  - [ ] No W-turn
  - [ ] Crouchslide
  - [ ] Autojump
    - [ ] no-boost on hold
    - [ ] boost on manual
  - [ ] Halfjump
  - [ ] Feetraise
---
- [ ] VQ2 rework (tech)
  - [ ] Autojump
    - [ ] no-boost on hold
    - [ ] boost on manual
  - [ ] Crouchslide with vq3 slick turnrate
  - [ ] Additive multijumps
  - [ ] Rampslides
  - [ ] Rampjumps (even during rampslide, if below dj-time)
  - [ ] Feetraise
---
- [ ] New mechanics
  - [ ] Airjump (Powerup and WW passive)
  - [ ] Walljumps (urt insp) (optional wsw-like powerup, to boost its power)
  - [ ] Dash
  - [ ] Blink
  - [ ] Charge
  - [ ] Bow
  - [ ] Hover
  - [ ] ...
- [ ] W based physics (WW)
  - [ ] W turning affected by phy_ground_accel and phy_speed (aka also by haste)
  - [ ] W accelerate (how to balance)
  - [ ] AirJump by default
  - [ ] Dash by default (ground and air)
  - [ ] Blink by default (MC-EoE style?)
  - [ ] Wallkick by default
  - [ ] Hover ability
- [ ] Hooks
- [ ] Vortex weapons (implosion/pull instead of explosion/knockback)
- [ ] Portals
- [ ] Powerups: Physics selection (cpm and vq3 sections in the same map)
- [ ] Look-behind command (map-based, not global)
---
- [x] vq1 movement (qw/ag)
  - [x] AD movement
    - [x] QW balanced
    - [x] AG balanced
  - [x] Half Jumps (aka Duckroll)
  - [x] Feetraise (SBJ)
    - [x] Basic implementation
    - [x] Fix crouch-jumping
  - [ ] Q1 rocket launcher
  - [ ] Gaus (needs new name)
  - [ ] ...
- [x] vq4 movement
  - [x] Crouchslide
  - [x] Rampslide
  - [x] Feetraise
  - [ ] Ramp jumps
- [x] vjk movement
  - [x] holdboost mechanic

