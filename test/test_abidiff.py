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

from baseclass import TestCompareRPMs, TestCompareKoji

# Simple source files for library ABI tests
old_library_source = """#include <math.h>

double exponent(double x, double y)
{
    return pow(x, y);
}
"""

new_library_source = """#include <stdio.h>

void exponent(void)
{
    return;
}
"""


# Test two builds that are not a rebase and do not change the ABI in
# an incompatible way (OK)
class AbidiffNoRebaseNoABIChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library()
        self.after_rpm.add_simple_library()

        self.inspection = "abidiff"
        self.result = "OK"


class AbidiffNoRebaseNoABIChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library()
        self.after_rpm.add_simple_library()

        self.inspection = "abidiff"
        self.result = "OK"


# Test two builds that are a rebase and do not break the ABI (OK)
class AbidiffRebaseNoABIChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        self.before_rpm.add_simple_library()
        self.after_rpm.add_simple_library()

        self.inspection = "abidiff"
        self.exitcode = 0
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class AbidiffRebaseNoABIChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_simple_library()
        self.after_rpm.add_simple_library()

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        self.inspection = "abidiff"
        self.exitcode = 0
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Test two builds that are a rebase and break the ABI (OK)
class AbidiffRebaseWithABIChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        self.before_rpm.add_simple_library(sourceContent=old_library_source)
        self.after_rpm.add_simple_library(sourceContent=new_library_source)

        self.inspection = "abidiff"
        self.exitcode = 0
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class AbidiffRebaseWithABIChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        # turn off all rpmbuild post processing stuff for the purposes of testing
        self.before_rpm.header += "\n%global __os_install_post %{nil}\n"
        self.after_rpm.header += "\n%global __os_install_post %{nil}\n"

        self.before_rpm.add_simple_library(sourceContent=old_library_source)
        self.after_rpm.add_simple_library(sourceContent=new_library_source)

        self.inspection = "abidiff"
        self.exitcode = 0
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Test two builds that are not a rebase and break the ABI (VERIFY)
class AbidiffNoRebaseWithABIChangeRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            sourceContent=old_library_source,
            compileFlags="-g -Wl,-soname,libcrashy.so.1",
        )
        self.after_rpm.add_simple_library(
            sourceContent=new_library_source,
            compileFlags="-g -Wl,-soname,libcrashy.so.1",
        )

        self.inspection = "abidiff"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class AbidiffNoRebaseWithABIChangeKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_simple_library(
            sourceContent=old_library_source,
            compileFlags="-g -Wl,-soname,libcrashy.so.1",
        )
        self.after_rpm.add_simple_library(
            sourceContent=new_library_source,
            compileFlags="-g -Wl,-soname,libcrashy.so.1",
        )

        self.inspection = "abidiff"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
