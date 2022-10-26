#
# Copyright 2020 Red Hat, Inc.
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

from baseclass import TestSRPM, TestRPMs, TestKoji
from baseclass import TestCompareSRPM, TestCompareRPMs, TestCompareKoji

# NOTE: keep the newline here so the computed SHA-256 digest matches test/data/politics/GENERIC
sealand_motto = """E Mare, Libertas
"""


# package contains allowed politically sensitive file (INFO)
class AllowedPoliticallySensitiveFileSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/sealand-motto.txt",
            rpmfluff.SourceFile("sealand-motto.txt", sealand_motto),
        )

        self.inspection = "politics"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AllowedPoliticallySensitiveFileRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/sealand-motto.txt",
            rpmfluff.SourceFile("sealand-motto.txt", sealand_motto),
        )

        self.inspection = "politics"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AllowedPoliticallySensitiveFileKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/sealand-motto.txt",
            rpmfluff.SourceFile("sealand-motto.txt", sealand_motto),
        )

        self.inspection = "politics"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AllowedPoliticallySensitiveFileCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/share/sealand-motto.txt",
            rpmfluff.SourceFile("sealand-motto.txt", sealand_motto),
        )

        self.inspection = "politics"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AllowedPoliticallySensitiveFileCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/share/sealand-motto.txt",
            rpmfluff.SourceFile("sealand-motto.txt", sealand_motto),
        )

        self.inspection = "politics"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class AllowedPoliticallySensitiveFileCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/share/sealand-motto.txt",
            rpmfluff.SourceFile("sealand-motto.txt", sealand_motto),
        )

        self.inspection = "politics"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# package contains denied politically sensitive file (BAD)
class ForbiddenPoliticallySensitiveFileSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/most-of-sealand-is.txt",
            rpmfluff.SourceFile("most-of-sealand-is.txt", "water"),
        )

        self.inspection = "politics"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class ForbiddenPoliticallySensitiveFileRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/most-of-sealand-is.txt",
            rpmfluff.SourceFile("most-of-sealand-is.txt", "water"),
        )

        self.inspection = "politics"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class ForbiddenPoliticallySensitiveFileKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/share/most-of-sealand-is.txt",
            rpmfluff.SourceFile("most-of-sealand-is.txt", "water"),
        )

        self.inspection = "politics"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class ForbiddenPoliticallySensitiveFileCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/share/most-of-sealand-is.txt",
            rpmfluff.SourceFile("most-of-sealand-is.txt", "water"),
        )

        self.inspection = "politics"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class ForbiddenPoliticallySensitiveFileCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/share/most-of-sealand-is.txt",
            rpmfluff.SourceFile("most-of-sealand-is.txt", "water"),
        )

        self.inspection = "politics"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class ForbiddenPoliticallySensitiveFileCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/share/most-of-sealand-is.txt",
            rpmfluff.SourceFile("most-of-sealand-is.txt", "water"),
        )

        self.inspection = "politics"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"
