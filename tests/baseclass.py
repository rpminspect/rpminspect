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
import shutil
import socket
import subprocess
import tempfile
import unittest
import json
import rpmfluff

# NVRs to use for the fake test packages
BEFORE_NAME = "vaporware"
BEFORE_VER = "0.1"
BEFORE_REL = "1"

AFTER_NAME = BEFORE_NAME
AFTER_VER = "0.2"
AFTER_REL = "2"

# Set this to True to keep rpminspect results (useful to debug the test
# suite but will make a big mess)
KEEP_RESULTS = False

# Exceptions used by the test suite
class MissingRpminspect(Exception):
    pass

class MissingRpminspectConf(Exception):
    pass

# Base test case class that ensures we have 'rpminspect'
# as an executable command.
class RequiresRpminspect(unittest.TestCase):
    def setUp(self):
        # make sure we have the program
        self.rpminspect = os.environ['RPMINSPECT']

        if not os.path.isfile(self.rpminspect) or not os.access(self.rpminspect, os.X_OK):
            raise MissingRpminspect

        # ensure we have the sample configuration file
        if not os.path.isfile(os.environ['RPMINSPECT_CONF']) or not os.access(os.environ['RPMINSPECT_CONF'], os.R_OK):
            raise MissingRpminspectConf

        # set in configFile()
        self.conffile = None

        # settings that the inheriting test can override
        self.buildhost_subdomain = None

    def configFile(self):
        # create a copy of the sample conf file for test purposes
        (handle, self.conffile) = tempfile.mkstemp()
        os.close(handle)

        shutil.copyfile(os.environ['RPMINSPECT_CONF'], self.conffile)

        f = open(os.environ['RPMINSPECT_CONF'], "r+")
        conflines = f.readlines()
        f.close()

        f = open(self.conffile, "w+")

        # modify settings for the test suite based on where it's running
        for line in conflines:
            if line.startswith("licensedb"):
                # point to our test license database
                # we have some known good licenses and known bad licenses
                f.write("licensedb = \"%s/licenses.json\"\n" % os.environ['RPMINSPECT_TEST_DATA_PATH'])
            elif line.startswith("buildhost_subdomain"):
                if self.buildhost_subdomain:
                    f.write("buildhost_subdomain = \"%s\"\n" % self.buildhost_subdomain)
                else:
                    f.write("buildhost_subdomain = \"%s\"\n" % socket.getfqdn())
            else:
                f.write(line)

        f.close()

    def tearDown(self):
        os.unlink(self.conffile)

