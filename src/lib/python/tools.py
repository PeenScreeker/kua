#!/usr/bin/python
import sys; sys.dont_write_bytecode = True;
#.....................................

#.....................................
def nothing(): pass
def discard(x): x = x
#.....................................
def err(msg): print(f"ERR:: {msg}"); import sys; sys.exit(1)
#.....................................
def thisFile(type='abspath'): 
  from os.path import abspath, basename, dirname
  from inspect import getsourcefile
  src = str(getsourcefile(lambda:0))
  match type:
    case 'abs'  | 'abspath': return abspath(src)
    case 'name' | 'base' | 'basename': return basename(src)
    case 'dir'  | 'folder': return dirname(src)
    case _: err(f"Incorrect type keyword in function {__file__}.thisFile(): {type}")
# def thisDir(): from os.path import dirname; from inspect import getsourcefile; return dirname(str(getsourcefile(lambda:0)))
def thisDir(): import pathlib, os; return pathlib.Path(os.path.dirname(__file__))
def thisPkg(): from os.path import dirname, relpath; return relpath(thisDir(), start=dirname(__file__))
#.....................................
def isStr(trg): return isinstance(trg, str)
def isPath(trg):
  import pathlib
  return isinstance(trg, pathlib.Path)
def isUnk(trg): 
  from os.path import exists
  return True if not trg or not exists(trg) else False
def isDir(trg):
  from pathlib import Path
  if isStr(trg): return Path(trg).is_dir()
  if not isPath(trg): return False
  if isUnk(trg): return False
  return trg.is_dir()
def isFile(trg):
  from pathlib import Path
  if isStr(trg): return Path(trg).is_file()
  if not isPath(trg): return False
  if isUnk(trg): return False
  return trg.is_file()
def isExec(trg):
  if not isFile(trg): return False
  import stat
  return bool(trg.stat().st_mode & stat.S_IEXEC)
def isEmpty(trg):
  from pathlib import Path
  if isStr(trg): trg = Path(trg)
  from os.path import getsize
  from os import listdir
  unkn = isUnk(trg)
  dir  = isDir(trg)
  file = isFile(trg)
  if dir:
    empty = not listdir(str(trg))
    return True if empty or unkn else False
  elif file:
    try:    empty = getsize(str(trg))
    except: empty = True  # If file does not exist, getsize will raise an exception
    return True if empty or unkn else False
  else:  # Input is not a dir or a file. Treat it as a python variable
    return True if not trg else False
#.....................................
def toPath(trg):
  if isPath(trg): return trg
  from pathlib import Path
  if isStr(trg): return Path(trg)
  else: err(f"Cannot convert {str(trg)} to pathlib.Path()")
def toExec(trg):
  if not isPath(trg): trg = toPath(trg)
  if isExec(trg): return
  import stat
  return trg.chmod(trg.stat().st_mode | stat.S_IEXEC)
#.....................................
from optparse import OptionParser
_parser = OptionParser()
def addOpts(opts):
  for opt in opts:
    global _parser; _parser.add_option("-"+opt, action="store_true")
def getOpt(kw):
  global _parser;
  (opts,args) = _parser.parse_args()
  for (key,value) in vars(opts).items():
    if kw == key and value: return True
  return False
def getArgs():
  global _parser
  (opts,args) = _parser.parse_args()
  try:    return args
  except: return None
def getArg(idx):
  try:    return getArgs()[idx] 
  except: return None
#.....................................
def getSys():  from platform import system;  return system()
def getArch(): from platform import machine; return machine()
#.....................................
def echo(msg): print("confy:",msg)
def echos(msg, v:str|None=None): v="v" if not v else v; echo(msg) if getOpt(v) else nothing()
echo(f"Importing {__name__} module")  # TODO: How to silence this by default?
#.....................................
def glob(path, pattern): 
  from pathlib import Path
  if not isinstance(pattern, list): return Path(path).glob(pattern)
  all = []
  for it in pattern:
    all.extend(Path(path).glob(it))
  return all
