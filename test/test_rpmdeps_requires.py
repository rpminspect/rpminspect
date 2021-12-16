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


# Requires:
# only for before/after compares:
#   * VERIFY -> gaining a new Requires on maint compare
#   * INFO   -> gaining a new Requires on rebase compare
#   * INFO   -> retaining a Requires on either maint or rebase compare
#   * VERIFY -> changing a Requires on maint compare
#   * INFO   -> changing a Requires on rebase compare
#     INFO   -> changing a Requires on a maint compare that is expected with the NVR change
#   * VERIFY -> losing a Requires on maint compare
#   * INFO   -> losing a Requires on rebase compare
# for both before/after compares and single builds:
#     BAD    -> missing epoch prefix on maint compare
#     INFO   -> missing epoch prefix on rebase compare
#     VERIFY -> missing explicit requires (both compare and single build)
#     BAD    -> unexpanded macro (both compare and single build)
#     VERIFY -> multiple providers of the same dep

# Provides
# Conflicts
# Obsoletes
# Enhances
# Recommends
# Suggests
# Supplements


from baseclass import TestKoji, TestRPMs, TestSRPM
from baseclass import TestCompareKoji, TestCompareRPMs, TestCompareSRPM

before_requires = "important-package >= 2.0.2-47"
after_requires = "important-package >= 4.7.0-1"


# Requires dependency is correct (OK) - control case
class RequiresCorrectSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.add_build_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class RequiresCorrectRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class RequiresCorrectKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Retaining Requires dependency in rebase comparison (INFO)
class RetainingRequiresRebaseCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_build_requires(after_requires)
        self.after_rpm.add_build_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RetainingRequiresRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires(after_requires)
        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class RetainingRequiresRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires(after_requires)
        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Retaining Requires dependency in maint comparison (OK)
class RetainingRequiresCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_build_requires(after_requires)
        self.after_rpm.add_build_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class RetainingRequiresCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires(after_requires)
        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class RetainingRequiresCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires(after_requires)
        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


# Gaining a new Requires in a rebase comparison (INFO)
class GainingRequiresRebaseCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_build_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class GainingRequiresRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class GainingRequiresRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Gaining a new Requires in a maint comparison (VERIFY)
class GainingRequiresCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_build_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class GainingRequiresCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class GainingRequiresCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Changing a Requires in a rebase comparison (INFO)
class ChangingRequiresRebaseCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_build_requires(before_requires)
        self.after_rpm.add_build_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingRequiresRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires(before_requires)
        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class ChangingRequiresRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires(before_requires)
        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Changing a Requires in a maint comparison (VERIFY)
class ChangingRequiresCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_build_requires(before_requires)
        self.after_rpm.add_build_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ChangingRequiresCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires(before_requires)
        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class ChangingRequiresCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires(before_requires)
        self.after_rpm.add_requires(after_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Losing a Requires in a rebase comparison (INFO)
class LosingRequiresRebaseCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_build_requires(before_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class LosingRequiresRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires(before_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class LosingRequiresRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_requires(before_requires)

        self.inspection = "rpmdeps"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# Losing a Requires in a maint comparison (VERIFY)
class LosingRequiresCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_build_requires(before_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class LosingRequiresCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires(before_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class LosingRequiresCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_requires(before_requires)

        self.inspection = "rpmdeps"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
