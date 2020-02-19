#
# Copyright (C) 2019-2020  Red Hat, Inc.
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

import unittest
from baseclass import *

# Verify rpminspect runs against an SRPM
class DefaultSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.result = 'OK'

# Verify rpminspect runs against an RPM
class DefaultRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.result = 'OK'

# Verify rpminspect runs against a Koji build
class DefaultKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.result = 'OK'

# Verify rpminspect runs against two SRPMs
class DefaultCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.result = 'OK'

# Verify rpminspect runs against two RPMs
class DefaultCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.result = 'OK'

# Verify rpminspect runs against two Koji builds
class DefaultCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.result = 'OK'
