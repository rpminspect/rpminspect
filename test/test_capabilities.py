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


class SecuritySKIPUnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_sys_nice=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecurityFAILUnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_sys_nice=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_sys_nice=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPUnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_sys_nice=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecuritySKIPUnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_sys_nice=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecuritySKIPUnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_sys_nice=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecurityINFORMUnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_sys_nice=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_sys_nice=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_sys_nice=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_sys_nice=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_sys_nice=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_sys_nice=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_sys_nice=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnapprovedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_sys_nice=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_sys_nice=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnapprovedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_sys_nice=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPCapabilitiesMismatchRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_chown=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecuritySKIPCapabilitiesMismatchKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_chown=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecuritySKIPCapabilitiesMismatchCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_chown=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecuritySKIPCapabilitiesMismatchCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip\n", "%caps(cap_chown=ep) /usr/sbin/skip\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecurityINFORMCapabilitiesMismatchRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_chown=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMCapabilitiesMismatchKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_chown=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMCapabilitiesMismatchCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_chown=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMCapabilitiesMismatchCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform\n", "%caps(cap_chown=ep) /usr/sbin/inform\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYCapabilitiesMismatchRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_chown=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYCapabilitiesMismatchKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_chown=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYCapabilitiesMismatchCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_chown=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYCapabilitiesMismatchCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify\n", "%caps(cap_chown=ep) /usr/sbin/verify\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILCapabilitiesMismatchRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_chown=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILCapabilitiesMismatchKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_chown=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILCapabilitiesMismatchCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_chown=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILCapabilitiesMismatchCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail\n", "%caps(cap_chown=ep) /usr/sbin/fail\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPUnexpectedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip1\n", "%caps(cap_chown=ep) /usr/sbin/skip1\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecuritySKIPUnexpectedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/skip1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip1\n", "%caps(cap_chown=ep) /usr/sbin/skip1\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecuritySKIPUnexpectedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip1\n", "%caps(cap_chown=ep) /usr/sbin/skip1\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecuritySKIPUnexpectedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/skip1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/skip1\n", "%caps(cap_chown=ep) /usr/sbin/skip1\n"
        )

        self.inspection = "capabilities"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecurityINFORMUnexpectedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform1\n", "%caps(cap_chown=ep) /usr/sbin/inform1\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnexpectedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/inform1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform1\n", "%caps(cap_chown=ep) /usr/sbin/inform1\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnexpectedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform1\n", "%caps(cap_chown=ep) /usr/sbin/inform1\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityINFORMUnexpectedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/inform1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/inform1\n", "%caps(cap_chown=ep) /usr/sbin/inform1\n"
        )

        self.inspection = "capabilities"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYUnexpectedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify1\n", "%caps(cap_chown=ep) /usr/sbin/verify1\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnexpectedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/verify1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify1\n", "%caps(cap_chown=ep) /usr/sbin/verify1\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnexpectedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify1\n", "%caps(cap_chown=ep) /usr/sbin/verify1\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityVERIFYUnexpectedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/verify1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/verify1\n", "%caps(cap_chown=ep) /usr/sbin/verify1\n"
        )

        self.inspection = "capabilities"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILUnexpectedCapabilitiesRPMs(TestRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail1\n", "%caps(cap_chown=ep) /usr/sbin/fail1\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnexpectedCapabilitiesKoji(TestKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_compilation(installPath="usr/sbin/fail1")
        sub = self.rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail1\n", "%caps(cap_chown=ep) /usr/sbin/fail1\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnexpectedCapabilitiesCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail1\n", "%caps(cap_chown=ep) /usr/sbin/fail1\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecurityFAILUnexpectedCapabilitiesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(
        have_caps_support, "rpm lacks %caps macro support (need rpm >= 4.7.0)"
    )
    def setUp(self):
        super().setUp()

        self.after_rpm.add_simple_compilation(installPath="usr/sbin/fail1")
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files = sub.section_files.replace(
            "/usr/sbin/fail1\n", "%caps(cap_chown=ep) /usr/sbin/fail1\n"
        )

        self.inspection = "capabilities"
        self.result = "BAD"
        self.waiver_auth = "Security"
