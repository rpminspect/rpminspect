#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
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
        self.result = "OK"

    def runTest(self):
        super().runTest()


class HasNoVirusRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )

        self.inspection = "virus"
        self.result = "OK"

    def runTest(self):
        super().runTest()


class HasNoVirusKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_simple_library(
            libraryName="whte_rbt.obj", installPath="usr/lib/whte_rbt.obj"
        )

        self.inspection = "virus"
        self.result = "OK"

    def runTest(self):
        super().runTest()


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
        self.result = "OK"

    def runTest(self):
        super().runTest()


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
        self.result = "OK"

    def runTest(self):
        super().runTest()


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
        self.result = "OK"

    def runTest(self):
        super().runTest()


# package that contains a virus (BAD)
class HasVirusSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()


class HasVirusRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()


class HasVirusKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()


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
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()


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
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()


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
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()
