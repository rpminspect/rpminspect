#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
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


# File removed from an after build but it's not a security-related file (OK)
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


# File removed from an after build from a security path (BAD)
class SecurityFileRemovedRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # create the test packages
        self.before_rpm.add_installed_file(
            "etc/sudoers.d/s3kr1t",
            rpmfluff.SourceFile("s3kr1t.sudo", "# This is a security related file!"),
            mode="0644",
        )

        self.inspection = "removedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileRemovedRebaseRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

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
        self.result = "BAD"
        self.waiver_auth = "Security"


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
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class SecurityFileRemovedKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # create the test packages
        self.before_rpm.add_installed_file(
            "etc/sudoers.d/s3kr1t",
            rpmfluff.SourceFile("s3kr1t.sudo", "# This is a security related file!"),
            mode="0644",
        )

        self.inspection = "removedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileRemovedRebaseKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        with open(os.environ["RPMINSPECT"], "rb") as f:
            before_src = f.read()

        # create the test packages
        self.before_rpm.add_installed_file(
            "usr/bin/mount", rpmfluff.SourceFile("mount.bin", before_src), mode="0755"
        )

        self.inspection = "removedfiles"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
