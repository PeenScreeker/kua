# Multiplatform
# Quake3e SCons Compiling setup
# :::::::::::::::::::::::::::::
# Based on: Quake3 Unix Makefile 
#         | Nov '98 | Zoid <zoid@idsoftware.com>
#         | Loki Hacking by Bernd Kreimeier
#                           Ryan C. Gordon
#                           Rafael Barrero
#                           ioq3 cr3w
# Ported to SCons by:
#   2022 | sOkam! for Opensource Defrag
#   2022 | .....  for Kua, with `confy` toolset
#
#::::::::::::::::::::::::::::::::::::::
# This is a multiplatform buildsystem.
# Keep that in mind if you modify it.
# SCons takes some learning, but its way more powerful than make-based systems
#   todo: Native support for macos
#::::::::::::::::::::::::::::::::::::::

#::::::::::::::::::::::::::::::::::::::
import sys; sys.dont_write_bytecode=True  # Do not create bytecode __pycache__ folder
#::::::::::::::::::::::::::::::::::::::

# Usage and Help
# :::::::::::::::
Help("""
: Action:     command              Description
::::::::::    ::::::::             ::::::::::::
                !cmd : not implemented
: Help:       scons -h             Shows this text.
: Verbose:    !scons -V             Make the output of scons fully verbose: Formatting removed and nothing filtered.
              TEMP: Use V=1        until bugfix. There is a reported issue in SCons. 
: Cores:      scons -j NUM         Use NUM of cores for building. Overwrites the value set in the scripts
: Clean:      scons -c             Cleans default build targets. Doesn't remove folders.
              scons -c NAME        Cleans selected build targets. Doesn't remove folders.
: Build:        
    Default   scons                Builds the targets set in the `build_default` list
    Game      scons game           Builds only the game code (dynamic library), without the engine   (sgame, cgame, ui)
    Debug     scons debug          Builds debug version only     (client+server+game).  Default when `build_default` is empty
    Engine    scons engine         Builds only the engine code, without any game code
    Server    !scons server         Builds only the dedicated server code (ded), without any client code
    Release   scons release        Builds release version only   (client+server+game)
    Distrib   scons distribute     Cross-compiles and builds release versions for all supported platforms.
    All       scons all            Builds all targets (including debug and release versions of both)
: Options:
    Platform  scons p=NAME         Cross-compilation for a platform different than the current host platform
                    platform=NAME   Valid names:                     # todo: darwin
                                    Win64: 'w', 'win'                : SCons 'win32':'64bit'
                                    Win32: 'w32', 'win32'            : SCons 'win32':'32bit'
                                    Linux: 'l', 'lnx', 'linux'       : SCons 'posix':'64bit'
                                    Linux: 'l32', 'lnx32', 'linux32' : SCons 'posix':'32bit'
    Arch      scons arch=NAME      Cross-compilation for an architecture other than the current host arch
                                    Don't use, unless you know what you are doing.     #TODO: x86, arm 
                                    Defaults to x86 or x86_64, if the host platform is of a different bitsize than the one provided. Else current host arch
                                    Uses SCons default names. Valid names: (64bit) 'amd64','x86_64'  (32bit) 'x86','arm'
: Configuration:
    ./scons_local.py              Allows the user to overwrite default configuration variables set in `scons_config.py`
                                   Useful to make permanent changes locally, without losing progress during updates
                                   Copy the template file `./lib/skel/scons_local.py` as `./scons_local.py`, or create a new empty file
""")
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
############################################################


###############
# Helpers & imports
# ::::::::::::::::::

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
############################################################


###############
# Configuration
# ::::::::::::::::
from lib.helper.config_scons import *

# For configuration different than the defaults, without losing changes during updates:
#   Create a new file named "scons_local.py" in the same directory as this one
#   Assign your configuration there
#   The values active in the file will overwrite any values set in lib.scons_config      
if h.exists('scons_local.py'): from scons_local import *

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::: 
############################################################


##############
# Import guard
#::::::::::::::::::
if __name__=='__main__': import sys; sys.exit(f'::MODULE-ERROR: {__file__} is not meant to be used as a module.')
############################################################
