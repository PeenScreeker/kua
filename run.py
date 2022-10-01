#!/usr/bin/python
import sys; sys.dont_write_bytecode=True  # Do not create bytecode __pycache__ folder
#....................................................................
print(f"confy: Starting: {__file__}")
from pathlib import Path
from src.lib.python.tools import bash, echo, cp, toExec
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
eExt   = ".x64" if b.posix else "-x64.exe"
ePath  = (rlsPath.parent/b.eName).with_suffix(eExt)  # For linking
engine = (instPath/b.eName).with_suffix(eExt)      # For running
cmd    = f"{engine}"

#....................................................................
if __name__=="__main__":
  echo(f"Executing: {__file__}")
  # Generate new links
  if gamePath.exists():
    echo(f"Relinking {gamePath} to {rlsPath}")
    gamePath.unlink()
    gamePath.symlink_to(rlsPath)
  if engine.exists():
    echo(f"Removing {engine}")
    engine.unlink()
  cp(ePath, engine)
  toExec(engine)
  if not kuaBaseq3.exists():
    kuaBaseq3.symlink_to(baseq3)

  # Run it
  echo(f'Executing: {cmd} at {str(instPath)}')
  bash(f'{cmd}', dir=str(instPath))
