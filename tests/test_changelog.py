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

import datetime
from baseclass import *

# changelog tests only apply to before & after build comparisons.
# Perform the following each as a separate test:
#
# In the source RPM:
# 1) Remove the %changelog section in the after build.  The test should
#    report that as VERIFY.
class LostChangeLogCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.after_rpm.section_changelog = None
        self.inspection = 'changelog'
        self.label = 'changelog'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# NOTE: just comparing RPMs here will return INFO not VERIFY because
# the loss of changelog test only occurs on SRPMs.
class LostChangeLogCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.after_rpm.section_changelog = None
        self.inspection = 'changelog'
        self.label = 'changelog'
        self.result = 'INFO'
        self.waiver_auth = 'Not Waivable'

class LostChangeLogCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.after_rpm.section_changelog = None
        self.inspection = 'changelog'
        self.label = 'changelog'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# 2) Prevent a %changelog in the before build but add one in the after
#    build.  The test should report that as INFO.
class GainedChangeLogCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.before_rpm.section_changelog = None
        self.inspection = 'changelog'
        self.label = 'changelog'
        self.result = 'INFO'
        self.waiver_auth = 'Not Waivable'

class GainedChangeLogCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.section_changelog = None
        self.inspection = 'changelog'
        self.label = 'changelog'
        self.result = 'INFO'
        self.waiver_auth = 'Not Waivable'

class GainedChangeLogCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.section_changelog = None
        self.inspection = 'changelog'
        self.label = 'changelog'
        self.result = 'INFO'
        self.waiver_auth = 'Not Waivable'

# 3) If the first entries in the changelog as the same in the before and
#    after build, the test should report it as BAD.
#class SameChangeLogCompareSRPM(TestCompareSRPM):
#    def setUp(self):
#        TestCompareSRPM.setUp(self)
#
#        # create a simple changelog and duplicate it
#        today = datetime.date.today().strftime("%a %b %d %Y")
#        clog = "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7\n- Upgrade to the latest and greatest"
#
#        # modify the changelogs
#        self.before_rpm.section_changelog = clog
#        self.after_rpm.section_changelog = clog
#
#        self.inspection = 'changelog'
#        self.label = 'changelog'
#        self.result = 'BAD'
#        self.waiver_auth = 'Anyone'

#class SameChangeLogCompareRPMs(TestCompareRPMs):
#    def setUp(self):
#        TestCompareRPMs.setUp(self)
#        self.after_rpm.section_changelog = self.before_rpm.section_changelog
#        self.inspection = 'changelog'
#        self.label = 'changelog'
#        self.result = 'BAD'
#        self.waiver_auth = 'Anyone'

#class SameChangeLogCompareKoji(TestCompareKoji):
#    def setUp(self):
#        TestCompareKoji.setUp(self)
#        self.after_rpm.section_changelog = self.before_rpm.section_changelog
#        self.inspection = 'changelog'
#        self.label = 'changelog'
#        self.result = 'BAD'
#        self.waiver_auth = 'Anyone'

# In the binary RPM:
# 1) Change some lines in the after build's %changelog.  The test should
#    report that as VERIFY.  If the number of add and delete lines in the
#    diff are balanced, this is reported as INFO.  There have to be more
#    delete lines than add lines to get VERIFY.
#class BalancedChangeLogEditCompareKoji(TestCompareKoji):
#    def setUp(self):
#        TestCompareKoji.setUp(self)
#
#        # create a simple prefix that will result in a one line edit script
#        today = datetime.date.today().strftime("%a %b %d %Y")
#        before_prefix = "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7\n- Upgrade to the latest and greatest\n\n"
#        after_prefix = "* %s Packie McPackerson <packie@mcpackerson.io> - 7.47\n- Upgrade to the latest and greatest\n\n"
#        suffix = "* %s Packie McPackerson <packie@mcpackerson.io> - 1.0\n- Initial package"
#
#        # modify the changelogs
#        self.before_rpm.section_changelog = before_prefix + suffix
#        self.after_rpm.section_changelog = after_prefix + suffix
#
#        self.inspection = 'changelog'
#        self.label = 'changelog'
#        self.result = 'INFO'
#        self.waiver_auth = 'Not Waivable'

#class UnbalancedChangeLogEditCompareKoji(TestCompareKoji):
#    def setUp(self):
#        TestCompareKoji.setUp(self)
#
#        # create a simple prefix that will result in a multiline removal
#        today = datetime.date.today().strftime("%a %b %d %Y")
#        before_prefix = "* %s Packie McPackerson <packie@mcpackerson.io> - 47.7\n- Upgrade to the latest and greatest\n\n"
#
#        # modify the changelog
#        self.before_rpm.section_changelog = before_prefix + self.before_rpm.section_changelog
#
#        self.inspection = 'changelog'
#        self.label = 'changelog'
#        self.result = 'VERIFY'
#        self.waiver_auth = 'Anyone'

# 2) Only add a new entry to the %changelog in the after build.  This
#    should report as INFO.



# 3) Add unprofessional language to the after build %changelog and make
#    sure that is reported as BAD.
#
# Make sure the diff generated by librpminspect can be applied by patch.
# Do a normal edit like adding an entry and then changing a date stamp
# on an existing entry.  Use diff(1) manually on those changelogs.  Then
# run rpminspect and capture the diff output in the results.  Compare
# these diffs with diff (making sure to trim the --- and +++ leading lines
# from the manually created diff).  If diff returns anything other than
# zero, fail the test.
