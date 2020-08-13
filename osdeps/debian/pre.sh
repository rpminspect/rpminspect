#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
dpkg --add-architecture i386
dpkg --configure -a
apt-get -y install libgcc-s1:i386
apt-get update
