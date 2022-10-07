# Copyright © 2022 Red Hat, Inc.
# Author(s): Zuzana Miklankova <zmiklank@redhat.com>
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

import os
import unittest
from baseclass import TestCompareRPMs, TestCompareKoji, TestRPMs, TestKoji

needed_flags = "-Wl,-z,now -fcf-protection=full -fplugin=annobin -O2 \
        -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS -fstack-protector-strong \
        -fstack-clash-protection -flto"

# This check requires the annocheck executable in the PATH
have_annocheck = True

if os.system("annocheck --help >/dev/null 2>&1") != 0:
    have_annocheck = False


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckHardenedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(compileFlags=needed_flags)
        self.after_rpm.add_simple_library(compileFlags=needed_flags)

        self.inspection = "annocheck"
        self.result = "INFO"
        self.exitcode = 0
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckHardenedCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(compileFlags=needed_flags)
        self.after_rpm.add_simple_library(compileFlags=needed_flags)

        self.inspection = "annocheck"
        self.result = "INFO"
        self.exitcode = 0
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckNotHardenedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(compileFlags=needed_flags)
        self.after_rpm.add_simple_library()

        self.inspection = "annocheck"
        self.result = "VERIFY"
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckNotHardenedCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(compileFlags=needed_flags)
        self.after_rpm.add_simple_library()

        self.inspection = "annocheck"
        self.result = "VERIFY"
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckHardenedRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(compileFlags=needed_flags)

        self.inspection = "annocheck"
        self.result = "INFO"
        self.exitcode = 0
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckHardenedKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(compileFlags=needed_flags)

        self.inspection = "annocheck"
        self.result = "INFO"
        self.exitcode = 0
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckNotHardenedRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library()

        self.inspection = "annocheck"
        self.result = "VERIFY"
        self.waiver_auth = "Not Waivable"


@unittest.skipUnless(have_annocheck, "annocheck not available")
class AnnocheckNotHardenedKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library()

        self.inspection = "annocheck"
        self.result = "VERIFY"
        self.waiver_auth = "Not Waivable"
