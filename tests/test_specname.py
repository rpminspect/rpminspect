#
# Copyright (C) 2019  Red Hat, Inc.
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
from baseclass import TestSRPM, TestRPMs, TestKoji

# Verify spec filename matches package name on SRPM (OK)
class TestSpecNameSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.inspection = 'specname'
        self.label = 'spec-file-name'
        self.result = 'OK'

# Verify spec filename matches package name on Koji build (OK)
class TestSpecNameKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.inspection = 'specname'
        self.label = 'spec-file-name'
        self.result = 'OK'

# Verify spec filename test on binary RPMs fails (BAD)
class TestSpecNameRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.inspection = 'specname'
        self.label = 'spec-file-name'
        self.result = 'BAD'

# Verify spec filename not matching package name fails (BAD)
class TestBadSpecNameSRPM(TestSRPM):
    @unittest.skip("requires addSpecBasename() support in rpmfluff")
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addSpecBasename("badspecname")
        self.inspection = 'specname'
        self.label = 'spec-file-name'
        self.result = 'BAD'

# Verify spec filename not matching package name fails on Koji build (BAD)
class TestBadSpecNameKojiBuild(TestKoji):
    @unittest.skip("requires addSpecBasename() support in rpmfluff")
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addSpecBasename("badspecname")
        self.inspection = 'specname'
        self.label = 'spec-file-name'
        self.result = 'BAD'
