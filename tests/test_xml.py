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

from baseclass import *

valid_xml = """<?xml version='1.0'?>
<!DOCTYPE greeting [
  <!ELEMENT greeting (#PCDATA)>
]>
<greeting>Hello, world!</greeting>
"""

invalid_xml = """<?xml version='1.0'?>
<!DOCTYPE greeting [
  <!ELEMENT greeting EMPTY>
]>
<greeting>Hello world</greeting>
"""

# XML file is well formed in RPM (OK)
class TestXMLWellFormedRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_installed_file('/usr/share/data/valid.xml',
                                    rpmfluff.SourceFile('valid.xml', valid_xml))
        self.inspection = 'xml'
        self.label = 'xml-files'
        self.result = 'OK'

# XML file is well formed in Koji build (OK)
class TestXMLWellFormedKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_installed_file('/usr/share/data/valid.xml',
                                    rpmfluff.SourceFile('valid.xml', valid_xml))
        self.inspection = 'xml'
        self.label = 'xml-files'
        self.result = 'OK'

# XML file is well formed in compare RPMs (OK)
class TestXMLWellFormedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid.xml',
                                           rpmfluff.SourceFile('valid.xml', valid_xml))
        self.after_rpm.add_installed_file('/usr/share/data/valid.xml',
                                          rpmfluff.SourceFile('valid.xml', valid_xml))
        self.inspection = 'xml'
        self.label = 'xml-files'
        self.result = 'OK'

# XML file is well formed in compare Koji builds (OK)
class TestXMLWellFormedCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid.xml',
                                           rpmfluff.SourceFile('valid.xml', valid_xml))
        self.after_rpm.add_installed_file('/usr/share/data/valid.xml',
                                          rpmfluff.SourceFile('valid.xml', valid_xml))
        self.inspection = 'xml'
        self.label = 'xml-files'
        self.result = 'OK'

# XML file is malformed in RPM (VERIFY)
class TestXMLWellFormedRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_installed_file('/usr/share/data/invalid.xml',
                                    rpmfluff.SourceFile('invalid.xml', invalid_xml))
        self.inspection = 'xml'
        self.label = 'xml-files'
        self.result = 'VERIFY'

# XML file is malformed in Koji build (VERIFY)
class TestXMLWellFormedKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_installed_file('/usr/share/data/invalid.xml',
                                    rpmfluff.SourceFile('invalid.xml', invalid_xml))
        self.inspection = 'xml'
        self.label = 'xml-files'
        self.result = 'VERIFY'

# XML file is malformed in compare RPMs (VERIFY)
class TestXMLWellFormedCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/invalid.xml',
                                           rpmfluff.SourceFile('invalid.xml', invalid_xml))
        self.after_rpm.add_installed_file('/usr/share/data/invalid.xml',
                                          rpmfluff.SourceFile('invalid.xml', invalid_xml))
        self.inspection = 'xml'
        self.label = 'xml-files'
        self.result = 'VERIFY'

# XML file is malformed in compare Koji builds (VERIFY)
class TestXMLWellFormedCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/invalid.xml',
                                           rpmfluff.SourceFile('invalid.xml', invalid_xml))
        self.after_rpm.add_installed_file('/usr/share/data/invalid.xml',
                                          rpmfluff.SourceFile('invalid.xml', invalid_xml))
        self.inspection = 'xml'
        self.label = 'xml-files'
        self.result = 'VERIFY'
