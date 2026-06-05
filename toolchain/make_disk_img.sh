#!/bin/bash

SCRIPTDIR=$(dirname "$BASH_SOURCE")
PROJECT_ROOT="$(cd "$SCRIPTDIR/.." && pwd)"
MOUNT_DIR="$PROJECT_ROOT/filesystem"
BACKUP_DIR="$PROJECT_ROOT/filesystem_bak"
source $SCRIPTDIR/MaxOS.sh

# TODO: Scalability for partition size and amount of partitions
# TODO: Better loop device handling
# TODO: too much if nesting
# TODO: make boot part 100mb instead

IMAGE="../MaxOS.img"

# If the disk image already exists no need to setup
if [ -f "$IMAGE" ]; then
    msg "Image already exists"
    exit 0
fi

# Prserve contents
if [ -d "$MOUNT_DIR" ]; then
    cp -r "$MOUNT_DIR" "$BACKUP_DIR" || fail "Backup failed"
fi

# Remove the disk on failure TODO: Also unmount the image, del back dir
ON_FAIL="rm -f $IMAGE"

#Create a 2GB image
qemu-img create "$IMAGE" 2G  || fail "Could not create image"

#Partion & Mount the image
dev=""
msg "Partitioning image"
if [ "$IS_MACOS" -eq 1 ]; then
   dev_arr=($(hdiutil attach -nomount ../MaxOS.img))
   dev=${dev_arr[0]}
   sudo diskutil partitionDisk $dev MBRFormat "MS-DOS FAT32" "BOOT" 1G "MS-DOS FAT32" "DATA" R
    sudo diskutil unmountDisk $dev
else

    if [ "$FILESYSTEM_TYPE" = "FAT" ]; then
      TYPE_CODE="b"  # FAT32
    else
      TYPE_CODE="83" # EXT2
    fi

  fdisk "$IMAGE" -u=cylinders << EOF
  o
  n
  p
  1
  1
  130
  t
  $TYPE_CODE
  a
  1
  n
  p
  2
  131
  243
  t
  2
  $TYPE_CODE
  w
EOF
  dev=$(sudo losetup --find --show --partscan "$IMAGE")
fi

# Get the partions
part1=""
part2=""
if [  "$IS_MACOS" -eq 1 ]; then
  part1="${dev}s1"
  part2="${dev}s2"
else
  msg "Attaching image to loop device"
  part1="${dev}p1"
  part2="${dev}p2"
fi

# Create mount directories
mkdir -p "$MOUNT_DIR/0" "$MOUNT_DIR/1" || fail "Could not create mount directories"

# Try and unmount any existing mount points
for mount_point in "$MOUNT_DIR/0" "$MOUNT_DIR/1"; do
  if mount | grep -q "$mount_point"; then
    umount "$mount_point" >/dev/null 2>&1 || warn "Couldn't unmount $mount_point"
  fi
done

# Create the images
create_fs_part() {

  local part="$1"
  local number="$2"
  msg "${part} - ${number}"

  local mount_point="$MOUNT_DIR/$number"
  mkdir -p "$mount_point" || fail "Could not create mount point"

  msg "Creating filesystem for partition $number"

  if [ "$FILESYSTEM_TYPE" = "FAT" ]; then
    if [ "$IS_MACOS" -eq 1 ]; then
        sudo mount_msdos -m 0777 -M 0777 "$part" "$mount_point" || fail "Could not mount partition $number"
    else
      mkfs.vfat -F 32 "$part" || fail "Could not create FAT32 filesystem"
      mount "$part" "$mount_point" 2>/dev/null || fail "Could not mount image to mount point"
    fi
  else
    # Format the image
    MKFS="mkfs.ext2"
    if [ "$IS_MACOS" -eq 1 ]; then
      MKFS="/opt/homebrew/opt/e2fsprogs/sbin/$MKFS"
    fi
    "$MKFS" "$part" 2>/dev/null || fail "Could not create EXT2 filesystem"

    if [ "$IS_MACOS" -eq 1 ]; then

      # Ensure tooling is setup
      if ! command -v fuse-ext2 >/dev/null 2>&1; then
         pushd "$SCRIPTDIR/external" > /dev/null
        ./ext2fuse.sh
        popd > /dev/null
      fi
      fuse-ext2 "/dev/r${part#/dev/}" "$mount_point" -o rw+,allow_other,noappledouble 2>/dev/null || fail "Could not mount image to mount point"
    else
      mount "$part" "$mount_point" 2>/dev/null || fail "Could not mount image to mount point"
    fi
  fi
}
create_fs_part "$part1" "0"
create_fs_part "$part2" "1"

