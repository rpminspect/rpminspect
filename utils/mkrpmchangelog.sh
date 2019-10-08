#!/bin/sh
#
# Generate %changelog entries for an RPM spec file based on the
# git commits for the project.  Requires consistent usage of
# tagging for releases.
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

LATEST_TAG="$(git tag --sort=taggerdate | tail -n 1)"

git log --format=%s ${LATEST_TAG}.. | tac | while read line ; do
    first=1
    echo "${line}" | sed -e 's|%|%%|g' | fold -s -w 70 | \
    while read subline ; do
        if [ ${first} -eq 1 ]; then
            echo "- ${subline}"
            first=0
        else
            echo "  ${subline}"
        fi
    done
done
