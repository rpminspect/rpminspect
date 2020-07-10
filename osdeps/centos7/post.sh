#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Manually install rpmfluff due to pip failure on CentOS 7
git clone https://pagure.io/rpmfluff.git
cd rpmfluff
python3 setup.py build
python3 setup.py install
