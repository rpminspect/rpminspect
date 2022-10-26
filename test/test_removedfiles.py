#
# Copyright 2022 Red Hat, Inc.
# Author(s): David Cantrell <dcantrell@redhat.com>
#            Preston Watson <prwatson@redhat.com>
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
import rpmfluff

from baseclass import TestCompareRPMs, TestCompareKoji


class FileNoRemovedRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )
        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", after_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class FileNoRemovedKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()
            after_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )
        self.after_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", after_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class FileRemovedRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class FileRemovedSecurityRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/share/polkit-1/actions/mount.so",
            rpmfluff.SourceFile("mount.so", before_src),
        )

        self.inspection = "removedfiles"
        self.result = "OK"
        self.waiver_auth = "WAIVABLE_BY_SECURITY"


class FileRemovedKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"
