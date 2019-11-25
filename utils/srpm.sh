#!/bin/sh
#
# Copyright (C) 2019 David Cantrell <david.l.cantrell@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

PATH=/usr/bin
CWD="$(pwd)"

if [ ! -f ${CWD}/.copr/Makefile ]; then
    echo "*** Missing .copr/Makefile, exiting." >&2
    exit 1
fi

if [ ! -f ${CWD}/changelog ]; then
    ${CWD}/utils/mkrpmchangelog.sh --copr > ${CWD}/changelog
fi

make -f ${CWD}/.copr/Makefile srpm outdir=${CWD} BUILDTYPE=release
