#!/usr/bin/python
import sys; sys.dont_write_bytecode = True;
#.....................................
print(f"confy: Starting: {__file__}")
#.....................................
from src.lib.python.tools import *
#.....................................

#.....................................
# Configuration
#...............
# Mod    #TODO: get from mod-info.h and engine header files
modVers     = "0.35a"
modAlias    = "mp"
modName     = f"k{modAlias}"
gameName    = "kua"
fullName    = f"{gameName}-multiplayer"
eName       = "kua"  # Engine base name
alwaysClean = False

#.....................................
# Folders
def thisDir(): import pathlib, os; return pathlib.Path(os.path.dirname(__file__))
#..................
rootDir = thisDir()
binDir  = rootDir/"bin"
srcDir  = rootDir/"src"
cfgDir  = srcDir/"cfg"
resDir  = rootDir/"res"
rlsDir  = binDir/"releases"/modVers
rlsBase = rlsDir/modName
winDir  = binDir/"release-win32-x86_64"  # TODO generate based on keywords
lnxDir  = binDir/"release-posix-x86_64"  # TODO generate based on keywords

#.....................................
# Files
#.............
# Config files
dscFile   = cfgDir/"description.txt"
cfgFiles  = glob(cfgDir, "*.cfg")
# Assets:
pk3Files  = glob(resDir, "*.pk3")  # Every pk3 file in the root asset folder
# Binaries and Code
winLibs   = glob(winDir, "*.dll")
winEngine = glob(winDir, "*.exe")
lnxLibs   = glob(lnxDir, "*.so")
lnxEngine = glob(lnxDir, "*.x64")
srcFiles  = glob(srcDir, "*")
srcZip    = rlsBase/"sourceCode.zip"
# Final zip
rlsZip    = rlsDir/f"{fullName}-{modVers}.zip"

#.....................................
# CLI options
#.............
addOpts("cbpdrv")
clean   = getOpt("c")  # Cleans everything before building
build   = getOpt("b")  # Builds with default options for current system only
debug   = getOpt("d")  # Builds debug version for current system only
release = getOpt("r")  # Builds release version for current system only
pack    = getOpt("p")  # Builds release for all platforms and Packs all files into the releases folder for distribution
verbose = getOpt('v')  # Makes the output of building completely verbose
noOpts = not pack and not build and not debug and not release
if noOpts: build = True  # Override to `b` when no options are given
distribute = pack        # Builds release for all platforms when packing
#.............
# Current system
posix = True if 'Linux'   in getSys() else False
win32 = True if 'Windows' in getSys() else False
arch  = getArch()

#.....................................
# Building
#...........
from multiprocessing import cpu_count
coresPc = 0.8
cores   = int(cpu_count()*coresPc)
scCmd   = f"scons -j{cores} V={1 if verbose else 0}"

#.....................................
# Cleaning
#...........
def CleanAll():
  bash(f"{scCmd} -c all", dir=str(srcDir))
  rm(rlsDir)

#.....................................
# Building
#...........
def BuildAll():
  if build or not distribute: bash(f"{scCmd}", dir=str(srcDir))
  if debug:                   bash(f"{scCmd} debug", dir=str(srcDir))
  if release or distribute:   bash(f"{scCmd} release", dir=str(srcDir))
  # if distribute:              bash(f"{scCmd} release p=win32", dir=str(srcDir))

#.....................................
# Packing
#...........
def PackAll():
  # Create folders if they don't exist
  md(rlsDir)
  md(rlsBase)
  # Copy .cfg and description.txt files
  cp(dscFile, rlsBase)
  for file in cfgFiles: cp(file, rlsBase)
  # Copy library files
  for file in winLibs: cp(file, rlsBase)
  for file in lnxLibs: cp(file, rlsBase)
  # Copy engine files
  for file in winEngine: cp(str(file), rlsDir)
  for file in lnxEngine: cp(str(file), rlsDir)
  # Assets:
  for file in pk3Files: cp(file, rlsBase)  # Copy all pk3 res files at root of res/ folder
  Pk3CreateAll(resDir, rlsBase, prefix=f"k.{modAlias}.", exclude=["skel", "dep", "src"])  # Pack all res/ subfolders as separate pk3 files
  # Zip and Copy source code
  ZipDir(srcDir, srcZip)
  # Pack everything
  Zip(glob(rlsBase, "*"), rlsZip, rlsDir)

#.....................................
if __name__=="__main__":
#.....................................
  echo(f"Executing: {__file__}")
  if clean or alwaysClean: CleanAll()
  BuildAll()
  if pack:   # if -p
    PackAll()
#.....................................

