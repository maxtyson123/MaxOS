#!/bin/bash
SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/MaxOS.sh

# Parse the args
while [ "$#" -gt "0" ]; do
  case "$1" in
    --image-path)
      IMAGE_PATH="$2"
      shift 2
      ;;
    --network-device)
      NETWORK_DEVICE="$2"
      shift 2
      ;;
    --port-forwarding-host)
      PORT_FORWARDING_HOST="$2"
      shift 2
      ;;
    --port-forwarding-guest)
      PORT_FORWARDING_GUEST="$2"
      shift 2
      ;;
    --debug)
      USE_DEBUG=1
      shift 1
      ;;
    *)
      warn "Error: Unknown argument $1"
      ;;
  esac
done

# Check if KVM is supported
USE_KVM=0
if [ -r /dev/kvm ]; then
  msg "KVM is supported on this machine."
  USE_KVM=1
fi

# Check if in WSL
IN_WSL=0
if command -v wslpath >/dev/null; then
  msg "WSL detected."
  IN_WSL=1
fi

# Get the image path
if [ -z "$IMAGE_PATH" ]; then
  msg "Using default image path."
  IMAGE_PATH="../MaxOS.img"
else

  # Check if the image path is valid
  if [ ! -f "$IMAGE_PATH" ]; then
    fail "Error: Image path $IMAGE_PATH does not exist."
  fi

fi

# If in WSL, convert the image path to a windows path
if [ "$IN_WSL" -ne "0" ]; then
  msg "Converting image path to windows path."
  IMAGE_PATH=$(wslpath -w "$IMAGE_PATH")
fi

# Check what accelerator to use
ACCELERATOR=""
if [ "$IN_WSL" -ne "0" ]; then
  msg "Using windows accelerator."
  ACCELERATOR="-accel whpx,kernel-irqchip=off -accel tcg"
else

  # Check if KVM is supported
  if [ "$USE_KVM" -ne "0" ]; then
    msg "Using KVM accelerator."
    ACCELERATOR="-enable-kvm"
  fi

fi

# Find the qemu executable
QEMU_EXECUTABLE=""
# Check if on linux or windows
if [ "$IN_WSL" -ne "0" ]; then
  msg "Using windows qemu."
  QEMU_EXECUTABLE="/mnt/c/Program Files/qemu/qemu-system-x86_64.exe"
else
  msg "Using linux qemu."
  QEMU_EXECUTABLE="qemu-system-x86_64"
fi

# Check what display type to use
DISPLAY_TYPE=""
if "$QEMU_EXECUTABLE" --display help | grep -q "sdl"; then
  msg "Using sdl display."
  DISPLAY_TYPE="-display sdl"
elif "$QEMU_EXECUTABLE" --display help | grep -q "cococa"; then
  msg "Using cocoa display."
  DISPLAY_TYPE="-display cocoa"
else
  fail "Error: No display type found."
fi

# Get the network interface
if [ -z "$NETWORK_DEVICE" ]; then
  msg "Using default network device."
  NETWORK_DEVICE="pcnet"
else
  if ! "$QEMU_EXECUTABLE" -netdev help | grep -q "$NETWORK_DEVICE"; then
    fail "Error: Network device $NETWORK_DEVICE not found."
  fi
fi

# Check if port forwarding is enabled
PORT_FORWARDING=""
if [ -n "$PORT_FORWARDING_HOST" ] && [ -n "$PORT_FORWARDING_GUEST" ]; then
  msg "Setting up port forwarding: Host $PORT_FORWARDING_HOST -> Guest $PORT_FORWARDING_GUEST"
  PORT_FORWARDING="-net user,hostfwd=tcp::$PORT_FORWARDING_HOST-:$PORT_FORWARDING_GUEST"
fi

# Check if we are debugging
DEBUG=""
if [  "$USE_DEBUG" -ne "0" ]; then
    DEBUG="-s -S"

    # WSL  needs to be disabled
    if [ "$IN_WSL" -ne "0" ]; then
      msg "WSL disabled."
      ACCELERATOR=""
    fi

    objcopy --only-keep-debug $SCRIPTDIR/../filesystem/boot/MaxOSk64 ../MaxOS.sym
    msg "Generated debug symbols"
fi

# Create the args
QEMU_ARGS=""
QEMU_ARGS="$QEMU_ARGS -m 2G"                               # 512 MB of RAM
QEMU_ARGS="$QEMU_ARGS -smp cores=4"                         # 4 cores
QEMU_ARGS="$QEMU_ARGS -serial stdio"                        # Use stdio for serial
if [  ! "$USE_DEBUG" -ne "0" ]; then
QEMU_ARGS="$QEMU_ARGS -d int"                               # Debug interrupts
fi
QEMU_ARGS="$QEMU_ARGS $DEBUG"                               # Enable debugging
QEMU_ARGS="$QEMU_ARGS $ACCELERATOR"                         # Enable acceleration
QEMU_ARGS="$QEMU_ARGS $DISPLAY_TYPE"                        # Enable display
QEMU_ARGS="$QEMU_ARGS -net nic,model=$NETWORK_DEVICE"       # Add a network device
QEMU_ARGS="$QEMU_ARGS $PORT_FORWARDING"                     # Add port forwarding
QEMU_ARGS="$QEMU_ARGS -drive file=$IMAGE_PATH,format=raw"   # Add the image as a drive
QEMU_ARGS="$QEMU_ARGS,if=ide,cache=directsync,id=disk0"     # Configure the drive to be an ide drive with direct sync caching
QEMU_ARGS="$QEMU_ARGS -no-reboot -no-shutdown"              # Don't reboot or shutdown on exit

# Run qemu
msg "Running qemu with args: $QEMU_ARGS"
"$QEMU_EXECUTABLE" $QEMU_ARGS