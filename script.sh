#!/bin/sh -x
sudo newfs -U /dev/ada1 #we made a new disk called cs_414_1.vdi which has 64mb. That new disk is going to act as a new partition which is ada1 (our original disk is ada0). This command creates a new filesystem for that disk, with all of the configuration parameters the superblock contains. Newfs creates the superblock for this filesystem.
sudo mount /dev/ada1 /newdisk #we need to mount this filesystem on a directory, and so we make a directory to be the mountpoint for the new disk we made. Once the filesystem is mounted, we can create files on this new filesystem (does that mean the output.txt file I make has to be in /newdisk?). Then we unmount to get rid of any caching the system did and dd our output.txt file into some disk image that we can xxd and read and within that xxd file we want to figure out what offset the superblock starts at. Once we know that, we can do all the stuff we talked about in his hours.

#any filesystem operation we want to do on this new filesystem has to be done under its mountpoint (/newdisk). That is why any filesystem operation we do for our original filesystem is done under / because / is the mountpoint for ada0p2 which is the partition we are working on. To see the raw bits of a filesystem, we need to unmount it, and look at the partition itself (/dev/ada1 for this new filesystem, /dev/ada0p2 for the original).
#
sudo chmod 777 /newdisk

cd /newdisk

mkdir dir1
cd dir1
mkdir dir1.1
echo goodbye > dir1.1/file2
mkdir dir1.2
mkdir dir1.3

cd ../
echo vroom > file1
echo mortality > dead
echo meat > beef
echo HUGE > big_file

echo hello > output.txt

cd /newdisk

find . > /home/sartaj/cs414/assignment3/found.txt

cd

cd cs414/assignment3

sudo umount /newdisk

sudo dd if=/dev/ada1 of=ada1.img

xxd ada1.img > ada1.hex
