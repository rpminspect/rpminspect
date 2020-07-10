#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Install rpmfluff manually
git clone https://pagure.io/rpmfluff.git
cd rpmfluff
SITE_PACKAGES="$(python3 -c "import sysconfig ; print(sysconfig.get_paths()['purelib'])")"
install -D -m 0644 rpmfluff/rpmfluff.py ${SITE_PACKAGES}/rpmfluff.py
