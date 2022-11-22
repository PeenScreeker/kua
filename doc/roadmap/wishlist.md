
### Roadmap: Postponed, but wanted as Core 
_(Features delayed due to scope control. Could take too long, shipping is priority)_  
_(All of them are very wanted, and will become Core eventually)_  
- [ ] CGLTF library integration
- [ ] Refactored engine
  - [ ] OpenGL3 renderer
  - [ ] GLFW, remove SDL
  - [ ] Nuklear UI integration
- [ ] Unlock 1000 maxfps. Change requires float, and could break long server times (+4days). 1ms = 1u:int = 1000fps
  - Notes: double should be good. But you never really need to cast full time to float. 
  The time bugs could be fixed by casting difference between times to float, which will be small enough
  _(no decimals allowed in millisec time per frame)_
  1000ms / 8ms = 125 fps
  1000ms / 7ms = 144 fps
  1000ms / 6ms = 166 fps
  1000ms / 5ms = 200 fps
  1000ms / 4ms = 250 fps
  ... // divide 1000 by 3, 2, etc to get the in-betweens
  1000ms / 1000 = 1ms  _Max FPS value allowed, because anything lower becomes decimals_

- [ ] Threads
