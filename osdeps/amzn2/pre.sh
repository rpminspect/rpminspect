#!/bin/sh
PATH=/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/sbin:/usr/sbin

# Enable the EPEL repo
amazon-linux-extras install epel -y

# Make sure all the GPG keys are imported
rpm --import /etc/pki/rpm-gpg/*
