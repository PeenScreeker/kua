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
rootDir   = thisDir()
binDir    = rootDir/"bin"
srcDir    = rootDir/"src"
cfgDir    = srcDir/"cfg"
resDir    = rootDir/"res"
resFilter = ["skel", "dep", "src"]
rlsDir    = binDir/"releases"/modVers
rlsBase   = rlsDir/modName
rlsWin    = binDir/"release-win32-x86_64"  # TODO generate based on keywords
rlsLnx    = binDir/"release-posix-x86_64"  # TODO generate based on keywords
# Debug folders for installing and running with debugger
dbgDir    = binDir/"debug"/modVers
dbgBase   = dbgDir/modName
dbgWin    = binDir/"debug-win32-x86_64"  # TODO generate based on keywords
dbgLnx    = binDir/"debug-posix-x86_64"  # TODO generate based on keywords

#.....................................
# Files
#.............
# Config files
dscFile   = cfgDir/"description.txt"
cfgFiles  = glob(cfgDir, "*.cfg")
# Assets:
pk3Files  = glob(resDir, "*.pk3")  # Every pk3 file in the root asset folder
# Binaries
rlsLibs_w   = glob(rlsWin, "*.dll")
rlsLibs_l   = glob(rlsLnx, "*.so")
rlsEngine_w = glob(rlsWin, "*.exe")
rlsEngine_l = glob(rlsLnx, "*.x64")
# Source Code
srcFiles  = glob(srcDir, "*")
srcZip    = rlsBase/"sourceCode.zip"
# Final zip
rlsZip    = rlsDir/f"{fullName}-{modVers}.zip"

# Debug binaries
dbgLibs_w   = glob(dbgWin, "*.dll")
dbgLibs_l   = glob(dbgLnx, "*.so")
dbgEngine_w = glob(dbgWin, "*.exe")
dbgEngine_l = glob(dbgLnx, "*.x64")

#.....................................
# CLI options
#.............
addOpts("cbpdrv")
clean   = getOpt("c")  # Cleans everything before building
debug   = getOpt("d")  # Builds debug version for current system only
release = getOpt("r")  # Builds release version for current system only
build   = getOpt("b") or debug or release # Will run the build commands
pack    = getOpt("p")  # Builds release for all platforms and Packs all files into the releases folder for distribution
verbose = getOpt("v")  # Makes the output of building completely verbose
noOpts = not pack and not build and not debug and not release
if noOpts:
  build   = True  # Override to `b` when no options are given
  release = True
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
  default = not release and not debug and not distribute
  if   default: bash(f"{scCmd}", dir=str(srcDir))
  elif debug:   bash(f"{scCmd} debug", dir=str(srcDir))
  elif release: bash(f"{scCmd} release", dir=str(srcDir))
  elif distribute:
    bash(f"{scCmd} release p=linux", dir=str(srcDir))
    bash(f"{scCmd} release p=win32", dir=str(srcDir))

#.....................................
# Storing built files for easier management
#...........
def StoreRls():
  # Create folders if they don't exist
  md(rlsDir)
  md(rlsBase)
  # Copy .cfg and description.txt files
  cp(dscFile, rlsBase)
  for file in cfgFiles: cp(file, rlsBase)
  # Copy library files
  for file in rlsLibs_w: cp(file, rlsBase)
  for file in rlsLibs_l: cp(file, rlsBase)
  # Copy engine files
  for file in rlsEngine_w: cp(str(file), rlsDir)
  for file in rlsEngine_l: cp(str(file), rlsDir)
  # Assets:
  for file in pk3Files: cp(file, rlsBase)  # Copy all pk3 res files at root of res/ folder
  # Pack all res/ subfolders as separate pk3 files
  Pk3CreateAll(resDir, rlsBase, prefix=f"k.{modAlias}.", exclude=resFilter)

def StoreDbg():
  # Create folders if they don't exist
  md(dbgDir)
  md(dbgBase)
  # Copy .cfg and description.txt files
  cp(dscFile, dbgBase)
  for file in cfgFiles: cp(file, dbgBase)
  # Copy library files
  for file in dbgLibs_w: cp(file, dbgBase)
  for file in dbgLibs_l: cp(file, dbgBase)
  # Copy engine files
  for file in dbgEngine_w: cp(str(file), dbgDir)
  for file in dbgEngine_l: cp(str(file), dbgDir)
  # Assets:
  for file in pk3Files: cp(file, dbgBase)  # Copy all pk3 res files at root of res/ folder
  # Pack all res/ subfolders as separate pk3 files
  Pk3CreateAll(resDir, dbgBase, prefix=f"k.{modAlias}.", exclude=resFilter)

#.....................................
# Packing
#...........
def PackAll():
  # Zip and Copy source code
  ZipDir(srcDir, srcZip)
  # Pack everything
  Zip(glob(rlsBase, "*"), rlsZip, rlsDir)

#.....................................
if __name__=="__main__":
#.....................................
  echo(f"Executing: {__file__}")
  if clean or alwaysClean: CleanAll()
  if build:   BuildAll()
  if release: StoreRls()
  if debug:   StoreDbg()
  if pack:    PackAll()  # if -p
#.....................................

