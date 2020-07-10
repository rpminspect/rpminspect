#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Install rpmfluff-0.5.7.1 manually
TAG=0.5.7.1
git clone https://pagure.io/rpmfluff.git
cd rpmfluff
git checkout -b ${TAG} ${TAG}
SITE_PACKAGES="$(python3 -c "import sysconfig ; print(sysconfig.get_paths()['purelib'])")"
install -D -m 0644 rpmfluff.py ${SITE_PACKAGES}/rpmfluff.py
