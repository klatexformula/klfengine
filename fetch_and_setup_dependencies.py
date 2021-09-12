#!/usr/bin/env python

import os.path
import sys
import shutil
import subprocess
import datetime
import argparse

import logging
logging.basicConfig(level=logging.DEBUG, format='%(message)s')

logger = logging.getLogger(__name__)


# will be set to a file within DEPS_SRCDIR
logfilename = None
logfile = None


#
# Define dependency GIT URL's and VERSIONS
#
# The versions are hard-coded based on current versions.  Update as you like!
#

DEPS_INFO = {
    'nlohmann-json': dict(
        giturl='https://github.com/nlohmann/json.git',
        gittag='v3.9.0',
        cmake=dict(vars=dict(
            JSON_BuildTests='off'
        )),
    ),
    'mpark-variant': dict(
        giturl='https://github.com/mpark/variant.git',
        gittag='v1.4.0',
    ),
    'gulrak-filesystem': dict(
        giturl='https://github.com/gulrak/filesystem.git',
        gittag='v1.3.2',
        cmake=dict(vars=dict(
            GHC_FILESYSTEM_BUILD_TESTING='off',
            GHC_FILESYSTEM_BUILD_EXAMPLES='off',
            GHC_FILESYSTEM_WITH_INSTALL='on',
        )),
    ),
    'arun11299-cppsubprocess': dict(
        giturl='https://github.com/arun11299/cpp-subprocess.git',
        gittag='v2.0',
        cmake=dict(vars=dict(
            BUILD_TESTING='off',
        )),
    ),
    # 'sheredom-subprocess': dict(
    #     giturl='https://github.com/sheredom/subprocess.h.git',
    #     gitcommit='9882bf9c0ed5f17c28497b5dd72ccd242bd18ef8', # Mar 22, 2021
    #     only_copy_files=[
    #         ('subprocess.h', 'include/sheredom/subprocess.h'),
    #     ]
    # ),
    'catch2': dict(
        giturl='https://github.com/catchorg/Catch2.git',
        gittag='v2.13.0',
        cmake=dict(vars=dict(
            CATCH_BUILD_TESTING='off',
            CATCH_BUILD_EXAMPLES='off',
            CATCH_BUILD_EXTRA_TESTS='off',
        )),
    ),
}


#
# Tools to run a process with some default options
#

def runprocess(cmd, cwd, **kwargs):
    progname = os.path.basename(cmd[0])
    try:
        logger.debug("Running %r [in folder %s] ...", cmd, cwd)
        print("\n*** Running: {!r} [in folder {}] ***\n".format(cmd, cwd),
              file=logfile)
        logfile.flush()
        subprocess.run(cmd, stdout=logfile, stderr=logfile, check=True, cwd=cwd, **kwargs)
        logfile.flush()
        logger.debug("   ... %s success", progname)
        print("*** {} Success ***".format(progname), file=logfile)
    except subprocess.CalledProcessError as e:
        logger.error("{}\nSee ‘{}’ for details.\n\n".format(e, logfilename),
                     exc_info=True)
        raise


# Default commands for GIT, CMAKE, MAKE -- use existing values from
# corresponding upper-case environment variables, or try to find the executable
CMDS = {
    tool: os.environ.get(tool.upper(), shutil.which(tool))
    for tool in ('git', 'cmake', 'make')
}


class InstallationEnvironment:
    def __init__(self, source_root_dir, prefix_install_dir):
        self.source_root_dir = source_root_dir
        self.prefix_install_dir = prefix_install_dir

#
# Base classes and implementations for our installer tool
#
class BaseDependencyInstaller:
    def __init__(self, depname, depinfo, instenv):
        self.depname = depname
        self.depinfo = depinfo
        self.instenv = instenv
        self.srcdir = os.path.join(self.instenv.source_root_dir, self.depname)
        self.srcbuilddir = os.path.join(self.srcdir, "build")
        if os.path.exists(self.srcdir):
            logger.error(
                "Please remove (or rename) the existing download ‘{}’ before "
                "running this script."
                .format(self.srcdir)
            )
            # don't exit, do that when we're actually asked to install

    def install_pre(self):
        if os.path.exists(self.srcdir):
            # error message already shown
            logger.error("Path ‘%s’ already exists, aborting", self.srcdir)
            sys.exit(2)
            
    def install_fetch(self):
        # git clone

        gitopts = [ '--depth=1' ]

        if 'gittag' in self.depinfo:
            gitopts = gitopts + [
                '--depth=1',
                '--branch',
                self.depinfo['gittag'],
            ]
        elif 'gitcommit' in self.depinfo:
            gitopts = gitopts + [
                '-n'
            ]

        runprocess(
            [ CMDS['git'],
              'clone',
              self.depinfo['giturl'],
              *gitopts,
              self.depname ],
            cwd=self.instenv.source_root_dir,
        )

        if 'gitcommit' in self.depinfo:
            # if we have a commit hash (can't be used with clone --branch XXX),
            # then we need to check it out now.
            runprocess(
                [ CMDS['git'],
                  'checkout',
                  self.depinfo['gitcommit'], ],
                cwd=self.srcdir,
            )
        
    def install_post(self):
        logger.info("Successfully installed {}".format(self.depname))

    def install(self):
        self.install_pre()
        self.install_fetch()
        self.install_compile_and_install()
        self.install_post()

