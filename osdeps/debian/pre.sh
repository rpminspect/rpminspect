#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
dpkg --add-architecture i386
apt-get -y install libterm-readline-perl-perl
apt-get -y install lib32gcc-s1
apt-get update
