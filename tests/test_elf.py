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
from baseclass import *

# Source code used for the -D_FORTIFY_SOURCE tests
fortify_src = open(os.environ['RPMINSPECT_TEST_DATA_PATH'] + '/fortify.c').read()

# Program built with noexecstack
class TestWithoutExecStackRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_simple_compilation(compileFlags='-Wl,-z,noexecstack')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Anyone'
        self.result = 'OK'

class TestWithoutEXecStackKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_simple_compilation(compileFlags='-Wl,-z,noexecstack')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Anyone'
        self.result = 'OK'

class TestWithoutExecStackCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags='-Wl,-z,noexecstack')
        self.after_rpm.add_simple_compilation(compileFlags='-Wl,-z,noexecstack')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Anyone'
        self.result = 'OK'

class TestWithoutExecStackCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags='-Wl,-z,noexecstack')
        self.after_rpm.add_simple_compilation(compileFlags='-Wl,-z,noexecstack')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Anyone'
        self.result = 'OK'

# Program built with execstack
class TestWithExecStackRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_simple_compilation(compileFlags='-Wl,-z,execstack')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'BAD'

class TestWithExecStackKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_simple_compilation(compileFlags='-Wl,-z,execstack')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'BAD'

class TestWithExecStackCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags='-Wl,-z,execstack')
        self.after_rpm.add_simple_compilation(compileFlags='-Wl,-z,execstack')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'VERIFY'

class TestWithExecStackCompare(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags='-Wl,-z,execstack')
        self.after_rpm.add_simple_compilation(compileFlags='-Wl,-z,execstack')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'VERIFY'

# Program lost full RELRO
class TestLostFullRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags='-Wl,-z,relro,-z,now')
        self.after_rpm.add_simple_compilation(compileFlags='-Wl,-z,norelro')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'BAD'

class TestLostFullRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags='-Wl,-z,relro,-z,now')
        self.after_rpm.add_simple_compilation(compileFlags='-Wl,-z,norelro')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'BAD'

# Program lost full RELRO but retained partial RELRO
class TestFulltoPartialRELROCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags='-Wl,-z,relro,-z,now')
        self.after_rpm.add_simple_compilation(compileFlags='-Wl,-z,relro')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'BAD'

class TestFulltoPartialRELROCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_simple_compilation(compileFlags='-Wl,-z,relro,-z,now')
        self.after_rpm.add_simple_compilation(compileFlags='-Wl,-z,relro')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'BAD'

# Program lost -D_FORTIFY_SOURCE
class TestLostFortifySourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Enabling FORTIFY_SOURCE requires -O1 or higher
        self.before_rpm.add_simple_compilation(sourceContent=fortify_src,
                                               compileFlags='-O2 -D_FORTIFY_SOURCE')
        self.after_rpm.add_simple_compilation(sourceContent=fortify_src,
                                              compileFlags='-O2')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'VERIFY'

class TestLostFortifySourceCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Enabling FORTIFY_SOURCE requires -O1 or higher
        self.before_rpm.add_simple_compilation(sourceContent=fortify_src,
                                               compileFlags='-O2 -D_FORTIFY_SOURCE')
        self.after_rpm.add_simple_compilation(sourceContent=fortify_src,
                                              compileFlags='-O2')
        self.inspection = 'elf'
        self.label = 'elf-object-properties'
        self.waiver_auth = 'Security'
        self.result = 'VERIFY'

# XXX: Program uses forbidden IPv6 function (VERIFY)

# XXX: Program lost -fPIC in after (BAD, WAIVABLE_BY_SECURITY)

# XXX: Program has or gained TEXTREL relocations
