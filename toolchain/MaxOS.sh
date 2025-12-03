err () {
  printf "\e[31m[ERROR] $1 \e[39m\n"
}

msg () {
  printf "\e[36m[INFO] $1 \e[39m\n"
}

rpcgen () {
  printf "\e[36m[RPCGEN] $1 \e[39m\n"
}

warn () {
  printf "\e[33m[WARN] $1 \e[39m\n"
}

success () {
  printf "\e[32m[SUCCESS] $1 \e[39m\n"
}

fail () {

  # Print error message
  err "$1"

  # If there is something to do on fail, do it (and only once)
  if [ -z "$FAILING" ]; then
    FAILING=1
    if [ ! -z "$ON_FAIL" ]; then
      $ON_FAIL
    fi
  fi

  # Exit with error code
  if [ $? -eq 0 ]; then
    exit 1
  else
    exit $?
  fi
}

SED_EXC=sed
STAT_EXC=stat

MOUNT_DIR="/mnt"

# Filesystem type: "FAT" or "EXT2"
FILESYSTEM_TYPE="EXT2"

# Set a variable if this is MacOS
IS_MACOS=0
if [[ ($(uname) == "Darwin") ]]; then
  IS_MACOS=1
  msg "Running on MacOS"

  #Set the path to the brew binary
  BREW_PATH=$(which brew)
  if [ -z "$BREW_PATH" ]; then
    err "Brew not found, please install brew"
    exit 1
  fi

  # Ensure use of gnu tools
  SED_EXC=gsed
  STAT_EXC=gstat

  MOUNT_DIR="/Volumes"
fi