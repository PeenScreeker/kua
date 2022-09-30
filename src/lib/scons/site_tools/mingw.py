import sys; sys.dont_write_bytecode=True  # Do not create bytecode __pycache__ folder
import os
import os.path

import SCons.Action
import SCons.Builder
import SCons.Tool
import SCons.Util

# Prefixes to search for:
pfx64 = ['mingw-w64-','x86_64-w64-mingw32-', 'amd64-mingw32msvc-']
pfx32 = ['mingw32-',
         'i386-mingw32msvc-', 'i486-mingw32msvc-', 'i586-mingw32msvc-',
         'i686-mingw32msvc-', 'i686-pc-mingw32-',  'i686-w64-mingw32-', ]
#prefixes = pfx64 #+ pfx32
def _find(env,bits):
  if   bits == 64: prefixes = pfx64
  elif bits == 32: prefixes = pfx32
  else: sys.exit(f'::ERR Trying to find an incorrect bit value for MinGW environment')
  for prefix in prefixes:
    # First search in the SCons path and then the OS path:
    if env.WhereIs(prefix + "gcc") or SCons.Util.WhereIs(prefix + "gcc"):
      return prefix
  return ""
def exists(env,bits): return bool(_find(env,bits))


# Define the generator function
def shlib_generator(target, source, env, for_signature):
  # Create the dll linker command
  cmd = SCons.Util.CLVar(["$SHLINK", "$SHLINKFLAGS"])
  # Add dll output argument
  dll = env.FindIxes(target, "SHLIBPREFIX", "SHLIBSUFFIX")
  if dll: cmd.extend(["-o", dll])
  cmd.extend(["$SOURCES", "$_LIBDIRFLAGS", "$_LIBFLAGS"])
  # Add implib flag
  implib = env.FindIxes(target, "LIBPREFIX", "LIBSUFFIX")
  if implib: cmd.append("-Wl,--out-implib," + implib.get_string(for_signature))
  # Add output-def flag for the target
  def_target = env.FindIxes(target, "WIN32DEFPREFIX", "WIN32DEFSUFFIX")
  if def_target: cmd.append("-Wl,--output-def," + def_target.get_string(for_signature))
  # Return the command data, stored in a list
  return [cmd]
# Define the generator action
shlib_action = SCons.Action.Action(shlib_generator, generator=1)


# Define the emitter function
def shlib_emitter(target, source, env):
  # Create the dll file target
  dll = env.FindIxes(target, "SHLIBPREFIX", "SHLIBSUFFIX")
  no_import_lib = env.get("no_import_lib", 0)
  if not dll:
    raise SCons.Errors.UserError("A shared library should have exactly one target with suffix: %s" % env.subst("$SHLIBSUFFIX"))
  # If we should import lib, and there is a target with this Ixes:
  if not no_import_lib and not env.FindIxes(target, "LIBPREFIX", "LIBSUFFIX"):
    # Append an import library to the list of targets.
    target.append(env.ReplaceIxes(dll, "SHLIBPREFIX", "SHLIBSUFFIX", "LIBPREFIX", "LIBSUFFIX"))
  # Append a def file to the target, if there isn't already a def file target, or a def file source.
  def_source = env.FindIxes(source, "WIN32DEFPREFIX", "WIN32DEFSUFFIX")
  def_target = env.FindIxes(target, "WIN32DEFPREFIX", "WIN32DEFSUFFIX")
  if not def_source and not def_target:
    target.append(env.ReplaceIxes(dll, "SHLIBPREFIX", "SHLIBSUFFIX", "WIN32DEFPREFIX", "WIN32DEFSUFFIX"))
  # Return the shlib data, stored in a tuple
  return (target, source)


# Define the .rc scanner, builder and action
res_action = SCons.Action.Action("$RCCOM", "$RCCOMSTR")
res_builder = SCons.Builder.Builder(action=res_action, suffix=".o", source_scanner=SCons.Tool.SourceFileScanner)
SCons.Tool.SourceFileScanner.add_scanner(".rc", SCons.Defaults.CScan)


