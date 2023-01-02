#Boot
sudo rm -rf /mnt/maxOS_img_1/boot/maxOS.bin
sudo cp maxOS.bin /mnt/maxOS_img_1/boot

#Grub Config
sudo rm -rf /mnt/maxOS_img_1/boot/grub/grub.cfg
sudo cp filesystem/boot/grub.cfg /mnt/maxOS_img_1/boot/grub

#Copy filesystem
sudo rm -rf /mnt/maxOS_img_1/bin    && sudo cp -r filesystem/bin    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/dev    && sudo cp -r filesystem/dev    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/etc    && sudo cp -r filesystem/etc    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/home   && sudo cp -r filesystem/home   /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/lib    && sudo cp -r filesystem/lib    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/media  && sudo cp -r filesystem/media  /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/opt    && sudo cp -r filesystem/opt    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/tmp    && sudo cp -r filesystem/tmp    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/usr    && sudo cp -r filesystem/usr    /mnt/maxOS_img_1
sudo rm -rf /mnt/maxOS_img_1/var    && sudo cp -r filesystem/var    /mnt/maxOS_img_1