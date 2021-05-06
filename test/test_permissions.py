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

from baseclass import TestSRPM, TestRPMs, TestKoji
from baseclass import TestCompareSRPM, TestCompareRPMs, TestCompareKoji


class ExecutableWithSetUIDSRPM(TestSRPM):
    """
    Assert when a setuid file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class ExecutableWithSetUIDRPMs(TestRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"


class ExecutableWithSetUIDKoji(TestKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"


class ExecutableWithSetUIDCompareSRPM(TestCompareSRPM):
    """
    Assert when a setuid file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class ExecutableWithSetUIDCompareRPMs(TestCompareRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"


class ExecutableWithSetUIDCompareKoji(TestCompareKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/mount", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "INFO"


class UnapprovedExecutableWithSetUIDSRPM(TestSRPM):
    """
    Assert when a setuid file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class UnapprovedExecutableWithSetUIDRPMs(TestRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedExecutableWithSetUIDKoji(TestKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedExecutableWithSetUIDCompareSRPM(TestCompareSRPM):
    """
    Assert when a setuid file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "OK"


class UnapprovedExecutableWithSetUIDCompareRPMs(TestCompareRPMs):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class UnapprovedExecutableWithSetUIDCompareKoji(TestCompareKoji):
    """
    Assert when a setuid file is in a package and it's on the fileinfo list,
    INFO result occurs when testing the RPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/trojan", rpmfluff.SourceFile("file", "a" * 5), mode="4755"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHProhibitedSRPM(TestSRPM):
    """
    Assert when a world-writable file is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class FileIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHProhibitedCompareSRPM(TestCompareSRPM):
    """
    Assert when a world-writable file is in a package,
    OK result occurs when testing the SRPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class FileIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="0757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHandISVTXProhibitedSRPM(TestSRPM):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    OK result occurs when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class FileIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHandISVTXProhibitedCompareSRPM(TestCompareSRPM):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    OK result occurs when testing the SRPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "OK"


class FileIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable file with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_file(
            "/usr/bin/derp", rpmfluff.SourceFile("file", "a" * 5), mode="1757"
        )

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHProhibitedSRPM(TestSRPM):
    """
    Assert when a world-writable dir is in a package,
    OK result occures when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_directory("/usr/share/dumping-ground", mode="0757")

        self.inspection = "permissions"
        self.result = "OK"


class DirIWOTHProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_directory("/usr/share/dumping-ground", mode="0757")

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_directory("/usr/share/dumping-ground", mode="0757")

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHProhibitedCompareSRPM(TestCompareSRPM):
    """
    Assert when a world-writable dir is in a package,
    OK result occures when testing the SRPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_directory("/usr/share/dumping-ground", mode="0757")

        self.inspection = "permissions"
        self.result = "OK"


class DirIWOTHProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_directory("/usr/share/dumping-ground", mode="0757")

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_directory("/usr/share/dumping-ground", mode="0757")

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHandISVTXProhibitedSRPM(TestSRPM):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    OK result occures when testing the SRPM.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_directory("/usr/share/dumping-ground", mode="1757")

        self.inspection = "permissions"
        self.result = "OK"


class DirIWOTHandISVTXProhibitedRPMs(TestRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_directory("/usr/share/dumping-ground", mode="1757")

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHandISVTXProhibitedKoji(TestKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.rpm.add_installed_directory("/usr/share/dumping-ground", mode="1757")

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHandISVTXProhibitedCompareSRPM(TestCompareSRPM):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    OK result occures when testing the SRPMs.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_directory("/usr/share/dumping-ground", mode="1757")

        self.inspection = "permissions"
        self.result = "OK"


class DirIWOTHandISVTXProhibitedCompareRPMs(TestCompareRPMs):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_directory("/usr/share/dumping-ground", mode="1757")

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"


class DirIWOTHandISVTXProhibitedCompareKoji(TestCompareKoji):
    """
    Assert when a world-writable dir with the sticky bit is in a package,
    BAD result occurs requiring a Security waiver.
    """

    def setUp(self):
        super().setUp()

        self.after_rpm.add_installed_directory("/usr/share/dumping-ground", mode="1757")

        self.inspection = "permissions"
        self.result = "BAD"
        self.waiver_auth = "Security"
