# $Id: 035eab37646641ac52d1a4c35a8cd0540867c993 $
#
# Configure's users Bourne shell (/bin/sh) or BASH environment for G4CMP
#
# Usage: . g4cmp_env.sh
#
# 20161006  Add G4WORKDIR to (DY)LD_LIBRARY_PATH
# 20170509  Define G4CMPLIB and G4CMPINCLUDE relative to G4CMPINSTALL
# 20200719  Set undefined *LD_LIBRARY_PATH; use $() in place of ``

# Identify location of script from user command (c.f. geant4make.sh)

if [ -z "$CMAKE_COMMAND" ]; then
  ISCMAKEBUILD=1
  export G4CMPINSTALL=@CMAKE_INSTALL_PREFIX@/share/G4CMP
else
  ISCMAKEBUILD=0
  export G4CMPINSTALL=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
fi
echo $G4CMPINSTALL

# Ensure that G4CMP installation is known

if [ -z "$G4CMPINSTALL" ]; then
  echo "ERROR: g4cmp_env.sh could not self-locate G4CMP installation."
  echo "Please cd to the installation area and source script again."
  return 1
fi

# If running script from source directory, assume GMake build

if [ ! ISCMAKEBUILD ]; then
  export G4CMPLIB=$G4WORKDIR/lib/$G4SYSTEM
  export G4CMPINCLUDE=$G4CMPINSTALL/library/include
else
  topdir=@CMAKE_INSTALL_PREFIX@
  export G4CMPLIB=$topdir/lib
  export G4CMPINCLUDE=$topdir/include/G4CMP
fi

# Extend library path to include G4CMP library location

export LD_LIBRARY_PATH=${G4CMPLIB}${LD_LIBRARY_PATH:+:}$LD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=${G4CMPLIB}${DYLD_LIBRARY_PATH:+:}$DYLD_LIBRARY_PATH

# Assign environment variables for runtime configuraiton

export G4LATTICEDATA=$G4CMPINSTALL/CrystalMaps
export G4ORDPARAMTABLE=$G4CMPINSTALL/G4CMPOrdParamTable.txt
