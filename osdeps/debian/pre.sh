#!/bin/sh

if [ "$(uname -m)" = "x86_64" ]; then
    dpkg --add-architecture i386
    apt install -y lib32gcc-s1
fi
