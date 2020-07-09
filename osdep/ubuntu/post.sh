#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
apt-get -y install linux-headers-$(uname -r)
