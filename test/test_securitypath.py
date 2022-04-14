#
# Copyright © 2021 Red Hat, Inc.
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

from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

contents = """
# Open things up
%wheel   ALL=(ALL) NOPASSWD: ALL
"""


################################################
# New file present or added to a security path #
################################################


# XXX: fix coming with a larger addedfiles inspection fix
# class FileAddedToSecurityPathRPMs(TestRPMs):
#    def setUp(self):
#        super().setUp()
#        self.rpm.add_installed_file(
#            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
#        )
#        self.inspection = "addedfiles"
#        self.result = "BAD"
#        self.waiver_auth = "Security"


class SecuritySKIPFileAddedToSecurityPathRPMs(TestRPMs):
    def setUp(self):
        super().setUp()
        self.rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# XXX: fix coming with a larger addedfiles inspection fix
# class SecurityINFORMFileAddedToSecurityPathRPMs(TestRPMs):
#    def setUp(self):
#        super().setUp()
#        self.rpm.add_installed_file(
#            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
#        )
#        self.inspection = "addedfiles"
#        self.result = "INFO"
#        self.waiver_auth = "Security"


# XXX: fix coming with a larger addedfiles inspection fix
# class SecurityVERIFYFileAddedToSecurityPathRPMs(TestRPMs):
#    def setUp(self):
#        super().setUp()
#        self.rpm.add_installed_file(
#            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
#        )
#        self.inspection = "addedfiles"
#        self.result = "VERIFY"
#        self.waiver_auth = "Security"


# XXX: fix coming with a larger addedfiles inspection fix
# class SecurityFAILFileAddedToSecurityPathRPMs(TestRPMs):
#    def setUp(self):
#        super().setUp()
#        self.rpm.add_installed_file(
#            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
#        )
#        self.inspection = "addedfiles"
#        self.result = "BAD"
#        self.waiver_auth = "Security"


# XXX: fix coming with a larger addedfiles inspection fix
# class FileAddedToSecurityPathKoji(TestKoji):
#    def setUp(self):
#        super().setUp()
#        self.rpm.add_installed_file(
#            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
#        )
#        self.inspection = "addedfiles"
#        self.result = "BAD"
#        self.waiver_auth = "Security"


class SecuritySKIPFileAddedToSecurityPathKoji(TestKoji):
    def setUp(self):
        super().setUp()
        self.rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# XXX: fix coming with a larger addedfiles inspection fix
# class SecurityINFORMFileAddedToSecurityPathKoji(TestKoji):
#    def setUp(self):
#        super().setUp()
#        self.rpm.add_installed_file(
#            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
#        )
#        self.inspection = "addedfiles"
#        self.result = "INFO"
#        self.waiver_auth = "Security"


# XXX: fix coming with a larger addedfiles inspection fix
# class SecurityVERIFYFileAddedToSecurityPathKoji(TestKoji):
#    def setUp(self):
#        super().setUp()
#        self.rpm.add_installed_file(
#            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
#        )
#        self.inspection = "addedfiles"
#        self.result = "VERIFY"
#        self.waiver_auth = "Security"


# XXX: fix coming with a larger addedfiles inspection fix
# class SecurityFAILFileAddedToSecurityPathKoji(TestKoji):
#    def setUp(self):
#        super().setUp()
#        self.rpm.add_installed_file(
#            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
#        )
#        self.inspection = "addedfiles"
#        self.result = "BAD"
#        self.waiver_auth = "Security"


class FileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecurityINFORMFileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileAddedToSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecurityINFORMFileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileAddedToSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


##################################################
# File removed from securith path in after build #
##################################################


class FileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecurityINFORMFileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileRemovedFromSecurityPathCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class FileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/wheel", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"


class SecuritySKIPFileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/skip", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class SecurityINFORMFileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/inform", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "INFO"
        self.waiver_auth = "Security"


class SecurityVERIFYFileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/verify", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "VERIFY"
        self.waiver_auth = "Security"


class SecurityFAILFileRemovedFromSecurityPathCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()
        self.after_rpm.add_installed_file(
            "/etc/sudoers.d/fail", rpmfluff.SourceFile("wheel", contents)
        )
        self.inspection = "addedfiles"
        self.result = "BAD"
        self.waiver_auth = "Security"
