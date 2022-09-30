#!/usr/bin/python
import sys; sys.dont_write_bytecode=True  # Do not create bytecode __pycache__ folder
#....................................................................
print(f"confy: Executing: {__file__}")
from pathlib import Path
from src.lib.python.tools import bash, echo, cp
from importlib import import_module
b = import_module(".", package="build")
#....................................................................

#....................................................................
# Config
gamesDir  = Path("/app/vg")
baseq3    = gamesDir/"defrag"/"baseq3"
#....................................................................


#....................................................................
# Folders
instPath  = gamesDir/b.gameName
gamePath  = instPath/b.modName
rlsPath   = Path(b.rlsBase)
kuaBaseq3 = instPath/"baseq3/"

#....................................................................
# Create folders
if not instPath.exists(): instPath.mkdir(exist_ok=True)

#....................................................................
# Linking
canLink  = True if not gamePath.exists() else gamePath.is_symlink()
try:    linkPath = str(gamePath.readlink())
except: linkPath = ""
linkNew = b.modVers not in linkPath if canLink else None # Will relink if the build version is different than the link

#....................................................................
eExt   = ".x64" if b.posix else "-x64.exe"
ePath  = (rlsPath.parent/b.eName).with_suffix(eExt)  # For linking
engine = (instPath/b.eName).with_suffix(eExt)      # For running
cmd    = f"{engine}"

#....................................................................
if __name__=="__main__":
  # Generate new links
  if canLink and linkNew:
    if linkPath: gamePath.unlink()
    gamePath.symlink_to(rlsPath)
  if engine.exists() and engine.is_symlink() and linkNew: 
    engine.unlink()
  if not engine.exists():
    # engine.symlink_to(ePath)
    cp(ePath, engine)
  if not kuaBaseq3.exists():
    kuaBaseq3.symlink_to(baseq3)

  # Run it
  echo(f'confy: Executing: {cmd} at {str(instPath)}')
  bash(f'{cmd}', dir=str(instPath))
