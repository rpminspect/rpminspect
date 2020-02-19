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

from baseclass import TestSRPM, TestRPMs, TestKoji

# Empty License tag fails on SRPM (BAD)
class TestEmptyLicenseTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# Empty License tag fails on RPMs (BAD)
class TestEmptyLicenseTagRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# Empty License tag fails on Koji build (BAD)
class TestEmptyLicenseTagKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# Forbidden License tag fails on SRPM (BAD)
class TestForbiddenLicenseTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("APSL-1.2")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# Forbidden License tag fails on RPMs (BAD)
class TestForbiddenLicenseTagRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.addLicense("APSL-1.2")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# Forbidden License tag fails on Koji build (BAD)
class TestForbiddenLicenseTagKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addLicense("APSL-1.2")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# License tag with unprofessional language fails on SRPM (BAD)
class TestBadWordLicenseTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("GPLv2+ and reallybadword and MIT")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# License tag with unprofessional language fails on RPMs (BAD)
class TestBadWordLicenseTagRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.addLicense("GPLv2+ and reallybadword and MIT")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# License tag with unprofessional language fails on Koji build (BAD)
class TestBadWordLicenseTagKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addLicense("GPLv2+ and reallybadword and MIT")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'BAD'

# Valid License tag passes on SRPM (OK)
class TestValidLicenseTagSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("GPLv3+")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag passes on RPMs (OK)
class TestValidLicenseTagRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.addLicense("GPLv3+")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag passes on Koji build (OK)
class TestValidLicenseTagKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addLicense("GPLv3+")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces passes on SRPM (OK)
class TestValidLicenseTagWithSpacesSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("ASL 2.0")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces passes on RPMs (OK)
class TestValidLicenseTagWithSpacesRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.addLicense("ASL 2.0")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces passes on Koji build (OK)
class TestValidLicenseTagWithSpacesKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addLicense("ASL 2.0")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces passes on SRPM (OK)
class TestValidLicenseTagWithBooleanSpacesSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("GPLv3+ and ASL 2.0")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces passes on RPMs (OK)
class TestValidLicenseTagWithBooleanSpacesRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.addLicense("ASL 2.0 and GPLv3+")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces passes on Koji build (OK)
class TestValidLicenseTagWithBooleanSpacesKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addLicense("GPLv3+ or ASL 2.0")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces and parens passes on SRPM (OK)
class TestValidLicenseTagWithBooleanSpacesParensSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addLicense("Artistic 2.0 and (GPL+ or Artistic)")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces and parens passes on RPMs (OK)
class TestValidLicenseTagWithBooleanSpacesParensRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.addLicense("Artistic 2.0 and (GPL+ or Artistic)")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'

# Valid License tag with spaces and parens passes on Koji build (OK)
class TestValidLicenseTagWithBooleanSpacesParensKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addLicense("Artistic 2.0 and (GPL+ or Artistic)")
        self.inspection = 'license'
        self.label = 'license'
        self.result = 'INFO'
