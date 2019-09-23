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

import subprocess
import unittest
import json
import rpmfluff
from baseclass import RequiresRpminspect

# Verify missing %{?dist} in Release fails (BAD)
class TestMissingDistTag(RequiresRpminspect):
    def setUp(self):
        RequiresRpminspect.setUp(self)
        self.rpm = rpmfluff.SimpleRpmBuild("example", "0.1", "1")
        self.rpm.make()

    def runTest(self):
        p = subprocess.Popen([self.rpminspect, '-c', self.conffile, '-F', 'json', '-T', 'disttag', self.rpm.get_built_srpm()], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (out, err) = p.communicate()
        results = json.loads(out)
        self.assertEqual(p.returncode, 1)
        self.assertEqual(results['dist-tag'][0]['result'], 'BAD')

# Verify running on not an SRPM fails
# Verify malformed %{?dist} in Release needs inspection (VERIFY)
# Verify missing Release tag fails (BAD)
# Verify correct %{?dist} usage passes (OK)
