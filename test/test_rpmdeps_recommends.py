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
import rpm
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

# determine if the Recommends tag type is present in librpm
# version must be >= 4.12.0
rpmver = list(map(lambda x: int(x), rpm.__version__.strip().split("-")[0].split(".")))
rpm_major = rpmver[0]
rpm_minor = rpmver[1]
rpm_update = rpmver[2]

if rpm_major < 4 or (rpm_major == 4 and rpm_minor < 12):
    have_recommends = False
else:
    have_recommends = True

# need to know if we are on ALT Linux or not because rpmbuild
# on that platform prohibits unexpanded macros, so we can skip
# those test cases
on_alt_linux = False

if os.path.isfile("/etc/altlinux-release") or os.path.isfile("/etc/alt-release"):
    on_alt_linux = True

before_recommends = "important-package >= 2.0.2-47"
after_recommends = "important-package >= 4.7.0-1"
unexpanded_recommends = "important-package >= 4.7.1-1%{_macro}"

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


# Recommends dependency is correct (OK) - control case
class RecommendsCorrectRPMs(TestRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class RecommendsCorrectKoji(TestKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Retaining Recommends dependency in rebase comparison (INFO)
class RetainingRecommendsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_recommends(after_recommends)
        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RetainingRecommendsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_recommends(after_recommends)
        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Retaining Recommends dependency in maint comparison (OK)
class RetainingRecommendsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(after_recommends)
        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class RetainingRecommendsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(after_recommends)
        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Gaining a new Recommends in a rebase comparison (INFO)
class GainingRecommendsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class GainingRecommendsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Gaining a new Recommends in a maint comparison (VERIFY)
class GainingRecommendsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class GainingRecommendsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Changing a Recommends in a rebase comparison (INFO)
class ChangingRecommendsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_recommends(before_recommends)
        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingRecommendsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_recommends(before_recommends)
        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Changing a Recommends in a maint comparison (VERIFY)
class ChangingRecommendsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(before_recommends)
        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ChangingRecommendsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(before_recommends)
        self.after_rpm.add_recommends(after_recommends)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Changing a Recommends in a maint comparison due to NVR (INFO)
class ChangingRecommendsExpectedCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        # ok result because this is a source RPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class ChangingRecommendsExpectedCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingRecommendsExpectedCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Losing a Recommends in a rebase comparison (INFO)
class LosingRecommendsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_recommends(before_recommends)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class LosingRecommendsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_recommends(before_recommends)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Losing a Recommends in a maint comparison (VERIFY)
class LosingRecommendsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(before_recommends)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class LosingRecommendsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(before_recommends)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Missing Epoch prefix on maint compare (BAD for Koji compares, OK
# otherwise)
class MissingEpochRecommendsSRPM(TestSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # the result is OK here because the Epoch check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochRecommendsRPMs(TestRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # the result is OK here because the rpmdeps inspection when
        # run against just a pair of RPMs can't check for missing
        # Epoch values in manual dependencies, so the inspection
        # doesn't fail
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochRecommendsKoji(TestKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # result is OK for Recommends weak dep; Epoch check is for Provides/Requires pairs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochRecommendsCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        before_sub.add_recommends("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # this is 'OK' because the missing Epoch check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochRecommendsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        before_sub.add_recommends("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # this is 'OK' because the Compare RPMs tests don't work with the
        # entire collection of RPMs, it does it iteratively
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochRecommendsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        before_sub.add_recommends("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # result is OK for Recommends weak dep; Epoch check is for Provides/Requires pairs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Missing Epoch prefix on rebase compare (INFO)
class MissingEpochRecommendsRebaseCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        before_sub.add_recommends("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        # ok result because this is a source RPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MissingEpochRecommendsRebaseCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        before_sub.add_recommends("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class MissingEpochRecommendsRebaseCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # manually add a devel subpackage with an incorrect Recommends
        before_sub.add_recommends("%s = %s-%s" % (BEFORE_NAME, BEFORE_VER, BEFORE_REL))
        after_sub.add_recommends("%s = %s-%s" % (AFTER_NAME, AFTER_VER, AFTER_REL))

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Unexpanded macro in Recommends (BAD)
class UnexpandedMacroRecommendsSRPM(TestSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.rpm.add_recommends(unexpanded_recommends)

        # this is OK because it's the SRPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class UnexpandedMacroRecommendsRPMs(TestRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.rpm.add_recommends(unexpanded_recommends)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroRecommendsKoji(TestKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.rpm.add_recommends(unexpanded_recommends)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroRecommendsCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(unexpanded_recommends)
        self.after_rpm.add_recommends(unexpanded_recommends)

        # this is OK because it's the SRPM
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class UnexpandedMacroRecommendsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(unexpanded_recommends)
        self.after_rpm.add_recommends(unexpanded_recommends)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class UnexpandedMacroRecommendsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
    def setUp(self):
        super().setUp()

        self.before_rpm.add_recommends(unexpanded_recommends)
        self.after_rpm.add_recommends(unexpanded_recommends)

        self.inspection = "rpmdeps"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Missing explicit Recommends (VERIFY)
class MissingExplicitRecommendsSRPM(TestSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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


class MissingExplicitRecommendsRPMs(TestRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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


class MissingExplicitRecommendsKoji(TestKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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


class MissingExplicitRecommendsCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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


class MissingExplicitRecommendsCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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


class MissingExplicitRecommendsCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # add an explicit recommends on the libs package
        self.rpm.add_recommends("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MultipleProvidersRPMs(TestRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # add an explicit recommends on the libs package
        self.rpm.add_recommends("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for single RPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MultipleProvidersKoji(TestKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # add an explicit recommends on the libs package
        self.rpm.add_recommends("vaporware-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class MultipleProvidersCompareSRPM(TestCompareSRPM):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # add an explicit recommends on the libs package
        self.before_rpm.add_recommends("vaporware-libs = %{version}-%{release}")
        self.after_rpm.add_recommends("vaporware-libs = %{version}-%{release}")

        # result is OK because this check is a no-op for SRPMs
        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class MultipleProvidersCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # add an explicit recommends on the libs package
        self.before_rpm.add_recommends("vaporware-libs = %{version}-%{release}")
        self.after_rpm.add_recommends("vaporware-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class MultipleProvidersCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_recommends, "librpm too old to support Recommends")
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

        # add an explicit recommends on the libs package
        self.before_rpm.add_recommends("vaporware-libs = %{version}-%{release}")
        self.after_rpm.add_recommends("vaporware-libs = %{version}-%{release}")

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
