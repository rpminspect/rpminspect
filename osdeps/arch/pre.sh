#!/bin/sh
PATH=/usr/local/bin:/bin:/usr/bin:/usr/local/sbin:/sbin:/usr/sbin

# Install required Python modules
pacman --noconfirm -S python-pip
pip3 install --upgrade pip setuptools cpp-coveralls rpmfluff PyYAML gcovr timeout-decorator
