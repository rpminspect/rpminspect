#
# Copyright 2022 Red Hat, Inc.
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

from baseclass import (
    TestSRPM,
    TestCompareSRPM,
    TestRPMs,
    TestKoji,
    TestCompareRPMs,
    TestCompareKoji,
)


debuginfo_remove = "\n%define _find_debuginfo_opts "
debuginfo_remove += '"--remove-section .symtab '
debuginfo_remove += '--remove-section .debug_info"\n'

debuginfo_keep = "\n%define _find_debuginfo_opts "
debuginfo_keep += '"--keep-section .symtab '
debuginfo_keep += "--keep-section .debug_info "
debuginfo_keep += '--keep-section .gnu_debugdata"\n'


# Missing sections in the debuginfo package (BAD, except for SRPMs)
class MissingSectionsInDebuginfoPkgSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += debuginfo_remove
        self.rpm.add_simple_compilation(compileFlags="-g")

        # this inspection is a no-op on SRPMs
        self.inspection = "debuginfo"
        self.result = "OK"


class MissingSectionsInDebuginfoPkgCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += debuginfo_remove
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.header += debuginfo_remove
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        # this inspection is a no-op on SRPMs
        self.inspection = "debuginfo"
        self.result = "OK"


class MissingSectionsInDebuginfoPkgRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.header += debuginfo_remove
        self.rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class MissingSectionsInDebuginfoPkgCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += debuginfo_remove
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.header += debuginfo_remove
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class MissingSectionsInDebuginfoPkgKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.header += debuginfo_remove
        self.rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class MissingSectionsInDebuginfoPkgCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += debuginfo_remove
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.header += debuginfo_remove
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Has debugging sections in the main package (BAD, except for SRPMs)
class HaveDebuggingSectionsInRegularPkgSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += debuginfo_keep
        self.rpm.add_simple_compilation(compileFlags="-g")

        # this inspection is a no-op on SRPMs
        self.inspection = "debuginfo"
        self.result = "OK"


class HaveDebuggingSectionsInRegularPkgCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += debuginfo_keep
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.header += debuginfo_keep
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        # this inspection is a no-op on SRPMs
        self.inspection = "debuginfo"
        self.result = "OK"


class HaveDebuggingSectionsInRegularPkgRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.header += debuginfo_keep
        self.rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HaveDebuggingSectionsInRegularPkgCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += debuginfo_keep
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.header += debuginfo_keep
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HaveDebuggingSectionsInRegularPkgKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.header += debuginfo_keep
        self.rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HaveDebuggingSectionsInRegularPkgCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += debuginfo_keep
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.header += debuginfo_keep
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Before build is stripped but now not stripped in the after build
class BeforeStrippedAfterNotStrippedCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.header += debuginfo_keep
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# XXX: need to figure out find_debuginfo_opts more
# class BeforeStrippedAfterNotStrippedDebugInfoCompareKoji(TestCompareKoji):
#    def setUp(self):
#        super().setUp()
#
#        self.extra_cfg = {}
#        self.extra_cfg["debuginfo"] = {}
#        self.extra_cfg["debuginfo"]["debuginfo_sections"] = ".debug_info"
#
#        self.before_rpm.header += debuginfo_remove
#        self.before_rpm.add_simple_compilation(compileFlags="")
#
#        self.after_rpm.add_simple_compilation(compileFlags="-g")
#
#        self.inspection = "debuginfo"
#        self.result = "INFO"
#        self.waiver_auth = "Not Waivable"


# XXX:
# Before build is not stripped, but the after build is
# class BeforeNotStrippedAfterStrippedCompareKoji(TestCompareKoji):
#    def setUp(self):
#        super().setUp()
#
#        self.extra_cfg = {}
#        self.extra_cfg["debuginfo"] = {}
#        self.extra_cfg["debuginfo"]["debuginfo_sections"] = ".debug_info"
#
#        self.after_rpm.header += debuginfo_keep
#        self.before_rpm.add_simple_compilation(compileFlags="-g")
#
#        self.after_rpm.add_simple_compilation(compileFlags="-g")
#
#        self.inspection = "debuginfo"
#        self.result = "INFO"
#        self.waiver_auth = "Not Waivable"

# XXX:
# class BeforeNotStrippedAfterStrippedCompareKoji(TestCompareKoji):
#    def setUp(self):
#        super().setUp()
#
#        self.extra_cfg = {}
#        self.extra_cfg["debuginfo"] = {}
#        self.extra_cfg["debuginfo"]["debuginfo_sections"] = ".debug_info"
#
#        self.after_rpm.header += debuginfo_keep
#        self.before_rpm.add_simple_compilation(compileFlags="-g")
#
#        self.after_rpm.add_simple_compilation(compileFlags="-g")
#
#        self.inspection = "debuginfo"
#        self.result = "BAD"
#        self.waiver_auth = "Anyone"
