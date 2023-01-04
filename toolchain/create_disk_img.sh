#Remove Old image
rm -rf maxOS.img

#Create a 2GB image
qemu-img create maxOS.img 2G #000000000

#Partion the image
fdisk maxOS.img -u=cylinders << EOF
o
n
p
1
1
130
n
p
2
131
243
a
1
w
EOF

#Install Filesystem Tools
sudo apt-get install dosfstools

#Try and unmount the old mount points
sudo umount /mnt/maxOS_img_1 || { echo "umount failed but not to worry" ; : ; }

#Try and unmount the old mount points
sudo umount /mnt/maxOS_img_2 || { echo "umount failed but not to worry" ; : ; }

#Close the loop devices
sudo losetup -D

#Mount the image to a loop device
sudo losetup --partscan /dev/loop0 maxOS.img

## IMAGE 1

#Create a FAT32 Filesystem (partion 1)
sudo mkfs.vfat -F 32 /dev/loop0p1

#Create a directory for the mount point
sudo mkdir -p /mnt/maxOS_img_1

#Mount the image to the mount point
sudo mount -o loop /dev/loop0p1 /mnt/maxOS_img_1

#Echo some test files
cd /mnt/maxOS_img_1
sudo  echo 'this is partition 1, file 1' | sudo tee file1.txt
sudo  echo 'this is partition 1, file 2' | sudo tee file2.txt

## IMAGE 2

#Create a FAT32 Filesystem (partion 2)
sudo mkfs.vfat -F 32 /dev/loop0p2

#Create a directory for the mount point
sudo mkdir -p /mnt/maxOS_img_2

#Mount the image to the mount point
sudo mount -o loop /dev/loop0p2 /mnt/maxOS_img_2

#Echo some test files
cd /mnt/maxOS_img_2
sudo  echo 'this is partition 2, file 1' | sudo tee file1.txt
sudo  echo 'this is partition 2, file 2' | sudo tee file2.txt

#Install grub
sudo grub-install --root-directory=/mnt/maxOS_img_1 --no-floppy --modules="normal part_msdos ext2" /dev/loop0