#.....................................
def StrToFile(string, file, mode='w'):
  match mode:
    case 'w': f = open(file, "w+")
    case 'a': f = open(file, "a+")
    case _: err("Incorrect type keyword in function StrToFile: {type}")
  f.write(string)
  f.close()
#.....................................
def FileToFile(src, trg):
  if not isPath(src) or not isPath(trg): err("FileToFile() only accepts Path objects")
  if not isFile(src): err(f"Tried to copy {str(src)} to {str(trg)}, but source is not a file.")
  # if not isFile(trg): err(f"Tried to copy {src} to {trg}, but target is not a file.")
  echos(f"Copying {str(src)} to {str(trg)}")
  from shutil import copyfile
  copyfile(src, trg)
def FileToDir(src, trg):
  if not isPath(src) or not isPath(trg): err("FileToDir() only accepts Path objects")
  if not isFile(src): err(f"Tried to copy {str(src)} to {str(trg)}, but source is not a file.")
  if not isDir(trg):  err(f"Tried to copy {str(src)} to {str(trg)}, but target is not a dir.")
  echos(f"Copying {src} to {trg}")
  from shutil import copyfile
  from os.path import join, basename
  copyfile(src, join(trg, basename(src)))
def FileToStr(src):
  with open(src, "r") as f:
    return f.read()
def FileReplaceWords(file, table, trg=None):
  from re import sub
  src = FileToStr(file)
  trg = file if trg is None else trg
  for old, new in table.items():
    if new in src: continue
    if old in src: print(".. replacing", old, "\t", new)
    src = src.replace(old, new)
    # sub(old, new, src)
  StrToFile(src, trg, "w")
#.....................................
def DirToDir(src, trg):
  if not isPath(src) or not isPath(trg): err("DirToDir() only accepts Path objects")
  if not isDir(src): err(f"Tried to copy {src} to {trg}, but source is not a dir.")
  if not isDir(trg): err(f"Tried to copy {src} to {trg}, but target is not a dir.")
  echos(f"Copying {src} to {trg}")
  from shutil import copytree
  copytree(src, trg)
#.....................................
def rm(trg):
  from shutil import rmtree
  trg = toPath(trg)
  if isDir(trg): rmtree(trg)
  else: trg.unlink(missing_ok=True)  # Remove symlink OR file
#.....................................
def cp(src,trg):
  srcT = 'd' if isDir(src) else 'f'
  trgT = 'd' if isDir(trg) else 'f'
  ftod = srcT in 'f' and trgT in 'd'
  ftof = srcT in 'f' and trgT in 'f'
  dtod = srcT in 'd' and trgT in 'd'
  dtof = srcT in 'd' and trgT in 'f'
  src = toPath(src)
  trg = toPath(trg)
  if ftod: FileToDir(src, trg)
  if ftof: FileToFile(src, trg)
  if dtod: DirToDir(src, trg)
  if dtof: err(f"Cannot copy folder to file:  {src}  to  {trg}")
#.....................................
def cd(trg):
  if not isDir(trg): err(f"Tried to change dir to {trg}, but it is not a folder")
  echos(f"Changing folder to {trg}")
  from os import chdir
  chdir(trg)
#.....................................
def mv(src,trg): 
  echos(f"Moving {src} to {trg}")
  import shutil; shutil.move(src,trg)
#.....................................
def md(trg): 
  import os
  if not os.path.exists(trg): os.makedirs(trg)
def mdOld(trg): from pathlib import Path; Path(trg).mkdir(exist_ok=True)
#.....................................
def ListToFile(lst, file, mode="w+"):
  with open(file, mode) as f: f.writelines(lst)
def ListFromFile(file):
  with open(file, "r") as f: return f.readlines()
