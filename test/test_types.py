#
# Copyright © 2020 Red Hat, Inc.
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

import rpmfluff
from baseclass import TestCompareSRPM, TestCompareRPMs, TestCompareKoji

rpminspect_sh = """#!/bin/sh
echo nope
"""


class SameTypeCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")
        self.after_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")

        self.inspection = "types"
        self.label = "types"
        self.result = "OK"


class SameTypeCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")
        self.after_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")

        self.inspection = "types"
        self.label = "types"
        self.result = "OK"


class SameTypeCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")
        self.after_rpm.add_simple_compilation(installPath="/usr/bin/rpminspect")

        self.inspection = "types"
        self.label = "types"
        self.result = "OK"


class ChangedTypeCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.SourceFile("rpminspect.source", rpminspect_sh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.GeneratedSourceFile("rpminspect.source", rpmfluff.make_png()),
        )

        self.inspection = "types"
        self.label = "types"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ChangedTypeCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.SourceFile("rpminspect.sh", rpminspect_sh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.GeneratedSourceFile("rpminspect.png", rpmfluff.make_png()),
        )

        self.inspection = "types"
        self.label = "types"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ChangedTypeCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.SourceFile("rpminspect.sh", rpminspect_sh),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/vaporware/rpminspect",
            rpmfluff.GeneratedSourceFile("rpminspect.png", rpmfluff.make_png()),
        )

        self.inspection = "types"
        self.label = "types"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