class CopyFilesDependencyInstaller(BaseDependencyInstaller):
    def install_compile_and_install(self):
        logger.info("Copying files for {}".format(self.depname))
        print("\n*** Copying files for {}:\n".format(self.depname), file=logfile)
        for fnsrc, fndest in self.depinfo['only_copy_files']:
            a = os.path.join(self.srcdir, fnsrc)
            b = os.path.join(self.instenv.prefix_install_dir, fndest)
            bdir = os.path.realpath(os.path.dirname(b))
            print("    - {} -> {}\n        in [{}]".format(a, b, bdir), file=logfile)
            os.makedirs(bdir, exist_ok=True)
            shutil.copy(a, b)

class StandardDependencyInstaller(BaseDependencyInstaller):
    def install_compile_and_install(self):
        self.install_configure()
        self.install_make()
        self.install_make_install()

    def install_configure(self):
        # make source build folder
        if not os.path.exists(self.srcbuilddir):
            os.mkdir(self.srcbuilddir)

        cmakevars = self.depinfo.get('cmake', {}).get('vars', {})

        # run CMake
        runprocess(
            [ CMDS['cmake'],
              '..',
              "-DCMAKE_INSTALL_PREFIX={}".format(
                  os.path.abspath(self.instenv.prefix_install_dir)),
              *[ "-D{}={}".format(k, v) for k, v in cmakevars.items() ] ],
            cwd=self.srcbuilddir,
        )

    def install_make(self):
        # by default, we only run 'make' separately before 'make install'
        # if make=True is set in the depinfo
        if self.depinfo.get('make', False):
            # make install
            runprocess(
                [ CMDS['make'], 'install' ],
                cwd=self.srcbuilddir
            )

    def install_make_install(self):
        # make install
        runprocess(
            [ CMDS['make'], 'install' ],
            cwd=self.srcbuilddir
        )

#
# Create appropriate installer object to use based on the DEPS_INFO information
#
def get_dependency_installer(depname, depinfo, instenv):
    if 'only_copy_files' in depinfo:
        return CopyFilesDependencyInstaller(depname, depinfo, instenv)

    return StandardDependencyInstaller(depname, depinfo, instenv)



#
# Folders where dependencies will be downloaded, and where they will be
# installed
#
_default_source_root_dir = os.path.join('.', 'deps_src')
_default_prefix_install_dir = os.path.join('.', 'deps_install')

#
# Parse arguments
#

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument(
    '--deps',
    action='store',
    help="Specify dependencies to install as a comma-separated list.  The possible "
    "dependencies you can choose from are:\n{}".format(
        ", ".join(["‘{}’".format(k) for k in DEPS_INFO.keys()])
    ),
    default=",".join(DEPS_INFO.keys())
)
arg_parser.add_argument(
    '--prefix',
    action='store',
    help="Where to install the dependency headers (by default, in "
    "a local 'deps_install' folder)",
    default=_default_prefix_install_dir,
)
arg_parser.add_argument(
    '--download-dir',
    action='store',
    help="Where to download and run build scripts for the dependencies (by default, in "
    "a local 'deps_src' folder)",
    default=_default_source_root_dir,
)
arg_parser.add_argument(
    '--local',
    action='store_true',
    help="Use this option when fetching the dependencies for your own project.  This option "
    "disables the check that makes sure that this script is run from the root directory of "
    "klfengine's source tree.  The deps_src & deps_install folders will be created "
    "relative to the current working directory.",
    default=False
)
args = arg_parser.parse_args()



#
# Ensure we're running in the root directory
#
if not os.path.exists('include/klfengine/klfengine') and not args.local:

    print("Please run this script in the root directory of the klfengine "
          "sources (or use --local).",
          file=sys.stderr)
    sys.exit(1)
    
    

#
# Ensure target directories exist
#
instenv = InstallationEnvironment(
    source_root_dir=args.download_dir,
    prefix_install_dir=args.prefix
)
logger.info("Downloading to %s and installing to %s",
            instenv.source_root_dir, instenv.prefix_install_dir)
os.makedirs(os.path.realpath(instenv.source_root_dir), exist_ok=True)
os.makedirs(os.path.realpath(instenv.prefix_install_dir), exist_ok=True)

#
# Set up log file
#
logfilename = os.path.join(instenv.source_root_dir,
                           "fetch_and_setup_dependencies.log")
logfile = open(logfilename, 'w')

print("*** Ran fetch_and_setup_dependencies on {}\n".format(datetime.datetime.now()),
      file=logfile)

#
# Install dependencies by creating dependency-installer objects
#


deplist = [x.strip() for x in args.deps.split(',')]

depinstallers = [
    get_dependency_installer(depname, DEPS_INFO[depname], instenv)
    for depname in deplist
]

logger.info("Will install dependencies: %s", " ".join(deplist))

for di in depinstallers:
    di.install()



logger.info("Done.")
