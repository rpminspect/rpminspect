#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import rpmfluff

from baseclass import TestCompareKoji


# Add a subpackage that only carries a %ghost directory
def add_ghost_subpackage(rpm):
    rpm.add_subpackage("ghost")
    rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/usr/share/vaporware.d\n"
    rpm.get_subpackage("ghost").section_files += "%ghost %dir /usr/share/vaporware.d\n"


# New package has empty payload across Koji builds (VERIFY)
class NewPkgHasEmptyPayload(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.after_rpm.add_subpackage(self.after_rpm.name + "-newthing")
        self.inspection = "lostpayload"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Packages continue to be empty (VERIFY)
class PkgStillHasEmptyPayload(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.inspection = "lostpayload"
        self.result = "VERIFY"


# Package lost payload across Koji builds (VERIFY)
class PkgLostPayload(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.inspection = "lostpayload"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Existing package is now missing across Koji builds (VERIFY)
class ExistingPkgMissing(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.before_rpm.add_subpackage(self.before_rpm.name + "-newthing")
        self.inspection = "lostpayload"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Subpackage continues to only carry %ghost entries (OK)
class GhostSubpkgStillEmpty(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.after_rpm.add_simple_payload_file()
        add_ghost_subpackage(self.before_rpm)
        add_ghost_subpackage(self.after_rpm)
        self.inspection = "lostpayload"
        self.result = "OK"


# Subpackage lost payload and now only carries %ghost entries (VERIFY)
class GhostSubpkgLostPayload(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.after_rpm.add_simple_payload_file()
        self.before_rpm.add_subpackage("ghost")
        self.before_rpm.add_installed_file(
            "/usr/share/vaporware.d/data",
            rpmfluff.SourceFile("data", "vaporware\n"),
            subpackageSuffix="ghost",
        )
        add_ghost_subpackage(self.after_rpm)
        self.inspection = "lostpayload"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Empty subpackage matches a glob in expected_empty (OK)
class ExpectedEmptyGlobStillEmpty(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.before_rpm.add_simple_payload_file()
        self.after_rpm.add_simple_payload_file()
        self.before_rpm.add_subpackage("foo+bar-devel")
        self.after_rpm.add_subpackage("foo+bar-devel")
        self.extra_cfg = {"emptyrpm": {"expected_empty": ["vaporware-*+*-devel"]}}
        self.inspection = "lostpayload"
        self.result = "OK"
