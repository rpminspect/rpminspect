#
# Copyright (C) 2019-2020  Red Hat, Inc.
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

import subprocess
import unittest
from baseclass import RequiresRpminspect

# Verify --help gives help output
class RpminspectHelp(RequiresRpminspect):
    def runTest(self):
        RequiresRpminspect.configFile(self)
        p = subprocess.Popen([self.rpminspect, '--help'], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        (out, err) = p.communicate()
        self.assertEqual(p.returncode, 0)

        out = out.splitlines()
        t1 = list(filter(lambda x: x.startswith(b'Usage:'), out))
        t2 = list(filter(lambda x: x.startswith(b'Options:'), out))

        self.assertEqual(len(t1), 1)
        self.assertEqual(len(t2), 1)

# Verify rpminspect doesn't segfault on release-less args
class RpminspectSegv(RequiresRpminspect):
    def runTest(self):
        RequiresRpminspect.configFile(self)
        p = subprocess.Popen([self.rpminspect, '42'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        p.communicate()
        self.assertNotEqual(p.returncode, 139)
