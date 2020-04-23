#
# Copyright (C) 2020  Red Hat, Inc.
# Author(s):  David Cantrell <dcantrell@redhat.com>
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

from baseclass import TestRPMs, TestKoji
from baseclass import TestCompareRPMs, TestCompareKoji

#############
# bin owner #
#############

# File in /bin owned by 'root' is OK in RPMs

# File in /sbin owned by 'root' is OK in RPMs

# File in /usr/bin owned by 'root' is OK in RPMs

# File in /usr/sbin owned by 'root' is OK in RPMs

# File in /bin owned by 'bin' is BAD in RPMs

# File in /sbin owned by 'bin' is BAD in RPMs

# File in /usr/bin owned by 'bin' is BAD in RPMs

# File in /usr/sbin owned by 'bin' is BAD in RPMs

# File in /bin owned by 'root' is OK in Koji build

# File in /sbin owned by 'root' is OK in Koji build

# File in /usr/bin owned by 'root' is OK in Koji build

# File in /usr/sbin owned by 'root' is OK in Koji build

# File in /bin owned by 'bin' is BAD in Koji build

# File in /sbin owned by 'bin' is BAD in Koji build

# File in /usr/bin owned by 'bin' is BAD in Koji build

# File in /usr/sbin owned by 'bin' is BAD in Koji build

# File in /bin owned by 'root' is OK when comparing RPMs

# File in /sbin owned by 'root' is OK when comparing RPMs

# File in /usr/bin owned by 'root' is OK when comparing RPMs

# File in /usr/sbin owned by 'root' is OK when comparing RPMs

# File in /bin owned by 'bin' is BAD when comparing RPMs

# File in /sbin owned by 'bin' is BAD when comparing RPMs

# File in /usr/bin owned by 'bin' is BAD when comparing RPMs

# File in /usr/sbin owned by 'bin' is BAD when comparing RPMs

# File in /bin owned by 'root' is OK when comparing Koji builds

# File in /sbin owned by 'root' is OK when comparing Koji builds

# File in /usr/bin owned by 'root' is OK when comparing Koji builds

# File in /usr/sbin owned by 'root' is OK when comparing Koji builds

# File in /bin owned by 'bin' is BAD when comparing Koji builds

# File in /sbin owned by 'bin' is BAD when comparing Koji builds

# File in /usr/bin owned by 'bin' is BAD when comparing Koji builds

# File in /usr/sbin owned by 'bin' is BAD when comparing Koji builds

#############
# bin group #
#############

# File in /bin owned by group 'root' is OK in RPMs

# File in /sbin owned by group 'root' is OK in RPMs

# File in /usr/bin owned by group 'root' is OK in RPMs

# File in /usr/sbin owned by group 'root' is OK in RPMs

# File in /bin owned by group 'bin' is BAD in RPMs

# File in /sbin owned by group 'bin' is BAD in RPMs

# File in /usr/bin owned by group 'bin' is BAD in RPMs

# File in /usr/sbin owned by group 'bin' is BAD in RPMs

# File in /bin owned by group 'root' is OK in Koji build

# File in /sbin owned by group 'root' is OK in Koji build

# File in /usr/bin owned by group 'root' is OK in Koji build

# File in /usr/sbin owned by group 'root' is OK in Koji build

# File in /bin owned by group 'bin' is BAD in Koji build

# File in /sbin owned by group 'bin' is BAD in Koji build

# File in /usr/bin owned by group 'bin' is BAD in Koji build

# File in /usr/sbin owned by group 'bin' is BAD in Koji build

# File in /bin owned by group 'root' is OK when comparing RPMs

# File in /sbin owned by group 'root' is OK when comparing RPMs

# File in /usr/bin owned by group 'root' is OK when comparing RPMs

# File in /usr/sbin owned by group 'root' is OK when comparing RPMs

# File in /bin owned by group 'bin' is BAD when comparing RPMs

# File in /sbin owned by group 'bin' is BAD when comparing RPMs

# File in /usr/bin owned by group 'bin' is BAD when comparing RPMs

# File in /usr/sbin owned by group 'bin' is BAD when comparing RPMs

# File in /bin owned by group 'root' is OK when comparing Koji builds

# File in /sbin owned by group 'root' is OK when comparing Koji builds

# File in /usr/bin owned by group 'root' is OK when comparing Koji
# builds

# File in /usr/sbin owned by group 'root' is OK when comparing Koji
# builds

# File in /bin owned by group 'bin' is BAD when comparing Koji builds

# File in /sbin owned by group 'bin' is BAD when comparing Koji builds

# File in /usr/bin owned by group 'bin' is BAD when comparing Koji
# builds

# File in /usr/sbin owned by group 'bin' is BAD when comparing Koji
# builds

###################
# Forbidden owner #
###################

# File owned by 'mockbuild' is BAD in RPMs

# File owned by 'mockbuild' is BAD in Koji build

# File owned by 'mockbuild' is BAD when comparing RPMs

# File owned by 'mockbuild' is BAD when comparing Koji builds

###################
# Forbidden group #
###################

# File owned by group 'mockbuild' is BAD in RPMs

# File owned by group 'mockbuild' is BAD in Koji build

# File owned by group 'mockbuild' is BAD when comparing RPMs

# File owned by group 'mockbuild' is BAD when comparing Koji builds

###################################
# CAP_SETUID and world executable #
###################################

# File with CAP_SETUID and o+x is BAD in RPMs

# File with CAP_SETUID and o+x is BAD in Koji build

# File with CAP_SETUID and o+x is BAD when comparing RPMs

# File with CAP_SETUID and o+x is BAD when comparing Koji builds

#################################
# CAP_SETUID and group writable #
#################################

# File with CAP_SETUID and g+w is BAD in RPMs

# File with CAP_SETUID and g+w is BAD in Koji build

# File with CAP_SETUID and g+w is BAD when comparing RPMs

# File with CAP_SETUID and g+w is BAD when comparing Koji builds

#################
# Owner changed #
#################

# File owner changed is VERIFY when comparing RPMs

# File owner changed is VERIFY when comparing Koji builds

#################
# Group changed #
#################

# File group changed is VERIFY when comparing RPMs

# File group changed is VERIFY when comparing Koji builds
