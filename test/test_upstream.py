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
from baseclass import *

# Read in the built rpminspect executable for use in these test RPMs
with open(os.environ['RPMINSPECT'], mode='rb') as f:
    ri_bytes = f.read()

###################################
# Single SRPM, RPM, or Koji build #
###################################

class SkipUpstreamSRPM(TestSRPM):
    """
    Test that single source RPM runs skip the upstream inspection.
    """
    def setUp(self):
        TestSRPM.setUp(self)

        # tell it to pass '-T upstream' to rpminspect
        self.inspection = 'upstream'

        # since it should skip, only look for our diagnostic output
        self.label = 'rpminspect'
        self.result = 'INFO'

    def runTest(self):
        TestSRPM.runTest(self)

        # make sure 'upstream' is not in the results
        self.assertFalse(self.inspection in self.results.keys())

class SkipUpstreamRPMs(TestRPMs):
    """
    Test that single binary RPM runs skip the upstream inspection.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # tell it to pass '-T upstream' to rpminspect
        self.inspection = 'upstream'

        # since it should skip, only look for our diagnostic output
        self.label = 'rpminspect'
        self.result = 'INFO'

    def runTest(self):
        TestRPMs.runTest(self)

        # make sure 'upstream' is not in the results
        self.assertFalse(self.inspection in self.results.keys())

class SkipUpstreamKoji(TestKoji):
    """
    Test that single Koji build runs skip the upstream inspection.
    """
    def setUp(self):
        TestKoji.setUp(self)

        # tell it to pass '-T upstream' to rpminspect
        self.inspection = 'upstream'

        # since it should skip, only look for our diagnostic output
        self.label = 'rpminspect'
        self.result = 'INFO'

    def runTest(self):
        TestKoji.runTest(self)

        # make sure 'upstream' is not in the results
        self.assertFalse(self.inspection in self.results.keys())

##########################
# Source RPM comparisons #
##########################

class DiffVerUpstreamCompareSRPM(TestCompareSRPM):
    """
    Test that comparing two SRPMs with different versions gives an OK
    result in the upstream inspection.
    """
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.inspection = 'upstream'
        self.label = 'upstream'
        self.result = 'OK'

class SameVerChangeUpstreamCompareSRPM(TestCompareSRPM):
    """
    Test that comparing two SRPMs with the same version but changing
    Source files in the package gives a VERIFY result in the upstream
    inspection.
    """
    def setUp(self):
        TestCompareSRPM.setUp(self)

        # recreate the after RPM with the same version as the before
        self.after_rpm = rpmfluff.SimpleRpmBuild(BEFORE_NAME, BEFORE_VER, AFTER_REL)

        # add the same installed target file with different sources
        self.before_rpm.add_installed_file(installPath='bin/rpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")
        self.after_rpm.add_simple_compilation(installPath="bin/rpminspect")
        self.after_rpm.section_install += "chmod 0755 $RPM_BUILD_ROOT/bin/rpminspect\n"

        # what we are checking
        self.inspection = 'upstream'
        self.label = 'upstream'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class SameVerRemoveUpstreamCompareSRPM(TestCompareSRPM):
    """
    Test that comparing two SRPMs with the same version but removing
    a Source file in the package gives a VERIFY result in the upstream
    inspection.
    """
    def setUp(self):
        TestCompareSRPM.setUp(self)

        # recreate the after RPM with the same version as the before
        self.after_rpm = rpmfluff.SimpleRpmBuild(BEFORE_NAME, BEFORE_VER, AFTER_REL)

        # simulate a removed source file by not including it in the after rpm
        self.before_rpm.add_installed_file(installPath='bin/rpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")

        # what we are checking
        self.inspection = 'upstream'
        self.label = 'upstream'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

##########################
# Koji build comparisons #
##########################

class DiffVerUpstreamCompareKoji(TestCompareKoji):
    """
    Test that comparing two Koji builds with different versions gives
    an OK result in the upstream inspection.

    """
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.inspection = 'upstream'
        self.label = 'upstream'
        self.result = 'OK'

class SameVerChangeUpstreamCompareKoji(TestCompareKoji):
    """
    Test that comparing two Koji builds with the same version but changing
    Source files in the package gives a VERIFY result in the upstream
    inspection.
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # recreate the after RPM with the same version as the before
        self.after_rpm = rpmfluff.SimpleRpmBuild(BEFORE_NAME, BEFORE_VER, AFTER_REL)

        # add the same installed target file with different sources
        self.before_rpm.add_installed_file(installPath='bin/rpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")
        self.after_rpm.add_simple_compilation(installPath="bin/rpminspect")
        self.after_rpm.section_install += "chmod 0755 $RPM_BUILD_ROOT/bin/rpminspect\n"

        # what we are checking
        self.inspection = 'upstream'
        self.label = 'upstream'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class SameVerRemoveUpstreamCompareKoji(TestCompareKoji):
    """
    Test that comparing two Koji builds with the same version but removing
    a Source file in the package gives a VERIFY result in the upstream
    inspection.
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # recreate the after RPM with the same version as the before
        self.after_rpm = rpmfluff.SimpleRpmBuild(BEFORE_NAME, BEFORE_VER, AFTER_REL)

        # simulate a removed source file by not including it in the after rpm
        self.before_rpm.add_installed_file(installPath='bin/rpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")

        # what we are checking
        self.inspection = 'upstream'
        self.label = 'upstream'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'
