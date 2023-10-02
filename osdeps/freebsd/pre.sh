#!/bin/sh
PATH=/usr/bin:/usr/sbin

# /etc/make.conf
echo "DEFAULT_VERSIONS+=ssl=openssl" >> /etc/make.conf

# Bootstrap the pkg command
env IGNORE_OSVERSION=yes pkg bootstrap -y
