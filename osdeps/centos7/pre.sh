#!/bin/sh
PATH=/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/sbin:/usr/sbin

# EPEL is required
yum install -y epel-release
rpm --import /etc/pki/rpm-gpg/*
