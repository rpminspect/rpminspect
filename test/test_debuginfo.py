#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import unittest

from baseclass import (
    TestSRPM,
    TestCompareSRPM,
    TestRPMs,
    TestKoji,
    TestCompareRPMs,
    TestCompareKoji,
)


# Manual block for %install that removes a debuginfo symbol
debug_block = "%{__debug_install_post}\n"
debug_block += "chmod 0755 $RPM_BUILD_ROOT/usr/lib/debug/usr/bin/hello-world*.debug\n"
debug_block += "fname=$(ls -1 $RPM_BUILD_ROOT/usr/lib/debug/usr/bin/hello-world*.debug | head -n 1)\n"
debug_block += "strip --remove-section=.symtab -o ${fname}.new ${fname}\n"
debug_block += "mv ${fname}.new ${fname}\n"
debug_block += "chmod 0444 ${fname}\n"


# Missing .gdb_index in the debuginfo package (BAD, except for SRPMs)
class MissingSectionsInDebuginfoPkgSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.makeDebugInfo = True
        self.rpm.add_simple_compilation(compileFlags="-g")
        self.rpm.section_install += debug_block

        # this inspection is a no-op on SRPMs
        self.inspection = "debuginfo"
        self.result = "OK"


class MissingSectionsInDebuginfoPkgCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.makeDebugInfo = True
        self.before_rpm.add_simple_compilation(compileFlags="-g")
        self.before_rpm.section_install += debug_block

        self.after_rpm.makeDebugInfo = True
        self.after_rpm.add_simple_compilation(compileFlags="-g")
        self.after_rpm.section_install += debug_block

        # this inspection is a no-op on SRPMs
        self.inspection = "debuginfo"
        self.result = "OK"


class MissingSectionsInDebuginfoPkgRPMs(TestRPMs):
    @unittest.skipIf(
        os.uname().sysname == "FreeBSD", "FreeBSD lacks find-debuginfo from debugedit"
    )
    def setUp(self):
        super().setUp()

        self.rpm.makeDebugInfo = True
        self.rpm.add_simple_compilation(compileFlags="-g")
        self.rpm.section_install += debug_block

        # this is a no-op because the RPM used is not a debuginfo RPM
        self.inspection = "debuginfo"
        self.result = "OK"


class MissingSectionsInDebuginfoPkgCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(
        os.uname().sysname == "FreeBSD", "FreeBSD lacks find-debuginfo from debugedit"
    )
    def setUp(self):
        super().setUp()

        self.before_rpm.makeDebugInfo = True
        self.before_rpm.add_simple_compilation(compileFlags="-g")
        self.before_rpm.section_install += debug_block

        self.after_rpm.makeDebugInfo = True
        self.after_rpm.add_simple_compilation(compileFlags="-g")
        self.after_rpm.section_install += debug_block

        # this is a no-op because the RPM used is not a debuginfo RPM
        self.inspection = "debuginfo"
        self.result = "OK"


class MissingSectionsInDebuginfoPkgKoji(TestKoji):
    @unittest.skipIf(
        os.uname().sysname == "FreeBSD", "FreeBSD lacks find-debuginfo from debugedit"
    )
    def setUp(self):
        super().setUp()

        self.rpm.makeDebugInfo = True
        self.rpm.add_simple_compilation(compileFlags="-g")
        self.rpm.section_install += debug_block

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class MissingSectionsInDebuginfoPkgCompareKoji(TestCompareKoji):
    @unittest.skipIf(
        os.uname().sysname == "FreeBSD", "FreeBSD lacks find-debuginfo from debugedit"
    )
    def setUp(self):
        super().setUp()

        self.before_rpm.makeDebugInfo = True
        self.before_rpm.add_simple_compilation(compileFlags="-g")
        self.before_rpm.section_install += debug_block

        self.after_rpm.makeDebugInfo = True
        self.after_rpm.add_simple_compilation(compileFlags="-g")
        self.after_rpm.section_install += debug_block

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Has debugging sections in the main package (BAD, except for SRPMs)
class HaveDebuggingSectionsInRegularPkgSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # don't need debuginfo for SRPM test
        self.rpm.makeDebugInfo = False
        self.rpm.add_simple_compilation(compileFlags="-g")

        # this inspection is a no-op on SRPMs
        self.inspection = "debuginfo"
        self.result = "OK"


class HaveDebuggingSectionsInRegularPkgCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # don't need debuginfo for SRPM test
        self.before_rpm.makeDebugInfo = False
        self.after_rpm.makeDebugInfo = False

        self.before_rpm.add_simple_compilation(compileFlags="-g")
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        # this inspection is a no-op on SRPMs
        self.inspection = "debuginfo"
        self.result = "OK"


class HaveDebuggingSectionsInRegularPkgRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # don't need debuginfo for SRPM test
        self.rpm.makeDebugInfo = False
        self.rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HaveDebuggingSectionsInRegularPkgCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.makeDebugInfo = False
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.makeDebugInfo = False
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HaveDebuggingSectionsInRegularPkgKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.makeDebugInfo = False
        self.rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class HaveDebuggingSectionsInRegularPkgCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.makeDebugInfo = False
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.makeDebugInfo = False
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Before build is stripped but now not stripped in the after build
class BeforeStrippedAfterNotStrippedCompareKoji(TestCompareKoji):
    @unittest.skipIf(
        os.uname().sysname == "FreeBSD", "FreeBSD lacks find-debuginfo from debugedit"
    )
    def setUp(self):
        super().setUp()

        self.before_rpm.makeDebugInfo = True
        self.before_rpm.add_simple_compilation(compileFlags="-g")
        self.before_rpm.section_install += debug_block

        self.after_rpm.makeDebugInfo = True
        self.after_rpm.add_simple_compilation(compileFlags="-g")

        self.inspection = "debuginfo"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Before build is not stripped, but the after build is
class BeforeNotStrippedAfterStrippedCompareKoji(TestCompareKoji):
    @unittest.skipIf(
        os.uname().sysname == "FreeBSD", "FreeBSD lacks find-debuginfo from debugedit"
    )
    def setUp(self):
        super().setUp()

        self.before_rpm.makeDebugInfo = True
        self.before_rpm.add_simple_compilation(compileFlags="-g")

        self.after_rpm.makeDebugInfo = True
        self.after_rpm.add_simple_compilation(compileFlags="-g")
        self.after_rpm.section_install += debug_block

        self.inspection = "debuginfo"
        self.result = "BAD"
        self.waiver_auth = "Anyone"