def setup(env,bits):
  # Search for mingw, and add it to the environment PATH
  mingw_prefix = _find(env,bits)
  if mingw_prefix:
    mingw_dir = os.path.dirname(env.WhereIs(mingw_prefix + "gcc") or SCons.Util.WhereIs(mingw_prefix + "gcc"))
    # The mingw bin directory must be added to the path:
    path = env["ENV"].get("PATH", [])
    if not path: path = []
    if SCons.Util.is_String(path): path = path.split(os.pathsep)
    env["ENV"]["PATH"] = os.pathsep.join([mingw_dir] + path)
  if not exists(env,bits): import sys; sys.exit(f'::ERR Trying to setup mingw for env: {env}, but MinGW wasnt found.')
  # Most of mingw is the same as gcc and friends...
  gnu_tools = ["gcc", "g++", "gnulink", "ar", "gas"]
  for tool in gnu_tools: SCons.Tool.Tool(tool)(env)
  # ... but a few things differ:
  env["CC"]              = mingw_prefix + "gcc"
  env["SHCCFLAGS"]       = SCons.Util.CLVar("$CCFLAGS")
  env["CXX"]             = mingw_prefix + "g++"
  env["SHCXXFLAGS"]      = SCons.Util.CLVar("$CXXFLAGS")
  env["SHLINKFLAGS"]     = SCons.Util.CLVar("$LINKFLAGS -shared")
  env["SHLINKCOM"]       = shlib_action
  env.Append(SHLIBEMITTER=[shlib_emitter])
  # This line isn't required and breaks C++ linking
  # env['LINK'] = mingw_prefix + 'g++'
  env["AS"]              = mingw_prefix + "as"
  env["AR"]              = mingw_prefix + "ar"
  env["RANLIB"]          = mingw_prefix + "ranlib"
  env["WIN32DEFPREFIX"]  = ""
  env["WIN32DEFSUFFIX"]  = ".def"
  env["SHOBJSUFFIX"]     = ".o"
  env["STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME"] = 1
  # RC
  env["RC"]              = mingw_prefix + "windres"
  env["RCFLAGS"]         = SCons.Util.CLVar("")
  env["RCINCFLAGS"]      = "$( ${_concat(RCINCPREFIX, CPPPATH, RCINCSUFFIX, __env__, RDirs, TARGET)} $)"
  env["RCINCPREFIX"]     = "--include-dir "
  env["RCINCSUFFIX"]     = ""
  env["RCCOM"]           = "$RC $RCINCFLAGS $RCINCPREFIX $SOURCE.dir $RCFLAGS -i $SOURCE -o $TARGET"
  env["BUILDERS"]["RES"] = res_builder
  # Override incorrect platform settings.
  env["OBJPREFIX"]       = ""
  env["OBJSUFFIX"]       = ".o"
  env["LIBPREFIX"]       = "lib"
  env["LIBSUFFIX"]       = ".a"
  env["SHOBJPREFIX"]     = "$OBJPREFIX"
  env["SHOBJSUFFIX"]     = "$OBJSUFFIX"
  env["PROGPREFIX"]      = ""
  env["PROGSUFFIX"]      = ".exe"
  env["LIBPREFIX"]       = ""
  env["LIBSUFFIX"]       = ".lib"
  env["SHLIBPREFIX"]     = ""
  env["SHLIBSUFFIX"]     = ".dll"
  env["LIBPREFIXES"]     = ["$LIBPREFIX"]
  env["LIBSUFFIXES"]     = ["$LIBSUFFIX"]

# Usage:   mingw.NewEnvironment() 
#   Returns an environment configured for mingw
def NewEnvironment(bits):
  from SCons.Script import Environment
  env = Environment()
  setup(env,bits)
  return env


#....................................................................
# Import guard
if __name__=='__main__': import sys; sys.exit(f'::MODULE-ERROR: {__file__} is only meant to be executed as a module')
#....................................................................
