#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

from baseclass import TestKoji, TestRPMs, TestSRPM


# Add a subpackage that only carries a %ghost directory
def add_ghost_subpackage(rpm):
    rpm.add_subpackage("ghost")
    rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/usr/share/vaporware.d\n"
    rpm.get_subpackage("ghost").section_files += "%ghost %dir /usr/share/vaporware.d\n"


# Regular SRPM has payload (OK)
class HasPayloadSRPM(TestSRPM):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.inspection = "emptyrpm"
        self.result = "OK"


# Regular RPMs have payload (OK)
class HasPayloadRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.inspection = "emptyrpm"
        self.result = "OK"


# Regular Koji build has payload (OK)
class HasPayloadKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.inspection = "emptyrpm"
        self.result = "OK"


# Regular package has empty payload (VERIFY)
class PkgHasEmptyPayload(TestRPMs):
    def setUp(self):
        super().setUp()
        self.inspection = "emptyrpm"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Packages in Koji build have empty payloads (VERIFY)
class KojiBuildHaveEmptyPayloads(TestKoji):
    def setUp(self):
        super().setUp()
        self.inspection = "emptyrpm"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Package only carries %ghost entries (INFO)
class PkgHasOnlyGhostsRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.section_install += "mkdir -p $RPM_BUILD_ROOT/usr/share/vaporware.d\n"
        self.rpm.get_subpackage(
            None
        ).section_files += "%ghost %dir /usr/share/vaporware.d\n"
        self.inspection = "emptyrpm"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Subpackage in Koji build only carries %ghost entries (INFO)
class SubpkgHasOnlyGhostsKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        add_ghost_subpackage(self.rpm)
        self.inspection = "emptyrpm"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Empty subpackage is listed in expected_empty (INFO)
class ExpectedEmptyKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.rpm.add_subpackage("devel")
        self.extra_cfg = {"emptyrpm": {"expected_empty": ["vaporware-devel"]}}
        self.inspection = "emptyrpm"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Empty subpackage matches a glob in expected_empty (INFO)
class ExpectedEmptyGlobKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.rpm.add_subpackage("foo+bar-devel")
        self.extra_cfg = {"emptyrpm": {"expected_empty": ["vapor*+*-dev?l"]}}
        self.inspection = "emptyrpm"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Empty subpackage does not match a glob in expected_empty (VERIFY)
class ExpectedEmptyGlobMismatchKojiBuild(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_simple_payload_file()
        self.rpm.add_subpackage("devel")
        self.extra_cfg = {"emptyrpm": {"expected_empty": ["vaporware-*+*-devel"]}}
        self.inspection = "emptyrpm"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
