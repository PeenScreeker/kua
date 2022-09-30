import sys; sys.dont_write_bytecode=True  # Do not create bytecode __pycache__ folder
#....................................................................
# Confy modules
from tools import *
if isVerbose(): print(f"confy: Importing module {__name__}")
#....................................................................

from SCons.Script import Dir

#....................................................................
# Defaults
rootDir = Dir("..")
binDir  = rootDir.Dir("bin")
srcDir  = rootDir.Dir("src")
libDir  = srcDir.Dir("lib")

ccstring  = "CC"
ldstring  = "LD"
cflLess  = [
  # Default in Q3 makefile:
  '-Wall','-Wstrict-prototypes',
  # New by sOkam:
  '-std=c11', '-Wpedantic', '-pedantic',  # Enforce ISO C99 standard
  ]
cflNoErr = cflLess +[
  '-Wextra',
  '-Wdouble-promotion',  # Warn when a float is promoted to double
  '-m64',
  # Recommended:
  '-Wmissing-prototypes','-Wmisleading-indentation','-Wduplicated-cond','-Wold-style-definition',
  '-Wconversion','-Wshadow','-Winit-self','-Wfloat-equal','-Wcast-align=strict',
  '-Wformat-overflow=2','-Wformat-truncation=2',
  '-fdiagnostics-format=text','-fdiagnostics-minimum-margin-width=5',#'-Wwrite-strings',
  ]
cflErr = ['-Werror', '-pedantic-errors',]
ccflags = cflNoErr + cflErr
#....................................................................


#....................................................................
# SCons config
coresPc   = 0.8                       # Percentage of CPU cores to use during compiling
scQuiet   = True                      # When True, make scons behave as if it was called with `scons -Q`
scDecider = 'MD5-timestamp'           # First timestamp, then MD5. SCcons default = 'MD5' = 'content'. Makefile default = timestamp = 'make'
scDir     = binDir.Dir('scons')       # Folder where the scons database will be stored
scDB      = scDir.File('scons-sign')  # Scons database file
scVerbose = isVerbose()
#....................................................................


#....................................................................
# For configuration different than the defaults, without losing changes during updates:
#   Create a new file named "scons_config.py" in the same directory as your SConstruct file
#   Assign your configuration there
#   The values active in that file will overwrite any values set in this one
from os.path import exists
if exists('scons_config.py'): from scons_config import *
#....................................................................


#....................................................................
# Import guard
if __name__=='__main__': import sys; sys.exit(f'::MODULE-ERROR: {__file__} is only meant to be executed as a module')
#....................................................................
