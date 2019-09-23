#
# Copyright (C) 2019  Red Hat, Inc.
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
import subprocess
import unittest
import json
import rpmfluff

# Exceptions used by the test suite
class MissingRpminspect(Exception):
    pass

class MissingRpminspectConf(Exception):
    pass

# Base test case class that ensures we have 'rpminspect'
# as an executable command.
class RequiresRpminspect(unittest.TestCase):
    def setUp(self):
        self.rpminspect = os.environ['RPMINSPECT']
        self.conffile = os.environ['RPMINSPECTCONF']

        if not os.path.isfile(self.rpminspect) or not os.access(self.rpminspect, os.X_OK):
            raise MissingRpminspect

        if not os.path.isfile(self.conffile) or not os.access(self.conffile, os.R_OK):
            raise MissingRpminspectConf

# Base test case class that tests on the SRPM package only
class TestSRPM(RequiresRpminspect):
    def setUp(self):
        RequiresRpminspect.setUp(self)
        self.rpm = rpmfluff.SimpleRpmBuild("example", "0.1", "1")

        # the inheriting class needs to override these
        self.inspection = None
        self.label = None

        # these default to 0 and OK
        self.exitcode = 0
        self.result = 'OK'

    def runTest(self):
        if not self.inspection:
            return

        self.rpm.make()
        self.p = subprocess.Popen([self.rpminspect,
                                   '-c', self.conffile,
                                   '-F', 'json',
                                   '-T', self.inspection,
                                   self.rpm.get_built_srpm()],
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()
        self.results = json.loads(self.out)

        # anything not OK or INFO is a non-zero return
        if self.result not in ['OK', 'INFO'] and self.exitcode == 0:
            self.exitcode = 1

        self.assertEqual(self.p.returncode, self.exitcode)
        self.assertEqual(self.results[self.label][0]['result'], self.result)

# Base test case class that tests the binary RPMs
class TestRPMs(RequiresRpminspect):
    def setUp(self):
        RequiresRpminspect.setUp(self)
        self.rpm = rpmfluff.SimpleRpmBuild("example", "0.1", "1")

        # the inheriting class needs to override these
        self.inspection = None
        self.label = None

        # these default to 0 and OK
        self.exitcode = 0
        self.result = 'OK'

    def runTest(self):
        if not self.inspection and not self.label:
            return

        self.rpm.make()

        # anything not OK or INFO is a non-zero return
        if self.result not in ['OK', 'INFO'] and self.exitcode == 0:
            self.exitcode = 1

        for a in self.rpm.get_build_archs():
            self.p = subprocess.Popen([self.rpminspect,
                                       '-c', self.conffile,
                                       '-F', 'json',
                                       '-T', self.inspection,
                                       self.rpm.get_built_rpm(a)],
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
            (self.out, self.err) = self.p.communicate()
            self.results = json.loads(self.out)

            self.assertEqual(self.p.returncode, self.exitcode)
            self.assertEqual(self.results[self.label][0]['result'], self.result)
