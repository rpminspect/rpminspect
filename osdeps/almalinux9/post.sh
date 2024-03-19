#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin
CWD="$(pwd)"

# Install 32-bit development files on x86_64 systems
if [ "$(uname -m)" = "x86_64" ]; then
    yum install -y glibc-devel.i686 glibc.i686 libgcc.i686
fi

# Have to build and install dash manually because it does not exist in
# EL9 as an RPM
git clone https://git.kernel.org/pub/scm/utils/dash/dash.git
cd dash || exit 1
TAG="$(git tag -l | sort -V | tail -n 1)"
git checkout -b "${TAG}" "${TAG}"
./autogen.sh
./configure --prefix=/usr/local
make V=1
make V=1 install
cd "${CWD}" || exit 1

# Update clamav database
freshclam
