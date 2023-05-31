#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Install 32-bit development files on 64-bit systems when available
case "$(uname -m)" in
    x86_64)
        dnf5 install -y glibc-devel.i686 glibc.i686 libgcc.i686 glibc-headers-x86
        ;;
    s390x)
        dnf5 install -y glibc-headers-s390
        ;;
esac

# Update the clamav database
freshclam