# Sync
msg "Syncing filesystem"
sync

# Make sure user owns it
msg "Mod user"
sudo chown -R $(whoami):staff "$MOUNT_DIR"
chmod -R 755 "$MOUNT_DIR"

# Restore (TODO dynamic with partitions) (uses tart here to stream as one and faster)
msg "Restore bak"
if [ -d "$BACKUP_DIR" ]; then
    if [ -d "$BACKUP_DIR/0" ]; then
        cp -rX "$BACKUP_DIR/0/"* "$MOUNT_DIR/0/" || fail "Failed copying to part 0"
    fi
    if [ -d "$BACKUP_DIR/1" ]; then
        cp -rX "$BACKUP_DIR/1/"* "$MOUNT_DIR/1/" || fail "Failed copying to part 1"
    fi
fi
rm -rf "$BACKUP_DIR"

# Define grub modules
GRUB_MODULES="normal part_msdos biosdisk echo multiboot2"
if [ "$FILESYSTEM_TYPE" = "FAT" ]; then
  GRUB_MODULES="$GRUB_MODULES fat"
fi

if [ "$FILESYSTEM_TYPE" = "EXT2" ]; then
  GRUB_MODULES="$GRUB_MODULES ext2"
fi

#Install grub to the image
if [ "$IS_MACOS" -eq 1 ]; then
  msg "Installing GRUB manually on macOS"

    CORE_IMG=core.img
    GRUB_PREFIX="(hd0,msdos1)/boot/grub"
    GRUB_LOCAL_DIR="/opt/homebrew/Cellar/i686-elf-grub/2.12/lib/i686-elf/grub/i386-pc"
    GRUB_DEST="$SCRIPTDIR/../filesystem/0/boot/grub/i386-pc"

    # Build the GRUB core image
    i686-elf-grub-mkimage -O i386-pc -o $CORE_IMG -p "$GRUB_PREFIX" $GRUB_MODULES  || fail "Could not create GRUB core image"

    # Write GRUB bootloader (boot.img + core.img) to disk image
    BOOT_IMG="$GRUB_LOCAL_DIR/boot.img"
    dd if=$BOOT_IMG of=../MaxOS.img bs=446 count=1 conv=notrunc 2>/dev/null || fail "Failed to write boot.img to $dev"
    dd if=$CORE_IMG of=../MaxOS.img bs=512 seek=1 conv=notrunc 2>/dev/null || fail "Failed to write core.img to $dev"

    # Copy GRUB modules to the image
    mkdir -p "$GRUB_DEST" 2>/dev/null || fail "Could not create grub directory"
    cp -X "$GRUB_LOCAL_DIR"/*.mod "$GRUB_DEST/" || fail "Could not copy grub modules to image"    

    rm -f $core_img

    msg "grub manually installed to disk image"
    sync
else
  msg "installing grub to disk image: $dev"
  grub-install --root-directory="$MOUNT_DIR/0" --no-floppy --modules="$GRUB_MODULES" "$dev" 2>/dev/null || fail "could not install grub"
  umount "${part1}" "${part2}" 2>/dev/null || fail "could not unmount"
  losetup -d "$dev" 2>/dev/null || fail "could not setup loopbacks"
fi

# Clean up the bullshit
sudo rm -rf "$MOUNT_DIR"/0/.fseventsd "$MOUNT_DIR"/1/.fseventsd
sudo rm -rf "$MOUNT_DIR"/0/lost+found "$MOUNT_DIR"/1/lost+found
