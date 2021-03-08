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

from baseclass import TestCompareRPMs


class FileSizeGrowsAtThreshold(TestCompareRPMs):
    """Assert when a file grows by exactly the configured threshold, VERIFY result occurs."""

    def setUp(self):
        super().setUp()

        # Tests have threshold configuration of 20%
        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 5)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 6)
        )

        self.inspection = "filesize"
        self.label = "filesize"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class FileSizeGrowsAboveThreshold(TestCompareRPMs):
    """Assert when a file grows by more than the configured threshold, VERIFY result occurs."""

    def setUp(self):
        super().setUp()

        # Tests have threshold configuration of 20%
        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 5)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 10)
        )

        self.inspection = "filesize"
        self.label = "filesize"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class FileSizeGrowsBelowThreshold(TestCompareRPMs):
    """Assert when a file grows by less than the configured threshold, an INFO result occurs."""

    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 10)
        )
        self.after_rpm.add_installed_file(
            "/some/file", rpmfluff.SourceFile("file", "a" * 11)
        )

        self.inspection = "filesize"
        self.label = "filesize"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
