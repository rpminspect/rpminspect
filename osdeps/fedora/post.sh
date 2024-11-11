#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
[ -d "${OSDEPS}" ] || exit 1

# Install 32-bit development files on 64-bit systems when available
case "$(uname -m)" in
    x86_64)
        dnf install -y glibc-devel.i686 glibc.i686 libgcc.i686 glibc-headers-x86
        ;;
    s390x)
        dnf install -y glibc-headers-s390
        ;;
esac

# This change to /usr/lib/rpm/macros was introduced on 24-Aug-2024 in
# commit 1a9803d0f8daf15bb706dc17783ab19589906487 to rpm, but it
# causes problems for the rpminspect test suite.  Undo the change.
sed -i -e '/^%%global\ __debug_package\ 1\\/d' /usr/lib/rpm/macros

# Update the clamav database
freshclam
