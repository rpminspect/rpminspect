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
import unittest

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
