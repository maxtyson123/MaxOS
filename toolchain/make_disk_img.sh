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
IMAGE_SIZE_GIB=2

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
qemu-img create "$IMAGE" "${IMAGE_SIZE_GIB}G" || fail "Could not create image"

#Partion & Mount the image
FDISK_HEADS=255
FDISK_SECTORS=63
SECTORS_PER_CYLINDER=$((FDISK_HEADS * FDISK_SECTORS))
FDISK_SECTOR_SIZE=512
FDISK_CYLINDERS=$((IMAGE_SIZE_GIB * 1024 * 1024 * 1024 / FDISK_SECTOR_SIZE / SECTORS_PER_CYLINDER))

PART1_START_CYL=1
PART1_END_CYL=130
PART2_START_CYL=131
PART2_END_CYL=243

case "$FILESYSTEM_TYPE" in
    FAT)  TYPE_CODE="0B" ;;
    EXT2) TYPE_CODE="83" ;;
    *)    fail "Unsupported filesystem type: $FILESYSTEM_TYPE" ;;
esac

dev=""
msg "Partitioning image"

if [ "$IS_MACOS" -eq 1 ]; then
    # Attach image without mounting its partitions
    dev_arr=($(hdiutil attach -nomount "$IMAGE")) || fail "Could not attach image"
    dev=${dev_arr[0]}

    # Reserve 1 MiB after the MBR for GRUB
    PART1_START_SECTOR=2048
    PART1_SIZE_SECTORS=$((PART1_END_CYL * SECTORS_PER_CYLINDER - PART1_START_SECTOR))

    PART2_START_SECTOR=$(((PART2_START_CYL - 1) * SECTORS_PER_CYLINDER))
    PART2_SIZE_SECTORS=$(((PART2_END_CYL - PART2_START_CYL + 1) * SECTORS_PER_CYLINDER))

    sudo fdisk -c "$FDISK_CYLINDERS" -h "$FDISK_HEADS" -s "$FDISK_SECTORS" -e "$dev" <<EOF || fail "Could not partition image"
y
reinit
edit 1
$TYPE_CODE
n
$PART1_START_SECTOR
$PART1_SIZE_SECTORS
edit 2
$TYPE_CODE
n
$PART2_START_SECTOR
$PART2_SIZE_SECTORS
flag 1
write
y
quit
EOF

    # Reattach to refresh the partition devices
    hdiutil detach "$dev" >/dev/null || fail "Could not detach image"
    dev_arr=($(hdiutil attach -nomount "$IMAGE")) || fail "Could not reattach image"
    dev=${dev_arr[0]}

else
    fdisk -H "$FDISK_HEADS" -S "$FDISK_SECTORS" -u=cylinders "$IMAGE" <<EOF || fail "Could not partition image"
o
n
p
1
$PART1_START_CYL
$PART1_END_CYL
t
$TYPE_CODE
a
1
n
p
2
$PART2_START_CYL
$PART2_END_CYL
t
2
$TYPE_CODE
w
EOF

    dev=$(sudo losetup --find --show --partscan "$IMAGE") || fail "Could not attach loop device"
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
          sudo newfs_msdos -F 32 "/dev/r${part#/dev/}" || fail "Could not create FAT32 filesystem"
          sudo mount_msdos -m 0777 -M 0777 "$part" "$mount_point" || fail "Could not mount partition $number"
      else
          mkfs.vfat -F 32 "$part" || fail "Could not create FAT32 filesystem"
          mount "$part" "$mount_point" 2>/dev/null || fail "Could not mount image to mount point"
      fi
  else
    # Format the image
    MKFS="mkfs.ext2"
    FORMAT_PART="$part"

    if [ "$IS_MACOS" -eq 1 ]; then
      MKFS="/opt/homebrew/opt/e2fsprogs/sbin/$MKFS"
      FORMAT_PART="/dev/r${part#/dev/}"
    fi

    sudo "$MKFS" -F "$FORMAT_PART" || fail "Could not create EXT2 filesystem on $FORMAT_PART"

    if [ "$IS_MACOS" -eq 1 ]; then

      # Ensure tooling is setup
      if ! command -v fuse-ext2 >/dev/null 2>&1; then
         pushd "$SCRIPTDIR/external" > /dev/null
        ./ext2fuse.sh
        popd > /dev/null
      fi
      fuse-ext2 "/dev/r${part#/dev/}" "$mount_point" -o rw+,allow_other,noappledouble 2>/dev/null || fail "Could not mount image to mount point"
    else
      sudo mount "$part" "$mount_point" 2>/dev/null || fail "Could not mount image to mount point"
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
CPF="-r"
if [ "$IS_MACOS" -eq 1 ]; then
  CPF="-rX"
fi
if [ -d "$BACKUP_DIR" ]; then
      cp "$CPF" "$BACKUP_DIR/0/"* "$MOUNT_DIR/0/" || fail "Failed copying to part 0"
      cp "$CPF" "$BACKUP_DIR/1/"* "$MOUNT_DIR/1/" || fail "Failed copying to part 1"
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
    rm -rf "$GRUB_DEST"
    mkdir -p "$GRUB_DEST" 2>/dev/null || fail "Could not create grub directory"
    cp -X "$GRUB_LOCAL_DIR"/*.mod "$GRUB_DEST/" || fail "Could not copy grub modules to image"
    chmod -R u+w "$GRUB_DEST"

    CORE_SIZE=$(stat -f%z "$CORE_IMG") || fail "Could not determine core.img size"
    EMBED_SIZE=$(((PART1_START_SECTOR - 1) * FDISK_SECTOR_SIZE))

    msg "GRUB core image: ${CORE_SIZE} bytes; embedding space: ${EMBED_SIZE} bytes"
    [ "$CORE_SIZE" -le "$EMBED_SIZE" ] || fail "GRUB core image exceeds available embedding space"

    rm -f $CORE_IMG


    msg "grub manually installed to disk image"
    sync
else
  msg "installing grub to disk image: $dev"
  sudo grub-install --root-directory="$MOUNT_DIR/0" --no-floppy --modules="$GRUB_MODULES" "$dev" 2>/dev/null || fail "could not install grub"
fi

# Clean up the bullshit
sudo rm -rf "$MOUNT_DIR"/0/.fseventsd "$MOUNT_DIR"/1/.fseventsd
sudo rm -rf "$MOUNT_DIR"/0/lost+found "$MOUNT_DIR"/1/lost+found
