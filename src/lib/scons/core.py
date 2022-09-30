import sys; sys.dont_write_bytecode=True
#....................................................................
# Confy modules
from config import *
from tools  import *
if isVerbose(): print(f"confy: Importing module {__name__}")
#....................................................................
 # SCons specific variables and methods
from SCons.Script import *
from SCons.Script import Alias, SetOption, Decider, SConsignFile
from SCons.Node import Node
from SCons.Script.SConscript import SConsEnvironment
#....................................................................
_scConfigured = False
def ConfigScons():
  global _scConfigured
  if _scConfigured: return  # Will only run once, inside site_init.py
  # Cores to build with
  from multiprocessing import cpu_count
  cores = max(2, int(cpu_count()*coresPc))
  SetOption('num_jobs', cores)
  ## Verbose
  if not scVerbose and scQuiet: SetOption('no_progress',1)  # Mute SCons progress text on non-verbose mode
  if scVerbose: Progress(': Evaluating  $TARGET\n') # SCons will echo when each file is being evaluated. TODO: Chapter 9.3
  ## Others
  Decider(scDecider) # Could be assigned per environment, but using global config instead
  SConsignFile(scDB.abspath)
  _scConfigured = True

#....................................................................
_aliasAll = []
def AliasToAll(key):
  global _aliasAll; _aliasAll += [key]
  Alias("all", _aliasAll)  # Every time this is called, the previous is overwritten
def AliasAdd(key, trg): Alias(key, trg); AliasToAll(key)
#....................................................................
# Import guard
if __name__=='__main__': import sys; sys.exit(f'::MODULE-ERROR: {__file__} is only meant to be executed as a module')
#....................................................................
