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

import os
import unittest

from rpmfluff.sourcefile import SourceFile
from rpmfluff.utils import CC

from baseclass import TestKoji, TestRPMs, TestSRPM
from baseclass import TestCompareKoji, TestCompareRPMs, TestCompareSRPM
from baseclass import BEFORE_NAME, BEFORE_VER, BEFORE_REL
from baseclass import AFTER_NAME, AFTER_VER, AFTER_REL

# determine if we have /usr/lib/rpm/elfdeps
# (missing on Alpine Linux as of 14-Jan-2022)
elfdeps = "/usr/lib/rpm/elfdeps"
have_elfdeps = False

if os.path.isfile(elfdeps) and os.access(elfdeps, os.X_OK):
    have_elfdeps = True

# need to know if we are on ALT Linux or not because rpmbuild
# on that platform prohibits unexpanded macros, so we can skip
# those test cases
on_alt_linux = False

if os.path.isfile("/etc/altlinux-release") or os.path.isfile("/etc/alt-release"):
    on_alt_linux = True

before_obsoletes = "important-package >= 2.0.2-47"
after_obsoletes = "important-package >= 4.7.0-1"
unexpanded_obsoletes = "important-package >= 4.7.1-1%{_macro}"

# library source code for use in these package builds
library_source = """#include <stdio.h>

void greet(const char *message)
{
    printf ("%s\\n", message);
}
"""

# source to use as /usr/bin example program in package builds
hello_lib_world = """#include <stdio.h>

void greet(const char *message);

int
main (int argc, char **argv)
{
    greet ("Hello world\\n");

    return 0;
}
"""


# Obsoletes dependency is correct (OK) - control case
class ObsoletesCorrectRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class ObsoletesCorrectKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Retaining Obsoletes dependency in rebase comparison (INFO)
class RetainingObsoletesRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_obsoletes(after_obsoletes)
        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RetainingObsoletesRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_obsoletes(after_obsoletes)
        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Retaining Obsoletes dependency in maint comparison (OK)
class RetainingObsoletesCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(after_obsoletes)
        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class RetainingObsoletesCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(after_obsoletes)
        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Gaining a new Obsoletes in a rebase comparison (INFO)
class GainingObsoletesRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class GainingObsoletesRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Gaining a new Obsoletes in a maint comparison (VERIFY)
class GainingObsoletesCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class GainingObsoletesCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Changing a Obsoletes in a rebase comparison (INFO)
class ChangingObsoletesRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_obsoletes(before_obsoletes)
        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingObsoletesRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_obsoletes(before_obsoletes)
        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Changing a Obsoletes in a maint comparison (VERIFY)
class ChangingObsoletesCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(before_obsoletes)
        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ChangingObsoletesCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(before_obsoletes)
        self.after_rpm.add_obsoletes(after_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Changing a Obsoletes in a maint comparison due to NVR (INFO)
class ChangingObsoletesExpectedCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # ok result because this is a source RPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class ChangingObsoletesExpectedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingObsoletesExpectedCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Losing a Obsoletes in a rebase comparison (INFO)
class LosingObsoletesRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_obsoletes(before_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class LosingObsoletesRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_obsoletes(before_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Losing a Obsoletes in a maint comparison (VERIFY)
class LosingObsoletesCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(before_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class LosingObsoletesCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(before_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Missing Epoch prefix on maint compare (BAD for Koji compares, OK
# otherwise)
class MissingEpochObsoletesSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.rpm.epoch = 47

        # add a subpackage
        sub = self.rpm.add_subpackage("devel")
        sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # the result is OK here because the Epoch check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochObsoletesRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.rpm.epoch = 47

        # add a subpackage
        sub = self.rpm.add_subpackage("devel")
        sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # the result is OK here because the rpmdeps inspection when
        # run against just a pair of RPMs can't check for missing
        # Epoch values in manual dependencies, so the inspection
        # doesn't fail
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochObsoletesKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.rpm.epoch = 47

        # add a subpackage
        sub = self.rpm.add_subpackage("devel")
        sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class MissingEpochObsoletesCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        before_sub.add_obsoletes("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # this is 'OK' because the missing Epoch check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochObsoletesCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        before_sub.add_obsoletes("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # this is 'OK' because the Compare RPMs tests don't work with the
        # entire collection of RPMs, it does it iteratively
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochObsoletesCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        before_sub.add_obsoletes("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Missing Epoch prefix on rebase compare (INFO)
class MissingEpochObsoletesRebaseCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        before_sub.add_obsoletes("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # ok result because this is a source RPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochObsoletesRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        before_sub.add_obsoletes("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class MissingEpochObsoletesRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # define an epoch value
        self.before_rpm.epoch = 47
        self.after_rpm.epoch = 47

        # add a subpackage
        before_sub = self.before_rpm.add_subpackage("devel")
        before_sub.group = "Development/Libraries"
        after_sub = self.after_rpm.add_subpackage("devel")
        after_sub.group = "Development/Libraries"

        # we need some stuff in the packages
        self.before_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.after_rpm.add_simple_compilation(installPath="usr/bin/vaporware")
        self.before_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            installPath="usr/lib/libvaporware.so.1",
            subpackageSuffix="devel",
            sourceContent=library_source,
        )

        # manually add a devel subpackage with an incorrect Obsoletes
        before_sub.add_obsoletes("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_obsoletes("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Unexpanded macro in Obsoletes (BAD)
class UnexpandedMacroObsoletesSRPM(TestSRPM):
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.rpm.add_obsoletes(unexpanded_obsoletes)

        # this is OK because it's the SRPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class UnexpandedMacroObsoletesRPMs(TestRPMs):
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.rpm.add_obsoletes(unexpanded_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroObsoletesKoji(TestKoji):
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.rpm.add_obsoletes(unexpanded_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroObsoletesCompareSRPM(TestCompareSRPM):
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(unexpanded_obsoletes)
        self.after_rpm.add_obsoletes(unexpanded_obsoletes)

        # this is OK because it's the SRPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class UnexpandedMacroObsoletesCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(unexpanded_obsoletes)
        self.after_rpm.add_obsoletes(unexpanded_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroObsoletesCompareKoji(TestCompareKoji):
    @unittest.skipIf(on_alt_linux, "ALT Linux rpmbuild prohibits unexpanded macros")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_obsoletes(unexpanded_obsoletes)
        self.after_rpm.add_obsoletes(unexpanded_obsoletes)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Missing explicit Obsoletes (VERIFY)
class MissingExplicitObsoletesSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"
        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # result is OK because rpminspect can't report missing deps on SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingExplicitObsoletesRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"
        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # result is OK because rpminspect can't report missing deps when just
        # looking at a single RPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingExplicitObsoletesKoji(TestKoji):
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"
        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class MissingExplicitObsoletesCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.before_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # result is OK because rpminspect can't report missing deps on SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingExplicitObsoletesCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.before_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # result is OK because rpminspect can't report missing deps on single
        # RPM comparisons
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingExplicitObsoletesCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add a subpackage
        self.before_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("libs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Multiple providers of the same thing (VERIFY)
class MultipleProvidersSRPM(TestSRPM):
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.rpm.add_subpackage("libs")
        self.rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # add an explicit obsoletes on the libs package
        self.rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MultipleProvidersRPMs(TestRPMs):
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.rpm.add_subpackage("libs")
        self.rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # add an explicit obsoletes on the libs package
        self.rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for single RPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MultipleProvidersKoji(TestKoji):
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.rpm.add_subpackage("libs")
        self.rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.rpm.add_payload_check(installPath, None)

        # add an explicit obsoletes on the libs package
        self.rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class MultipleProvidersCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.before_rpm.add_subpackage("libs")
        self.before_rpm.add_subpackage("morelibs")
        self.after_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # add an explicit obsoletes on the libs package
        self.before_rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")
        self.after_rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# XXX: this test needs to be rewritten to properly dupe the shared library
# class MultipleProvidersCompareRPMs(TestCompareRPMs):
#    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
#    def setUp(self):
#        super().setUp()
#
#        # add subpackages
#        self.before_rpm.add_subpackage("libs")
#        self.before_rpm.add_subpackage("morelibs")
#        self.after_rpm.add_subpackage("libs")
#        self.after_rpm.add_subpackage("morelibs")
#
#        # we need some stuff in the packages, first a shared library
#        self.before_rpm.add_simple_library(
#            libraryName="libvaporware.so",
#            installPath="usr/lib/libvaporware.so",
#            subpackageSuffix="libs",
#            sourceContent=library_source,
#        )
#        self.before_rpm.add_simple_library(
#            libraryName="libvaporware.so",
#            installPath="usr/lib/libvaporware.so",
#            subpackageSuffix="morelibs",
#            sourceContent=library_source,
#        )
#        self.after_rpm.add_simple_library(
#            libraryName="libvaporware.so",
#            installPath="usr/lib/libvaporware.so",
#            subpackageSuffix="libs",
#            sourceContent=library_source,
#        )
#        self.after_rpm.add_simple_library(
#            libraryName="libvaporware.so",
#            installPath="usr/lib/libvaporware.so",
#            subpackageSuffix="morelibs",
#            sourceContent=library_source,
#        )
#
#        # now add a program linked with that library
#        sourceFileName = "main.c"
#        installPath = "usr/bin/vaporware"
#
#        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
#        self.before_rpm.section_build += (
#            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
#        )
#        self.before_rpm.section_build += (
#            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
#        )
#        self.before_rpm.create_parent_dirs(installPath)
#        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
#        binsub = self.before_rpm.get_subpackage(None)
#        binsub.section_files += "/%s\n" % installPath
#        self.before_rpm.add_payload_check(installPath, None)
#
#        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
#        self.after_rpm.section_build += (
#            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
#        )
#        self.after_rpm.section_build += (
#            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
#        )
#        self.after_rpm.create_parent_dirs(installPath)
#        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
#        binsub = self.after_rpm.get_subpackage(None)
#        binsub.section_files += "/%s\n" % installPath
#        self.after_rpm.add_payload_check(installPath, None)
#
#        # add an explicit obsoletes on the libs package
#        self.before_rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")
#        self.after_rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")
#
#        self.inspection = "rpmdeps"
#        self.result = "VERIFY"
#        self.waiver_auth = "Anyone"


class MultipleProvidersCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_elfdeps, "system lacks %s executable" % elfdeps)
    def setUp(self):
        super().setUp()

        # add subpackages
        self.before_rpm.add_subpackage("libs")
        self.before_rpm.add_subpackage("morelibs")
        self.after_rpm.add_subpackage("libs")
        self.after_rpm.add_subpackage("morelibs")

        # we need some stuff in the packages, first a shared library
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.before_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="libs",
            sourceContent=library_source,
        )
        self.after_rpm.add_simple_library(
            libraryName="libvaporware.so",
            installPath="usr/lib/libvaporware.so",
            subpackageSuffix="morelibs",
            sourceContent=library_source,
        )

        # now add a program linked with that library
        sourceFileName = "main.c"
        installPath = "usr/bin/vaporware"

        self.before_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.before_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.before_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.before_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.before_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.before_rpm.add_payload_check(installPath, None)

        self.after_rpm.add_source(SourceFile(sourceFileName, hello_lib_world))
        self.after_rpm.section_build += (
            "%if 0%{?__isa_bits} == 32\n%define mopt -m32\n%endif\n"
        )
        self.after_rpm.section_build += (
            CC + " %%{?mopt} %s -L. -lvaporware\n" % sourceFileName
        )
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "cp a.out $RPM_BUILD_ROOT/%s\n" % installPath
        binsub = self.after_rpm.get_subpackage(None)
        binsub.section_files += "/%s\n" % installPath
        self.after_rpm.add_payload_check(installPath, None)

        # add an explicit obsoletes on the libs package
        self.before_rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")
        self.after_rpm.add_obsoletes("vaporware-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
