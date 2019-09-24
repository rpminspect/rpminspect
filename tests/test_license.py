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

from baseclass import TestSRPM

# Empty License tag should fail (BAD)
class TestEmptyLicenseTag(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# Forbidden License tag should fail (BAD)
class TestForbiddenLicenseTag(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("APSL-1.2")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# License tag with unprofessional language should fail (BAD)
class TestBadWordLicenseTag(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("GPLv2+ and reallybadword and MIT")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# Valid License tag should pass (OK)
class TestValidLicenseTag(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("GPLv3+")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'