#.....................................
def LineInFile(string, file):
  lines = ListFromFile(file)
  for it in lines:
    if it == string: return True
  return False
def LineToFile(line, file, mode="w+"):
  with open(file, mode) as f: f.write(line)
def LineFromFile(file, pos):
  lines = ListFromFile(file)
  return lines[pos]
#.....................................
def bash(cmd, dir="", type='print'):
  from os.path import curdir
  dir = curdir if not dir else dir
  com = " ".join(list(filter(None, cmd.split(";")))).split(" ")
  import subprocess
  match type:
    case 'pipe':  result = subprocess.run(com, cwd=dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    case 'print': result = subprocess.run(com, cwd=dir)
    case _: err(f"Incorrect type keyword in function bash(): {type}")
  return result
#.....................................
def Zip(src,trg,rel=None):
  if isPath(src): src = str(src)
  if isPath(trg): trg = str(trg)
  if isPath(rel): rel = str(rel)
  from os.path import join, relpath, basename, dirname, isfile, isdir
  from os import walk
  if not rel: rel = str(dirname(trg))
  from zipfile import ZipFile, ZIP_DEFLATED
  z = ZipFile(trg, 'w', ZIP_DEFLATED)
  # Zip all files contained in the list
  srcfiles = [f for f in src if isfile(f)]
  for it in srcfiles:
    root = dirname(it)
    frel = relpath(dirname(it), rel)
    file = basename(it)
    z.write(join(root, file), arcname=join(frel, file))
  # Walk through all dirs, and zip everything in them recursively
  srcdirs  = [d for d in src if isdir(d)]
  for it in srcdirs:
    for root, dirs, files in walk(it):
      for file in files:
        frel = relpath(root, rel)
        z.write(join(root, file), arcname=join(frel, file))
  z.close()

def Pk3(list, trg, rel=None):
  if isPath(trg): trg = str(trg)
  if isPath(rel): rel = str(rel)
  from os.path import splitext
  z = splitext(trg)[0]+".zip" 
  p = splitext(trg)[0]+".pk3" 
  Zip(list, z, rel)
  mv(z,p)
#.....................................
def ZipDir(src,trg,rel=None):
  if isPath(src): src = str(src)
  if isPath(trg): trg = str(trg)
  if isPath(rel): rel = str(rel)
  from os.path import relpath, join; from os import walk
  from zipfile import ZipFile, ZIP_DEFLATED
  if not rel: rel = src
  z = ZipFile(trg, 'w', ZIP_DEFLATED)
  for root, dirs, files in walk(src):
    for file in files:
      frel = relpath(root, rel)
      z.write(join(root, file), arcname=join(frel, file))
  z.close()

def Pk3Dir(src, trg, rel=None):
  if isPath(src): src = str(src)
  if isPath(trg): trg = str(trg)
  if isPath(rel): rel = str(rel)
  from os.path import basename, splitext, dirname, join
  z = splitext(basename(trg))[0]+".zip"
  p = splitext(basename(trg))[0]+".pk3"
  ZipDir(src, z, rel)
  mv(z, join(dirname(trg), p))
#.....................................
def Pk3CreateAll(src, trg, prefix=None, exclude=None):
  if isPath(src): src = str(src)
  if isPath(trg): trg = str(trg)
  from os.path import join, basename, isdir 
  if prefix is None: prefix = "y.custom."
  dirs = [d for d in glob(src, "*") if d.is_dir() or isdir(d)]
  if exclude: dirs = [d for d in dirs if d.name not in exclude]
  for it in dirs:  # For every folder in src
    if isEmpty(it): continue # Skip empty folder
    pk3 = join(trg, f"{prefix}{basename(it)}.pk3")
    Pk3Dir(it, pk3)
#.....................................


#....................................................................
# Import guard
if __name__=='__main__': import sys; sys.exit(f'::MODULE-ERROR: {__file__} is only meant to be used as a module')
#....................................................................
