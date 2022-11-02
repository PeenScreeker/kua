
### Roadmap: Postponed, but wanted as Core 
_(Features delayed due to scope control. Could take too long, shipping is priority)_  
_(All of them are very wanted, and will become Core eventually)_  
- [ ] CGLTF library integration
- [ ] Refactored engine
  - [ ] OpenGL3 renderer
  - [ ] GLFW, remove SDL
  - [ ] Nuklear UI integration
- [ ] Unlock 1000 maxfps. Change requires float, and would break long server times (+4days). 1ms = 1u:int = 1000fps
  - Notes: double should be good. But you never really need to cast full time to float. 
  The time bugs could be fixed by casting difference between times to float, which will be small enough
  so instead of `float delta = (float)endTime - (float)startTime`, it would be `delta = (float)(endTime - startTime)`
- [ ] Threads
