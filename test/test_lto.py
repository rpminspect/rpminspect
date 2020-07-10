#
# Copyright (C) 2020  Red Hat, Inc.
# Author(s):  David Cantrell <dcantrell@redhat.com>
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

import os
import unittest
from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"]

# Source code used for the -flto tests
lto_src = open(datadir + "/lto.c").read()

# NOTE: The add_simple_compilation() calls to rpmfluff use '-o a.out' in
# the compileFlags due to a limitation in rpmfluff.  It always tries to
# copy a compiled file by the name of 'a.out' so if you are invoking gcc
# to compile a .o, you need to tell gcc to name the output a.out otherwise
# rpmfluff will fail.  This does not impact ELF executables which is the
# more common use.

# No LTO symbols in .o files (OK)
class NoLTOSymbolsRelocRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.label = "LTO"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class NoLTOSymbolsRelocKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.label = "LTO"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class NoLTOSymbolsRelocCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.label = "LTO"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class NoLTOSymbolsRelocCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.label = "LTO"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# No LTO symbols in .a files (OK)
class NoLTOSymbolsStaticLibRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # first create an object file
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.label = "LTO"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class NoLTOSymbolsStaticLibKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # first create an object file
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.label = "LTO"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class NoLTOSymbolsStaticLibCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # first create an object file
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.after_rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.label = "LTO"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class NoLTOSymbolsStaticLibCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # first create an object file
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -fno-lto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.after_rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.label = "LTO"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# LTO symbols present in .o files (BAD)
class LTOSymbolsRelocRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.label = "LTO"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsRelocKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.label = "LTO"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsRelocCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.label = "LTO"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsRelocCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )
        self.inspection = "lto"
        self.label = "LTO"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


# LTO symbols present in .a files (BAD)
class LTOSymbolsStaticLibRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # first create an object file
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.label = "LTO"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsStaticLibKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # first create an object file
        self.rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.label = "LTO"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsStaticLibCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # first create an object file
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.after_rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.label = "LTO"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"


class LTOSymbolsStaticLibCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # first create an object file
        self.after_rpm.add_simple_compilation(
            sourceContent=lto_src,
            compileFlags="-c -flto -o a.out",
            installPath="usr/lib/lto.o",
        )

        # now also create it as a .a file
        self.after_rpm.section_install += (
            "( cd $RPM_BUILD_ROOT/usr/lib ; ar r liblto.a lto.o )\n"
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/lib/liblto.a\n"

        self.inspection = "lto"
        self.label = "LTO"
        self.result = "BAD"
        self.waiver_auth = "Not Waivable"
