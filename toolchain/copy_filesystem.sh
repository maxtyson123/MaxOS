#!/bin/bash
source ./maxOS.sh

msg "Copying boot files to image"

#Boot
sudo rm -rf /mnt/maxOS_img_1/boot/maxOS.bin || warn "Could not remove old kernel"
sudo cp ../maxOS.bin /mnt/maxOS_img_1/boot     || fail "Could not copy kernel"

#Grub Config
sudo rm -rf /mnt/maxOS_img_1/boot/grub/grub.cfg || warn "Could not remove old grub config"
sudo cp ../filesystem/boot/grub.cfg /mnt/maxOS_img_1/boot/grub || fail "Could not copy grub config"

#Copy filesystem
msg "Copying filesystem to image"
sudo rm -rf /mnt/maxOS_img_1/bin    && sudo cp -r ../filesystem/bin    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/dev    && sudo cp -r ../filesystem/dev    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/etc    && sudo cp -r ../filesystem/etc    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/home   && sudo cp -r ../filesystem/home   /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/lib    && sudo cp -r ../filesystem/lib    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/media  && sudo cp -r ../filesystem/media  /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/opt    && sudo cp -r ../filesystem/opt    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/tmp    && sudo cp -r ../filesystem/tmp    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/usr    && sudo cp -r ../filesystem/usr    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/var    && sudo cp -r ../filesystem/var    /mnt/maxOS_img_1