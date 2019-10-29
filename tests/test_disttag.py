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

# Verify missing %{?dist} in Release fails on SRPM (BAD)
class TestMissingDistTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.inspection = 'disttag'
        self.label = 'dist-tag'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# Verify missing %{?dist} in Release fails on Koji build (BAD)
class TestMissingDistTagKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.inspection = 'disttag'
        self.label = 'dist-tag'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# Verify running on not an SRPM fails
class TestDistTagOnNonSRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.release = '1%{?dist}'
        self.inspection = 'disttag'
        self.label = 'dist-tag'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# Verify malformed %{?dist} tag in Release fails on SRPM (BAD)
class TestMalformedDistTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.release = '1dist'
        self.inspection = 'disttag'
        self.label = 'dist-tag'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# Verify malformed %{?dist} tag in Release fails on Koji build (BAD)
class TestMalformedDistTagKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.release = '1dist'
        self.inspection = 'disttag'
        self.label = 'dist-tag'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# Verify correct %{?dist} usage passes on SRPM (OK)
class TestDistTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.release = '1%{?dist}'
        self.inspection = 'disttag'
        self.label = 'dist-tag'

# Verify correct %{?dist} usage passes on Koji build (OK)
class TestDistTagKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.release = '1%{?dist}'
        self.inspection = 'disttag'
        self.label = 'dist-tag'
