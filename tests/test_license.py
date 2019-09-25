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

from baseclass import TestSRPM, TestRPMs, TestKoji

# Empty License tag fails on SRPM (BAD)
class TestEmptyLicenseTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# XXX: Empty License tag fails on built RPMs (BAD)
#class TestEmptyLicenseTagRPMs(TestRPMs):

# XXX: Empty License tag fails on Koji build (BAD)
#class TestEmptyLicenseTagKojiBuild(TestKoji):

# Forbidden License tag fails on SRPM (BAD)
class TestForbiddenLicenseTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("APSL-1.2")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# XXX: Forbidden License tag fails on built RPMs (BAD)
#class TestForbiddenLicenseTagRPMs(TestRPMs):

# XXX: Forbidden License tag fails on Koji build (BAD)
#class TestForbiddenLicenseTagKojiBuild(TestKoji):

# License tag with unprofessional language fails on SRPM (BAD)
class TestBadWordLicenseTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("GPLv2+ and reallybadword and MIT")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# XXX: License tag with unprofessional language fails on built RPMs (BAD)
#class TestBadWordLicenseTagRPMs(TestRPMs):

# XXX: License tag with unprofessional language fails on Koji build (BAD)
#class TestBadWordLicenseTagKojiBuild(TestKoji):

# Valid License tag passes on SRPM (OK)
class TestValidLicenseTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("GPLv3+")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# XXX: Valid License tag passes on built RPMs (OK)
#class TestValidLicenseTagRPMs(TestRPMs):

# XXX: Valid License tag passes on Koji build (OK)
#class TestValidLicenseTagKojiBuild(TestKoji):
