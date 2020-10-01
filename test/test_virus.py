#
# Copyright (C) 2020  Red Hat, Inc.
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
from baseclass import TestCompareSRPM, TestCompareRPMs, TestCompareKoji


# package that does not contain a virus (OK)
class HasNoVirusSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class HasNoVirusRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class HasNoVirusKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class HasNoVirusCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )
        self.after_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class HasNoVirusCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )
        self.after_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class HasNoVirusCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )
        self.after_rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# package that contains a virus (BAD)
class HasVirusSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.label = "virus"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HasVirusRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.label = "virus"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HasVirusKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.label = "virus"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HasVirusCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_fake_virus(
            "usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755"
        )
        self.after_rpm.add_fake_virus(
            "usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HasVirusCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_fake_virus(
            "usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755"
        )
        self.after_rpm.add_fake_virus(
            "usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HasVirusCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_fake_virus(
            "usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755"
        )
        self.after_rpm.add_fake_virus(
            "usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755"
        )

        self.inspection = "virus"
        self.label = "virus"
        self.result = "BAD"
        self.waiver_auth = "Anyone"
