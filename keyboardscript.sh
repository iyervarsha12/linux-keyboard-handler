#!/bin/sh
insmod ioctlqueue.ko
gcc -o ioctlqueuetest.exe ioctlqueuetest.c
./ioctlqueuetest.exe
rmmod ioctlqueue