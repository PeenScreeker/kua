# Naming Convention:
## Glosary
cur     : Current host
trg     : Target
rls     : Release
dbg     : Debug
win     : Windows
lnx     : Linux
env     : Environment
src     : Source code
obj     : Object built from source code
__Name  : Name of X (string)
__Dir   : Folder containing X
__File  : File of X (object or string)
sc      : SCons related
*tru*   : Real value of Aliases. e.g: truArch='w', scArch='win32'
*valid* : Supported by the code. Not valid = support is not implemented

# Word Order:
Organization goes FIRST. Program organization, not idiomatic english.
  Category -> Thing -> Differentiating Exception 
  : catThing_excpt
  Example:
    English (wrong):     initialize_build_target
    Organization first:  trgBuild_init

# Case styling:
camelCase preferred 
PascalCase for classes/constants only, and for SCons builtin methods
snake_case discouraged, only for exception management
All-caps for SCons environment variables -only- (CCFLAGS, etc)
