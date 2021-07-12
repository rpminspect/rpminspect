#
# Copyright © 2021 Red Hat, Inc.
# Author(s): David Cantrell <dcantrell@redhat.com>
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
from distutils.version import LooseVersion
from baseclass import TestRPMs, TestKoji
from baseclass import TestCompareRPMs, TestCompareKoji

# Some tests require rpm >= 4.7.0
proc = subprocess.Popen(
    ["rpmbuild", "--version"], stdout=subprocess.PIPE, stderr=subprocess.PIPE
)
(out, err) = proc.communicate()
if LooseVersion(out.split()[2].decode("utf-8")) >= LooseVersion("4.7.0"):
    have_caps_support = True
else:
    have_caps_support = False


# package contains a file with capabilities(7) but it is not on the
# list (BAD)
class UnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/unapproved")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/unapproved\n", "%caps(cap_sys_nice=ep) /usr/sbin/unapproved\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/unapproved")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/unapproved\n", "%caps(cap_sys_nice=ep) /usr/sbin/unapproved\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/unapproved")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/unapproved\n", "%caps(cap_sys_nice=ep) /usr/sbin/unapproved\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/unapproved")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/unapproved\n", "%caps(cap_sys_nice=ep) /usr/sbin/unapproved\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


# package contains a file with approved capabilities(7) (OK)
class ApprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/approved")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/approved\n", "%caps(cap_sys_nice=ep) /usr/sbin/approved\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ApprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/approved")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/approved\n", "%caps(cap_sys_nice=ep) /usr/sbin/approved\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class ApprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/approved")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/approved\n", "%caps(cap_sys_nice=ep) /usr/sbin/approved\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class ApprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/approved")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/approved\n", "%caps(cap_sys_nice=ep) /usr/sbin/approved\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"
