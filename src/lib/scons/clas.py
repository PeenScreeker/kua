import sys; sys.dont_write_bytecode=True
#....................................................................
# Confy modules
from config import *
from core   import *
from tools  import *
if isVerbose(): print(f"confy: Importing module {__name__}")
#....................................................................
# SCons specific variables and methods
from SCons.Script import *
from SCons.Node import Node
from SCons.Script.SConscript import SConsEnvironment
#....................................................................
class BuildObj:
  def __init__(self, key:str|None=None, out:str|None=None, src:list|None=None, \
               sDir:Node|None=None, oDir:Node|None=None, rDir:Node|None=None, default=False, \
               cfl:list|None=None, defs:list|None=None, libs:list|None=None, parse:list|None=None, \
               env:SConsEnvironment|None=None, cStr:str|None=None, lStr:str|None=None ):
    self.sDir  = sDir if sDir is not None else srcDir
    self.oDir  = oDir if oDir is not None else binDir
    self.rDir  = rDir if rDir is not None else os.curdir
    self.key   = key
    self.bin   = out if out is not None else key
    self.src   = src
    self.env   = env
    self.cfl   = cfl
    self.defs  = defs
    self.libs  = libs
    self.parse = parse 
    self.cStr  = cStr if cStr else None if cStr in [""] or isVerbose() else ccstring
    self.lStr  = lStr if lStr else None if lStr in [""] or isVerbose() else ldstring
    self.default = default

  def setup(self):
    lnkDir    = LinkDir(self.sDir, self.oDir)
    self.src  = self.src if not isNodeGlob(self.src) else GlobBasename(self.src, dir=self.sDir, rel=self.rDir)
    self.src  = [lnkDir.File(f) for f in self.src] if self.src else None
    self.trg  = lnkDir.File(self.bin)
    # Prepare the environment
    if not self.env: self.env = Environment()
    if self.cfl:     self.env.AppendUnique(CCFLAGS=self.cfl)
    if self.libs:    self.env.AppendUnique(LIBS=self.libs)
    if self.defs:    self.env.AppendUnique(CPPDEFINES=self.defs)
    if self.parse:   self.env.ParseConfig(self.parse)
    if self.cStr:    self.env.Replace(CCCOMSTR=f'{self.cStr} $SOURCES')
    if self.lStr:    self.env.Replace(LINKCOMSTR=f'{self.lStr} $TARGET')
    # Setup SCons to compile
    self._build()
    AliasAdd(self.key, self.trg)
    if self.default: Default(self.key)

  def _build(self):
    self.env.Program(self.trg, self.src)

  def clone(self): 
    from copy import deepcopy # Class cloning
    return deepcopy(self)     # Create a true clone. Default behavior is just a reference
#....................................................................


#....................................................................
# Import guard
if __name__=='__main__': import sys; sys.exit(f'::MODULE-ERROR: {__file__} is only meant to be executed as a module')
#....................................................................
