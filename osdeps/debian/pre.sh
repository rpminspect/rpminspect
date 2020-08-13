#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
dpkg --add-architecture i386
apt-get -y install libgcc-s1:i386
apt-get update
