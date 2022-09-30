import sys; sys.dont_write_bytecode=True  # Do not create bytecode __pycache__ folder
#....................................................................
from SCons.Script import ARGUMENTS
def isVerbose(): return bool(ARGUMENTS.get('V') == '1')
if isVerbose(): print(f"confy: Importing module {__name__}")
#....................................................................
from SCons.Script import VariantDir, Glob
from SCons.Node import Node
#....................................................................
def LinkDir(src, trg):  # trg=src :: Make trg a virtual copy of src
  VariantDir(trg,src, duplicate=0)
  if isVerbose(): print(f'confy: Linked {src} to {trg}')
  return trg
def MapDir (src, trg):  # trg=src :: Make trg a copy of src, that contains a duplicate of src when it was compiled
  VariantDir(trg,src, duplicate=1)
  if isVerbose(): print(f'confy: Mapped {src} to {trg}')
  return trg
#....................................................................
def isNode(src): return isinstance(src, Node)
def isNodeGlob(lst): return any(isNode(it) for it in lst)
def GlobBasename(pattern, dir=None, rel=None): 
  from os.path import basename, join, relpath, abspath
  if dir:
    if isNode(rel):       rel = rel.abspath
    elif not isNode(rel): rel = abspath(rel) if rel else dir
    if isNode(dir):       dir = relpath(dir.abspath, rel.abspath if isNode(rel) else rel)
  pattern = pattern if not dir else join(dir, pattern)
  return [basename(f.abspath) for f in Glob(pattern)]
#....................................................................


#....................................................................
# Import guard
if __name__=='__main__': import sys; sys.exit(f'::MODULE-ERROR: {__file__} is only meant to be executed as a module')
#....................................................................
