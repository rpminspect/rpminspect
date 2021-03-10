#!/bin/sh
PATH=/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/sbin:/usr/sbin

if [ "$(uname -a)" = "x86_64" ]; then
    dpkg --add-architecture i386
    apt-get -y install lib32gcc-s1
fi

apt-get -y install libterm-readline-perl-perl
apt-get update

# install and update pip and setuptools
apt-get -y install python3-pip python3-setuptools
pip-3 install --upgrade pip setuptools
