#!/usr/bin/python
import sys; sys.dont_write_bytecode=True
#.....................................
def thisPkg(): from os.path import dirname, relpath; return relpath(thisDir(), start=dirname(__file__))
# str(getsourcefile(lambda:0))
#.....................................

#.....................................
validExts = [".c", ".h", ".cpp", ".inl", ".hpp"]
#.....................................

#.....................................
def _Cformat(f, style=None): 
  from importlib import import_module
  tools = import_module(f"{__name__}.tools".replace(".cformat",""))
  from os import curdir
  cmd = "clang-format -i"
  if style: 
    style = f"-style=file:{style}"
    tools.bash(f"{cmd} {style} {f} ", dir=curdir)
  else:
    tools.bash(f"{cmd} {f}", dir=curdir)
  print(f"fmt-> {f}")
#.....................................
def getExt(file):
  from os.path import splitext
  return splitext(file)[1]
def isValid(file): bool(getExt(file) in validExts)
#.....................................
def Cformat(trg, style=None):
  from os import walk
  from os.path import isdir, isfile, join, splitext
  if isdir(trg):
    for root, dirs, files in walk(trg):
      for file in [f for f in files if isValid(f)]:
        _Cformat(join(root, file), style)
  elif isfile(trg):
    _Cformat(trg, style)
#.....................................

#.....................................
if __name__=="__main__": 
  from ..tools import getArg, echo, err
  from importlib import import_module
  tools = import_module(f"{__name__}.tools")
  src    = tools.getArg(0)
  style  = tools.getArg(1)
  if not src:    tools.err("Need to pass a target as the first parameter")
  if not style:  tools.echo("confy: Using default format style file")
  Cformat(src, style)
  tools.echo(f"confy: Done formatting {src}")
#.....................................
