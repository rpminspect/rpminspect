#!/bin/sh
PATH=/usr/bin

if [ "$(uname -m)" = "x86_64" ]; then
    dpkg --add-architecture i386
    apt-get -y install lib32gcc-s1
fi

apt-get -y install libterm-readline-perl-perl
apt-get -y update

# install and update pip and setuptools
apt-get -y install python3-pip python3-setuptools python3-testresources
pip3 install --upgrade pip setuptools testresources
