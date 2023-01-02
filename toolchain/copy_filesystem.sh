#Boot
sudo rm -rf /mnt/maxOS_img_1/boot/maxOS.bin
sudo cp maxOS.bin /mnt/maxOS_img_1/boot

#Grub Config
sudo rm -rf /mnt/maxOS_img_1/boot/grub/grub.cfg
sudo cp filesystem/boot/grub.cfg /mnt/maxOS_img_1/boot/grub

