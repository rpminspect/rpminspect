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

from baseclass import *

# Verify valid Vendor passes on an SRPM (OK)
class ValidVendorSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify valid Vendor passes on binary RPMs (OK)
class ValidVendorRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify valid Vendor passes on Koji build (OK)
class ValidVendorKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify invalid Vendor fails on an SRPM (BAD)
class InvalidVendorSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.addVendor("Amalgamated Amalgamations LLC")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify invalid Vendor fails on binary RPMs (BAD)
class InvalidVendorRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.addVendor("Amalgamated Amalgamations LLC")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify invalid Vendor fails on Koji build (BAD)
class InvalidVendorKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.addVendor("Amalgamated Amalgamations LLC")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify gaining Vendor reports verify on SRPM (VERIFY)
class GainingVendorCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.after_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify gaining Vendor reports verify on built RPMS (VERIFY)
class GainingVendorCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.after_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify gaining Vendor reports verify on Koji build (VERIFY)
class GainingVendorCompareKojiBuild(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.after_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify losing Vendor reports verify on SRPM (VERIFY)
class LosingVendorCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.waiver_auth = "Anyone"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify losing Vendor reports verify on built RPMs (VERIFY)
class LosingVendorCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.waiver_auth = "Anyone"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify losing Vendor reports verify on Koji build (VERIFY)
class LosingVendorCompareKojiBuild(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.waiver_auth = "Anyone"
        self.result = "OK"


# Verify changing Vendor reports verify on SRPM (VERIFY)
class ChangingVendorCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.addVendor("Amalgamated Amalgamations LLC")
        self.after_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.waiver_auth = "Anyone"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify changing Vendor reports verify on built RPMs (VERIFY)
class ChangingVendorCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.addVendor("Amalgamated Amalgamations LLC")
        self.after_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify changing Vendor reports verify on Koji build (VERIFY)
class LosingVendorCompareKojiBuild(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.addVendor("Amalgamated Amalgamations LLC")
        self.after_rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify invalid Buildhost subdomain fails on an SRPM (BAD)
class InvalidBuildhostSubdomainSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.buildhost_subdomain = ".totallylegitbuilder.com"
        self.rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify invalid Buildhost subdomain fails on binary RPMs (BAD)
class InvalidBuildhostSubdomainRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.buildhost_subdomain = ".totallylegitbuilder.com"
        self.rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify invalid Buildhost subdomain fails on Koji build (BAD)
class InvalidBuildhostSubdomainKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.buildhost_subdomain = ".totallylegitbuilder.com"
        self.rpm.addVendor("Vendorco Ltd.")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify Summary without bad words passes on an SRPM (OK)
class CleanSummarySRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.add_summary("Lorem ipsum dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify Summary without bad words passes on binary RPMs (OK)
class CleanSummaryRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_summary("Lorem ipsum dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify Summary without bad words passes on Koji build (OK)
class CleanSummaryKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_summary("Lorem ipsum dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify Summary with bad words fails on an SRPM (BAD)
class DirtySummarySRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.add_summary("Lorem ipsum reallybadword dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify Summary with bad words fails on binary RPMs (BAD)
class DirtySummaryRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_summary("Lorem ipsum reallybadword dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify Summary with bad words fails on Koji build (BAD)
class DirtySummaryKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_summary("Lorem ipsum reallybadword dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify changing Summary reports verify on SRPM (VERIFY)
class ChangingSummaryCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.add_summary("Lorem ipsum dolor sit amet")
        self.after_rpm.add_summary("Lorem ipsum")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify changing Summary reports verify on built RPMs (VERIFY)
class ChangingSummaryCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_summary("Lorem ipsum dolor sit amet")
        self.after_rpm.add_summary("Lorem ipsum")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify changing Summary reports verify on Koji build (VERIFY)
class ChangingSummaryCompareKojiBuild(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_summary("Lorem ipsum dolor sit amet")
        self.after_rpm.add_summary("Lorem ipsum")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify Description without bad words passes on an SRPM (OK)
class CleanDescriptionSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify Description without bad words passes on binary RPMs (OK)
class CleanDescriptionRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify Description without bad words passes on Koji build (OK)
class CleanDescriptionKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "OK"


# Verify Description with bad words fails on an SRPM (BAD)
class DirtyDescriptionSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod reallybadword tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify Description with bad words fails on binary RPMs (BAD)
class DirtyDescriptionRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod reallybadword tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify Description with bad words fails on Koji build (BAD)
class DirtyDescriptionKojiBuild(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod reallybadword tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "BAD"


# Verify changing Description reports verify on SRPM (VERIFY)
class ChangingDescriptionCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod reallybadword tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.after_rpm.add_description("Lorem ipsum dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify changing Description reports verify on built RPMs (VERIFY)
class ChangingDescriptionCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod reallybadword tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.after_rpm.add_description("Lorem ipsum dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Verify changing Description reports verify on Koji build (VERIFY)
class ChangingDescriptionCompareKojiBuild(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_description(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod reallybadword tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
        )
        self.after_rpm.add_description("Lorem ipsum dolor sit amet")
        self.inspection = "metadata"
        self.label = "header-metadata"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
