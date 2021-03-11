#
# Copyright © 2019 Red Hat, Inc.
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

import os
import subprocess
import unittest

import rpmfluff

try:
    from rpmfluff import simple_library_source
except ImportError:
    from rpmfluff.samples import simple_library_source

from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"]

# Source code used for the -D_FORTIFY_SOURCE tests
fortify_src = open(datadir + "/fortify.c").read()

# Simple source file for library tests
test_library_source = """#include <math.h>

double exponent(double x, double y)
{
    return pow(x, y);
}
"""

# Figure out if the system is 32-bit capable or not
have_gcc_multilib = False
args = ["gcc", "-print-multi-lib"]
proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
(out, err) = proc.communicate()

if proc.returncode == 0 and str(out).find("@m32") != -1:
    have_gcc_multilib = True


# Simple way to figure out if we are musl or glibc
have_musl_libc = False
args = ["patchelf", "--print-interpreter", "/sbin/init"]
proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
(out, err) = proc.communicate()

if proc.returncode == 0 and str(out).find("ld-musl") != -1:
    have_musl_libc = True


# Program built with noexecstack
class WithoutExecStackRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Not Waivable"
        self.result = "OK"


class WithoutExecStackKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Not Waivable"
        self.result = "OK"


class WithoutExecStackCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Not Waivable"
        self.result = "OK"


class WithoutExecStackCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,noexecstack")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Not Waivable"
        self.result = "OK"


# Program built with execstack
class WithExecStackRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_simple_compilation(compileFlags="-Wl,-z,execstack")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


class WithExecStackKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_simple_compilation(compileFlags="-Wl,-z,execstack")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


class WithExecStackCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,execstack")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,execstack")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class WithExecStackCompare(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,execstack")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,execstack")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


# Program lost full RELRO
class LostFullRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,now")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,norelro")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


class LostFullRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,now")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,norelro")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


# Program lost full RELRO but retained partial RELRO
class FulltoPartialRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,now")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,lazy")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


class FulltoPartialRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,now")
        self.after_rpm.add_simple_compilation(compileFlags="-Wl,-z,relro,-z,lazy")
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


# Program lost -D_FORTIFY_SOURCE
class LostFortifySourceCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(have_musl_libc, "musl libc lacks _FORTIFY_SOURCE support")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Enabling FORTIFY_SOURCE requires -O1 or higher
        self.before_rpm.add_simple_compilation(
            sourceContent=fortify_src,
            compileFlags="-fno-stack-protector -O2 -D_FORTIFY_SOURCE",
        )
        self.after_rpm.add_simple_compilation(
            sourceContent=fortify_src,
            compileFlags="-fno-stack-protector -O2 -D_FORTIFY_SOURCE=0",
        )
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class LostFortifySourceCompareKoji(TestCompareKoji):
    @unittest.skipIf(have_musl_libc, "musl libc lacks _FORTIFY_SOURCE support")
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Enabling FORTIFY_SOURCE requires -O1 or higher
        self.before_rpm.add_simple_compilation(
            sourceContent=fortify_src,
            compileFlags="-fno-stack-protector -O2 -D_FORTIFY_SOURCE",
        )
        self.after_rpm.add_simple_compilation(
            sourceContent=fortify_src,
            compileFlags="-fno-stack-protector -O2 -D_FORTIFY_SOURCE=0",
        )
        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


# Program lost -fPIC in after (BAD, WAIVABLE_BY_SECURITY)
class LostPICCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        installPath = "usr/lib/libsimple.a"

        self.before_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.before_rpm.section_build += "gcc -m32 -fPIC -c simple.c\n"
        self.before_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.after_rpm.section_build += "gcc -m32 -c simple.c\n"
        self.after_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


class LostPICCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        TestCompareKoji.setUp(self)

        installPath = "usr/lib/libsimple.a"

        self.before_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.before_rpm.section_build += "gcc -m32 -fPIC -c simple.c\n"
        self.before_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(rpmfluff.SourceFile("simple.c", test_library_source))
        self.after_rpm.section_build += "gcc -m32 -c simple.c\n"
        self.after_rpm.section_build += "ar -crs libsimple.a simple.o\n"
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.a $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


# Program has or gained TEXTREL relocations (32-bit arches only)
class HasTEXTRELRPMs(TestRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        TestRPMs.setUp(self)

        installPath = "usr/lib/libfoo.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.rpm.add_source(rpmfluff.SourceFile("simple.c", simple_library_source))
        self.rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


class HasTEXTRELCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        installPath = "usr/lib/libfoo.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"


class HasTEXTRELCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_gcc_multilib, "gcc lacks multilib (-m32) support")
    def setUp(self):
        TestCompareKoji.setUp(self)

        installPath = "usr/lib/libfoo.so"

        # Can't use rpmfluff here because it always adds -fPIC
        self.before_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.before_rpm.section_build += (
            "gcc -m32 -fPIC -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(
            rpmfluff.SourceFile("simple.c", simple_library_source)
        )
        self.after_rpm.section_build += (
            "gcc -m32 -fno-pic -shared -Wl,-z,noexecstack -o libsimple.so simple.c\n"
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += (
            "cp libsimple.so $RPM_BUILD_ROOT/%s\n" % installPath
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "elf"
        self.label = "elf-object-properties"
        self.waiver_auth = "Security"
        self.result = "BAD"
