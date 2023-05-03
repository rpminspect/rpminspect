#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import unittest
from baseclass import TestCompareRPMs, TestCompareKoji, TestRPMs, TestKoji
import rpmfluff

# This check requires the udevadm verify executable in the PATH
have_udevadm = True

# As udevadm verify was introduced in systemd v254,
# skip the udevrules tests if udevadm verify is not available.
if os.system("udevadm verify --help >/dev/null 2>&1") != 0:
    have_udevadm = False


valid_udev_rules = """
PROGRAM="a", RESULT=="b", GOTO="c"
LABEL="c"
"""


invalid_udev_rules = """
RESULT=="a", PROGRAM="b", GOTO="c"
LABEL="d"
"""


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class ValidUdevRulesRPM(TestRPMs):
    """
    Valid udev rules file is OK for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/lib/udev/rules.d/valid_udev.rules",
            rpmfluff.SourceFile("valid_udev.rules", valid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "OK"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class ShMalformedRPM(TestRPMs):
    """
    Invalid udev rules file is BAD for RPMs
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/lib/udev/rules.d/invalid_udev.rules",
            rpmfluff.SourceFile("invalid_udev.rules", invalid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class ValidUdevRulesKoji(TestKoji):
    """
    Valid udev rules file is OK for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/lib/udev/rules.d/valid_udev.rules",
            rpmfluff.SourceFile("valid_udev.rules", valid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "OK"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class ShMalformedKoji(TestKoji):
    """
    Invalid udev rules file is BAD for Koji build
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/lib/udev/rules.d/invalid_udev.rules",
            rpmfluff.SourceFile("invalid_udev.rules", invalid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class ValidUdevRulesCompareRPMs(TestCompareRPMs):
    """
    Valid udev rules file is OK for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/valid_udev.rules",
            rpmfluff.SourceFile("valid_udev.rules", valid_udev_rules),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/valid_udev.rules",
            rpmfluff.SourceFile("valid_udev.rules", valid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "OK"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class ShMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid udev rules file is BAD for comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/invalid_udev.rules",
            rpmfluff.SourceFile("invalid_udev.rules", invalid_udev_rules),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/invalid_udev.rules",
            rpmfluff.SourceFile("invalid_udev.rules", invalid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class ValidUdevRulesCompareKoji(TestCompareKoji):
    """
    Valid udev rules file is OK for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/valid_udev.rules",
            rpmfluff.SourceFile("valid_udev.rules", valid_udev_rules),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/valid_udev.rules",
            rpmfluff.SourceFile("valid_udev.rules", valid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "OK"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class ShMalformedCompareKoji(TestCompareKoji):
    """
    Invalid udev rules file is BAD for comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/invalid_udev.rules",
            rpmfluff.SourceFile("invalid_udev.rules", invalid_udev_rules),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/invalid_udev.rules",
            rpmfluff.SourceFile("invalid_udev.rules", invalid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class InvalidUdevRulesCompareRPMs(TestCompareRPMs):
    """
    Valid udev rules file in before, invalid in after is BAD when comparing RPMs
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/valid_udev.rules",
            rpmfluff.SourceFile("valid_udev.rules", valid_udev_rules),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/invalid_udev.rules",
            rpmfluff.SourceFile("invalid_udev.rules", invalid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


@unittest.skipUnless(have_udevadm, "udevadm verify is not available")
class InvalidUdevRulesCompareKoji(TestCompareKoji):
    """
    Valid udev rules file in before, invalid in after is BAD when comparing Koji builds
    """

    def setUp(self):
        super().setUp()
        self.before_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/valid_udev.rules",
            rpmfluff.SourceFile("valid_udev.rules", valid_udev_rules),
        )
        self.after_rpm.add_installed_file(
            "/usr/lib/udev/rules.d/invalid_udev.rules",
            rpmfluff.SourceFile("invalid_udev.rules", invalid_udev_rules),
        )
        self.inspection = "udevrules"
        self.result = "BAD"
        self.waiver_auth = "Anyone"
