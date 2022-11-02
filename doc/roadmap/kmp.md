

<!--................................-->
- [ ] 1.00


<!--................................-->
## Racing HUD
- [ ] Fix scoreboard
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
  - [ ] Auto demo recording on timerStart, stop on timerEnd, and cancel on playerdie
- [ ] Ghosts and Records
  - [ ] Records saving to disk 
  - [ ] Automatic replay recording
  - [ ] Ghosts
    - [ ] new : Multiple ghosts (example: own & spec'ed player)
- [ ] Improved replay tools (q3mme inspired, using the new UI tools)
- [ ] Gamemodes
  - [ ] Run (complete)
  - [ ] Freestyle Mode
  - [ ] FastCap
  - [ ] New:
    - [ ] 1v1 Race
    - [ ] Native race mode (like AG, first to finish wins. also FFA race, standings based on finish order)
    - [ ] PVP race mode (can interact with other players, and do things to change their movement)
    - [ ] Timed Duel maps : Pick all key items to set a time (like ctf, but for duel maps instead)
    - [ ] First To Finish: Group and 1v1
- [ ] Leaderboards
  - [ ] Server leaderboards
  - [ ] Web access to leaderboards data
- [ ] New data and stats:
  - [ ] Persistent stats per map.
  - [ ] Checkpoints: player vs wr, p vs pb (spec or own), p vs own pb
  - [ ] Getting data from leaderboards server to compare
