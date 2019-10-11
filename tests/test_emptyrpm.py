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

import subprocess
import unittest
from baseclass import *

# Regular SRPM has payload (OK)
class TestHasPayloadSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.add_simple_payload_file()
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'OK'

# Regular RPMs have payload (OK)
class TestHasPayloadRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_simple_payload_file()
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'OK'

# Regular Koji build has payload (OK)
class TestHasPayloadKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_simple_payload_file()
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'OK'

# Regular package has empty payload (VERIFY)
class TestPkgHasEmptyPayload(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'VERIFY'

# Packages in Koji build have empty payloads (VERIFY)
class TestKojiBuildHaveEmptyPayloads(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'VERIFY'

# New package has empty payload across Koji builds (VERIFY)
class TestNewPkgHasEmptyPayload(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_payload_file()
        self.after_rpm.add_subpackage(self.after_rpm.name + '-newthing')
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'VERIFY'

# Packages continue to be empty (INFO)
class TestPkgStillHasEmptyPayload(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'INFO'

# Package lost payload across Koji builds (VERIFY)
class TestPkgLostPayload(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_payload_file()
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'VERIFY'

# Existing package is now missing across Koji builds (VERIFY)
class TestExistingPkgMissing(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_payload_file()
        self.before_rpm.add_subpackage(self.before_rpm.name + '-newthing')
        self.inspection = 'emptyrpm'
        self.label = 'empty-payload'
        self.result = 'VERIFY'
