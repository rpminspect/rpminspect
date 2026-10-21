#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import unittest

from rpmfluff.sourcefile import SourceFile
from rpmfluff.utils import CC

from baseclass import TestCompareRPMs, TestCompareSRPM, TestCompareKoji
from baseclass import BEFORE_NAME, BEFORE_VER, BEFORE_REL
from baseclass import AFTER_NAME, AFTER_VER, AFTER_REL

# Read in the built rpminspect executable for use in these test RPMs
with open(os.environ["RPMINSPECT"], mode="rb") as f:
    ri_bytes = f.read()


# Test rebase reporting level set to VERIFY for external package dependencies
class RebaseReportingLevelVerifyExternalSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "VERIFY"


class RebaseReportingLevelVerifyExternalRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "VERIFY"


class RebaseReportingLevelVerifyExternalKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "VERIFY"


# Test rebase reporting level set to BAD for external package dependencies
class RebaseReportingLevelBadExternalSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to BAD
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "BAD"


class RebaseReportingLevelBadExternalRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to BAD
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "BAD"


class RebaseReportingLevelBadExternalKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should use rebase_reporting_level
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to BAD
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "BAD"


# Test that internal subpackage dependencies are always reported at INFO level
# even when rebase_reporting_level is set to VERIFY or BAD
class InternalSubpackageDepWithVerifyLevelSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # Create a libs subpackage
        self.after_rpm.add_subpackage("libs")

        # internal subpackage dependency - should always be INFO
        # regardless of rebase_reporting_level setting
        self.before_rpm.add_requires("%{name}-libs")
        self.after_rpm.add_requires("%{name}-libs")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY, but internal deps stay INFO
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "VERIFY"


class InternalSubpackageDepWithVerifyLevelRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # Create a libs subpackage
        self.after_rpm.add_subpackage("libs")

        # internal subpackage dependency - should always be INFO
        # regardless of rebase_reporting_level setting
        self.before_rpm.add_requires("%{name}-libs")
        self.after_rpm.add_requires("%{name}-libs")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY, but internal deps stay INFO
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "VERIFY"


class InternalSubpackageDepWithVerifyLevelKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # Create a libs subpackage
        self.after_rpm.add_subpackage("libs")

        # internal subpackage dependency - should always be INFO
        # regardless of rebase_reporting_level setting
        self.before_rpm.add_requires("%{name}-libs")
        self.after_rpm.add_requires("%{name}-libs")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"

        # Set rebase_reporting_level to VERIFY, but internal deps stay INFO
        self.rpminspect_config["rpmdeps"]["rebase_reporting_level"] = "VERIFY"


# Test rebase reporting level defaults to INFO for external deps
class RebaseReportingLevelDefaultExternalSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should default to INFO
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RebaseReportingLevelDefaultExternalRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should default to INFO
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RebaseReportingLevelDefaultExternalKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # external package dependency - should default to INFO
        self.before_rpm.add_requires("external-somepkg = 1.0-1")
        self.after_rpm.add_requires("external-somepkg = 2.0-1")

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
