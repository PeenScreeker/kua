import sys; sys.dont_write_bytecode=True
#....................................................................

from config import *
from core   import *
from clas   import *
from tools  import *

# Call core.ConfigScons() before starting the scons script
ConfigScons()

#....................................................................
# Import guard
if __name__=='__main__': import sys; sys.exit(f'::MODULE-ERROR: {__file__} is only meant to be executed as a module')
#....................................................................
