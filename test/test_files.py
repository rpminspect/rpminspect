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

import rpmfluff
from baseclass import TestSRPM, TestKoji, TestCompareSRPM, TestCompareKoji


class ValidFilesSectionSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)

        # add a library and proper %files reference
        libraryName = "libfoo.so"
        sourceFileName = "foo.c"
        _sourceId = self.rpm.add_source(
            rpmfluff.SourceFile(sourceFileName, rpmfluff.simple_library_source)
        )
        self.rpm.section_build += "gcc --shared -fPIC -o %s %s %s\n" % (
            libraryName,
            "",
            sourceFileName,
        )
        self.rpm.section_install += (
            "install -D -m 0755 %s %%{buildroot}%%{_libdir}/%s\n"
            % (libraryName, libraryName)
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%{_libdir}/%s\n" % libraryName

        # the inspection results expected
        self.inspection = "%files"
        self.label = "%files"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class ValidFilesSectionKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # add a library and proper %files reference
        libraryName = "libfoo.so"
        sourceFileName = "foo.c"
        _sourceId = self.rpm.add_source(
            rpmfluff.SourceFile(sourceFileName, rpmfluff.simple_library_source)
        )
        self.rpm.section_build += "gcc --shared -fPIC -o %s %s %s\n" % (
            libraryName,
            "",
            sourceFileName,
        )
        self.rpm.section_install += (
            "install -D -m 0755 %s %%{buildroot}%%{_libdir}/%s\n"
            % (libraryName, libraryName)
        )
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%{_libdir}/%s\n" % libraryName

        # the inspection results expected
        self.inspection = "%files"
        self.label = "%files"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class ValidFilesSectionCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)

        # add a library and proper %files reference
        libraryName = "libfoo.so"
        sourceFileName = "foo.c"
        _sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile(sourceFileName, rpmfluff.simple_library_source)
        )
        self.after_rpm.section_build += "gcc --shared -fPIC -o %s %s %s\n" % (
            libraryName,
            "",
            sourceFileName,
        )
        self.after_rpm.section_install += (
            "install -D -m 0755 %s %%{buildroot}%%{_libdir}/%s\n"
            % (libraryName, libraryName)
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%{_libdir}/%s\n" % libraryName

        # the inspection results expected
        self.inspection = "%files"
        self.label = "%files"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class ValidFilesSectionCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add a library and proper %files reference
        libraryName = "libfoo.so"
        sourceFileName = "foo.c"
        _sourceId = self.after_rpm.add_source(
            rpmfluff.SourceFile(sourceFileName, rpmfluff.simple_library_source)
        )
        self.after_rpm.section_build += "gcc --shared -fPIC -o %s %s %s\n" % (
            libraryName,
            "",
            sourceFileName,
        )
        self.after_rpm.section_install += (
            "install -D -m 0755 %s %%{buildroot}%%{_libdir}/%s\n"
            % (libraryName, libraryName)
        )
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%{_libdir}/%s\n" % libraryName

        # the inspection results expected
        self.inspection = "%files"
        self.label = "%files"
        self.result = "OK"
        self.waiver_auth = "Not Waivable"


class InvalidFilesSectionSRPM(TestSRPM):
    def setUp(self):
        TestSRPM.setUp(self)
        self.rpm.add_simple_library()

        self.inspection = "%files"
        self.label = "%files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidFilesSectionKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_simple_library()

        self.inspection = "%files"
        self.label = "%files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidFilesSectionCompareSRPM(TestCompareSRPM):
    def setUp(self):
        TestCompareSRPM.setUp(self)
        self.after_rpm.add_simple_library()

        self.inspection = "%files"
        self.label = "%files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class InvalidFilesSectionCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.after_rpm.add_simple_library()

        self.inspection = "%files"
        self.label = "%files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
