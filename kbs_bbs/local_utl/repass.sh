#! /bin/sh
gcc -o repass repass.c
gcc -o rehome rehome.c rehome.o
cp /home/system/bbs/.PASSWDS /home/system/bbs/.PASSWDS.old
repass
cp /home/system/bbs/.PASSWDS.tmp /home/system/bbs/.PASSWDS
chown bbs.bbs /home/system/bbs/.PASSWDS
rehome