# Base test case class that tests on the SRPM package only
class TestSRPM(RequiresRpminspect):
    def setUp(self):
        RequiresRpminspect.setUp(self)
        self.rpm = rpmfluff.SimpleRpmBuild(AFTER_NAME, AFTER_VER, AFTER_REL)

        # the inheriting class needs to override these
        self.inspection = None
        self.label = None

        # these default to 0 and OK
        self.exitcode = 0
        self.result = 'OK'

    def configFile(self):
        RequiresRpminspect.configFile(self)

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        self.rpm.do_make()

        args = [self.rpminspect, '-c', self.conffile, '-F', 'json']
        if self.inspection:
            args.append('-T')
            args.append(self.inspection)

        if KEEP_RESULTS:
            args.append('-k')

        args.append(self.rpm.get_built_srpm())

        self.p = subprocess.Popen(args,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()

        try:
            self.results = json.loads(self.out)
        except json.decoder.JSONDecodeError:
            print("\n\ninspection: |%s|\nstdout: |%s|\nstderr: |%s|\n\n" % (self.inspection, self.out, self.err))

        # anything not OK or INFO is a non-zero return
        if self.result not in ['OK', 'INFO'] and self.exitcode == 0:
            self.exitcode = 1

        # dump stdout and stderr if these do not match
        if self.p.returncode != self.exitcode:
            print("\n\nstdout: |%s|\nstderr: |%s|\n\n" % (self.out, self.err))

        self.assertEqual(self.p.returncode, self.exitcode)
        self.assertEqual(self.results[self.label][0]['result'], self.result)

# Base test case class that compares a before and after SRPM
class TestCompareSRPM(RequiresRpminspect):
    def setUp(self):
        RequiresRpminspect.setUp(self)
        self.before_rpm = rpmfluff.SimpleRpmBuild(BEFORE_NAME, BEFORE_VER, BEFORE_REL)
        self.after_rpm = rpmfluff.SimpleRpmBuild(AFTER_NAME, AFTER_VER, AFTER_REL)

        # the inheriting class needs to override these
        self.inspection = None
        self.label = None

        # these default to 0 and OK
        self.exitcode = 0
        self.result = 'OK'

    def configFile(self):
        RequiresRpminspect.configFile(self)

    def runTest(self):
        self.configFile()

        if not self.inspection:
            return

        self.before_rpm.do_make()
        self.after_rpm.do_make()

        args = [self.rpminspect, '-c', self.conffile, '-F', 'json']

        if self.inspection:
            args.append('-T')
            args.append(self.inspection)

        if KEEP_RESULTS:
            args.append('-k')

        args.append(self.before_rpm.get_built_srpm())
        args.append(self.after_rpm.get_built_srpm())

        self.p = subprocess.Popen(args,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
        (self.out, self.err) = self.p.communicate()

        try:
            self.results = json.loads(self.out)
        except json.decoder.JSONDecodeError:
            print("\n\ninspection: |%s|\nstdout: |%s|\nstderr: |%s|\n\n" % (self.inspection, self.out, self.err))

        # anything not OK or INFO is a non-zero return
        if self.result not in ['OK', 'INFO'] and self.exitcode == 0:
            self.exitcode = 1

        # dump stdout and stderr if these do not match
        if self.p.returncode != self.exitcode:
            print("\n\nstdout: |%s|\nstderr: |%s|\n\n" % (self.out, self.err))

        self.assertEqual(self.p.returncode, self.exitcode)
        self.assertEqual(self.results[self.label][0]['result'], self.result)

# Base test case class that tests the binary RPMs
class TestRPMs(TestSRPM):
    def runTest(self):
        TestSRPM.configFile(self)

        if not self.inspection and not self.label:
            return

        self.rpm.do_make()

        # anything not OK or INFO is a non-zero return
        if self.result not in ['OK', 'INFO'] and self.exitcode == 0:
            self.exitcode = 1

        for a in self.rpm.get_build_archs():
            args = [self.rpminspect, '-c', self.conffile, '-F', 'json']

            if self.inspection:
                args.append('-T')
                args.append(self.inspection)

            if KEEP_RESULTS:
                args.append('-k')

            args.append(self.rpm.get_built_rpm(a))

            self.p = subprocess.Popen(args,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
            (self.out, self.err) = self.p.communicate()

            try:
                self.results = json.loads(self.out)
            except json.decoder.JSONDecodeError:
                print("\n\ninspection: |%s|\nstdout: |%s|\nstderr: |%s|\n\n" % (self.inspection, self.out, self.err))

            # dump stdout and stderr if these do not match
            if self.p.returncode != self.exitcode:
                print("\n\nstdout: |%s|\nstderr: |%s|\n\n" % (self.out, self.err))

            self.assertEqual(self.p.returncode, self.exitcode)
            self.assertEqual(self.results[self.label][0]['result'], self.result)

# Base test case class that compares before and after built RPMs
class TestCompareRPMs(TestCompareSRPM):
    def runTest(self):
        TestCompareSRPM.configFile(self)

        if not self.inspection and not self.label:
            return

        self.before_rpm.do_make()
        self.after_rpm.do_make()

        # anything not OK or INFO is a non-zero return
        if self.result not in ['OK', 'INFO'] and self.exitcode == 0:
            self.exitcode = 1

        for a in self.before_rpm.get_build_archs():
            args = [self.rpminspect, '-c', self.conffile, '-F', 'json']

            if self.inspection:
                args.append('-T')
                args.append(self.inspection)

            if KEEP_RESULTS:
                args.append('-k')

            args.append(self.before_rpm.get_built_rpm(a))
            args.append(self.after_rpm.get_built_rpm(a))

            self.p = subprocess.Popen(args,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
            (self.out, self.err) = self.p.communicate()

            try:
                self.results = json.loads(self.out)
            except json.decoder.JSONDecodeError:
                print("\n\ninspection: |%s|\nstdout: |%s|\nstderr: |%s|\n\n" % (self.inspection, self.out, self.err))

            # dump stdout and stderr if these do not match
            if self.p.returncode != self.exitcode:
                print("\n\nstdout: |%s|\nstderr: |%s|\n\n" % (self.out, self.err))

            self.assertEqual(self.p.returncode, self.exitcode)
            self.assertEqual(self.results[self.label][0]['result'], self.result)

# Base test case class that tests a fake Koji build
class TestKoji(TestSRPM):
    def runTest(self):
        TestSRPM.configFile(self)

        if not self.inspection:
            return

        # generate the build
        self.rpm.do_make()

        # copy everything in to place as if Koji built it
        with tempfile.TemporaryDirectory() as kojidir:
            # copy over the SRPM to the fake koji build
            srcdir = kojidir + '/src'
            os.makedirs(srcdir, exist_ok=True)
            shutil.copy(self.rpm.get_built_srpm(), srcdir)

            # copy over the built RPMs to the fake koji build
            for a in self.rpm.get_build_archs():
                adir = kojidir + '/' + a
                os.makedirs(adir, exist_ok=True)
                shutil.copy(self.rpm.get_built_rpm(a), adir)

            args = [self.rpminspect, '-c', self.conffile, '-F', 'json', '-r', AFTER_REL]
            if self.inspection:
                args.append('-T')
                args.append(self.inspection)

            if KEEP_RESULTS:
                args.append('-k')

            args.append(kojidir)

            self.p = subprocess.Popen(args,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
            (self.out, self.err) = self.p.communicate()

            try:
                self.results = json.loads(self.out)
            except json.decoder.JSONDecodeError:
                print("\n\ninspection: |%s|\nstdout: |%s|\nstderr: |%s|\n\n" % (self.inspection, self.out, self.err))

            # anything not OK or INFO is a non-zero return
            if self.result not in ['OK', 'INFO'] and self.exitcode == 0:
                self.exitcode = 1

            # dump stdout and stderr if these do not match
            if self.p.returncode != self.exitcode:
                print("\n\nstdout: |%s|\nstderr: |%s|\n\n" % (self.out, self.err))

            self.assertEqual(self.p.returncode, self.exitcode)
            self.assertEqual(self.results[self.label][0]['result'], self.result)

# Base test case class that compares before and after Koji builds
class TestCompareKoji(TestCompareSRPM):
    def runTest(self):
        TestCompareSRPM.configFile(self)

        if not self.inspection:
            return

        # generate the build
        self.before_rpm.do_make()
        self.after_rpm.do_make()

        # copy everything in to place as if Koji built it
        with tempfile.TemporaryDirectory() as kojidir:
            # copy over the SRPM to the fake koji build
            beforesrcdir = kojidir + '/before/src'
            aftersrcdir = kojidir + '/after/src'
            os.makedirs(beforesrcdir, exist_ok=True)
            os.makedirs(aftersrcdir, exist_ok=True)
            shutil.copy(self.before_rpm.get_built_srpm(), beforesrcdir)
            shutil.copy(self.after_rpm.get_built_srpm(), aftersrcdir)

            # copy over the built RPMs to the fake koji build
            for a in self.before_rpm.get_build_archs():
                adir = kojidir + '/before/' + a
                os.makedirs(adir, exist_ok=True)
                shutil.copy(self.before_rpm.get_built_rpm(a), adir)

            for a in self.after_rpm.get_build_archs():
                adir = kojidir + '/after/' + a
                os.makedirs(adir, exist_ok=True)
                shutil.copy(self.after_rpm.get_built_rpm(a), adir)

            args = [self.rpminspect, '-c', self.conffile, '-F', 'json', '-r', AFTER_REL]

            if self.inspection:
                args.append('-T')
                args.append(self.inspection)

            if KEEP_RESULTS:
                args.append('-k')

            args.append(kojidir + '/before')
            args.append(kojidir + '/after')

            self.p = subprocess.Popen(args,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
            (self.out, self.err) = self.p.communicate()

            try:
                self.results = json.loads(self.out)
            except json.decoder.JSONDecodeError:
                print("\n\ninspection: |%s|\nstdout: |%s|\nstderr: |%s|\n\n" % (self.inspection, self.out, self.err))

            # anything not OK or INFO is a non-zero return
            if self.result not in ['OK', 'INFO'] and self.exitcode == 0:
                self.exitcode = 1

            # dump stdout and stderr if these do not match
            if self.p.returncode != self.exitcode:
                print("\n\nstdout: |%s|\nstderr: |%s|\n\n" % (self.out, self.err))

            self.assertEqual(self.p.returncode, self.exitcode)
            self.assertEqual(self.results[self.label][0]['result'], self.result)
