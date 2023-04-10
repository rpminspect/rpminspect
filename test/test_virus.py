#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import unittest

from baseclass import TestSRPM, TestRPMs, TestKoji
from baseclass import TestCompareSRPM, TestCompareRPMs, TestCompareKoji

# These tests more often than not time out in our FreeBSD CI job, so just
# disable them on that platform.
on_freebsd = False

if os.uname().sysname == "FreeBSD":
    on_freebsd = True


# package that does not contain a virus (OK)
class HasNoVirusSRPM(TestSRPM):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()


class HasVirusRPMs(TestRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()


class HasVirusKoji(TestKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()

        self.rpm.add_fake_virus("usr/lib/whte_rbt.obj", "whte_rbt.pas", mode="0755")

        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"

    def runTest(self):
        super().runTest()


class HasVirusCompareSRPM(TestCompareSRPM):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
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


class HasKnownVirusSkipRPMs(TestRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.rpm.add_fake_virus("usr/lib/wrskip.o", "wrskip.pas", mode="0755")
        self.inspection = "virus"
        self.result = "OK"


class HasKnownVirusSkipCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.before_rpm.add_fake_virus("usr/lib/wrskip.o", "wrskip.pas", mode="0755")
        self.after_rpm.add_fake_virus("usr/lib/wrskip.o", "wrskip.pas", mode="0755")
        self.inspection = "virus"
        self.result = "OK"


class HasKnownVirusSkipKoji(TestKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.rpm.add_fake_virus("usr/lib/wrskip.o", "wrskip.pas", mode="0755")
        self.inspection = "virus"
        self.result = "OK"


class HasKnownVirusSkipCompareKoji(TestCompareKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.before_rpm.add_fake_virus("usr/lib/wrskip.o", "wrskip.pas", mode="0755")
        self.after_rpm.add_fake_virus("usr/lib/wrskip.o", "wrskip.pas", mode="0755")
        self.inspection = "virus"
        self.result = "OK"


class HasKnownVirusInformRPMs(TestRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.rpm.add_fake_virus("usr/lib/wrinform.o", "wrinform.pas", mode="0755")
        self.inspection = "virus"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class HasKnownVirusInformCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.before_rpm.add_fake_virus(
            "usr/lib/wrinform.o", "wrinform.pas", mode="0755"
        )
        self.after_rpm.add_fake_virus("usr/lib/wrinform.o", "wrinform.pas", mode="0755")
        self.inspection = "virus"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class HasKnownVirusInformKoji(TestKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.rpm.add_fake_virus("usr/lib/wrinform.o", "wrinform.pas", mode="0755")
        self.inspection = "virus"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class HasKnownVirusInformCompareKoji(TestCompareKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.before_rpm.add_fake_virus(
            "usr/lib/wrinform.o", "wrinform.pas", mode="0755"
        )
        self.after_rpm.add_fake_virus("usr/lib/wrinform.o", "wrinform.pas", mode="0755")
        self.inspection = "virus"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class HasKnownVirusVerifyRPMs(TestRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.rpm.add_fake_virus("usr/lib/wrverify.o", "wrverify.pas", mode="0755")
        self.inspection = "virus"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class HasKnownVirusVerifyCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.before_rpm.add_fake_virus(
            "usr/lib/wrverify.o", "wrverify.pas", mode="0755"
        )
        self.after_rpm.add_fake_virus("usr/lib/wrverify.o", "wrverify.pas", mode="0755")
        self.inspection = "virus"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class HasKnownVirusVerifyKoji(TestKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.rpm.add_fake_virus("usr/lib/wrverify.o", "wrverify.pas", mode="0755")
        self.inspection = "virus"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class HasKnownVirusVerifyCompareKoji(TestCompareKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.before_rpm.add_fake_virus(
            "usr/lib/wrverify.o", "wrverify.pas", mode="0755"
        )
        self.after_rpm.add_fake_virus("usr/lib/wrverify.o", "wrverify.pas", mode="0755")
        self.inspection = "virus"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class HasKnownVirusFailRPMs(TestRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.rpm.add_fake_virus("usr/lib/wrfail.o", "wrfail.pas", mode="0755")
        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"


class HasKnownVirusFailCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.before_rpm.add_fake_virus("usr/lib/wrfail.o", "wrfail.pas", mode="0755")
        self.after_rpm.add_fake_virus("usr/lib/wrfail.o", "wrfail.pas", mode="0755")
        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"


class HasKnownVirusFailKoji(TestKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.rpm.add_fake_virus("usr/lib/wrfail.o", "wrfail.pas", mode="0755")
        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"


class HasKnownVirusFailCompareKoji(TestCompareKoji):
    @unittest.skipIf(on_freebsd, "usually times out on FreeBSD")
    def setUp(self):
        super().setUp()
        self.before_rpm.add_fake_virus("usr/lib/wrfail.o", "wrfail.pas", mode="0755")
        self.after_rpm.add_fake_virus("usr/lib/wrfail.o", "wrfail.pas", mode="0755")
        self.inspection = "virus"
        self.result = "BAD"
        self.waiver_auth = "Security"
