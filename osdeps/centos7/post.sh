#!/bin/sh
PATH=/bin:/usr/bin:/sbin:/usr/sbin

# Install rpmfluff manually
git clone https://pagure.io/rpmfluff.git
cd rpmfluff
sed -i -e '/^\#\!/d' setup.py
python3 setup.py build
python3 setup.py install
