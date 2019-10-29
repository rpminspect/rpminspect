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

# Man page in the correct section subdirectory in RPM (OK)
class TestManPageCorrectSectionRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_manpage()
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'OK'

# Man page in the correct section subdirectory in Koji build (OK)
class TestManPageCorrectSectionKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_manpage()
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'OK'

# Man page in the correct section subdirectory in compare RPMs (OK)
class TestManPageCorrectSectionCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_manpage()
        self.after_rpm.add_manpage()
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'OK'

# Man page in the correct section subdirectory in compare Koji (OK)
class TestManPageCorrectSectionCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_manpage()
        self.after_rpm.add_manpage()
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'OK'

# Man page not gzipped in RPM (VERIFY)
class TestManPageNotGzippedRPM(TestRPMs):
    # Don't use add_manpage() here so we can disable automatic compression
    # of man pages and construct the correct %files section.
    def setUp(self):
        TestRPMs.setUp(self)

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add an uncompressed man page
        sourceIndex = self.rpm.add_source(rpmfluff.SourceFile('foo.1', rpmfluff.sample_man_page))
        installPath = 'usr/share/man/man1/foo.1'
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += 'cp %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n' % (sourceIndex, self.rpm.escape_path(installPath))
        sub = self.rpm.get_subpackage(None)
        sub.section_files += '/%s\n' % installPath
        self.rpm.add_payload_check(installPath, None)

        # the test
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Man page not gzipped in Koji build (VERIFY)
class TestManPageNotGzippedKoji(TestKoji):
    # Don't use add_manpage() here so we can disable automatic compression
    # of man pages and construct the correct %files section.
    def setUp(self):
        TestKoji.setUp(self)

        # disable automatic man page compression in rpmbuild
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add an uncompressed man page
        sourceIndex = self.rpm.add_source(rpmfluff.SourceFile('foo.1', rpmfluff.sample_man_page))
        installPath = 'usr/share/man/man1/foo.1'
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += 'cp %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n' % (sourceIndex, self.rpm.escape_path(installPath))
        sub = self.rpm.get_subpackage(None)
        sub.section_files += '/%s\n' % installPath
        self.rpm.add_payload_check(installPath, None)

        # the test
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Man page not gzipped in compare RPMs (VERIFY)
class TestManPageNotGzippedCompareRPMs(TestCompareRPMs):
    # Don't use add_manpage() here so we can disable automatic compression
    # of man pages and construct the correct %files section.
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # disable automatic man page compression in rpmbuild
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add an uncompressed man page
        beforeIndex = self.before_rpm.add_source(rpmfluff.SourceFile('foo.1', rpmfluff.sample_man_page))
        afterIndex = self.after_rpm.add_source(rpmfluff.SourceFile('foo.1', rpmfluff.sample_man_page))
        installPath = 'usr/share/man/man1/foo.1'
        self.before_rpm.create_parent_dirs(installPath)
        self.after_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += 'cp %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n' % (beforeIndex, self.before_rpm.escape_path(installPath))
        self.after_rpm.section_install += 'cp %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n' % (afterIndex, self.after_rpm.escape_path(installPath))
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += '/%s\n' % installPath
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += '/%s\n' % installPath
        self.before_rpm.add_payload_check(installPath, None)
        self.after_rpm.add_payload_check(installPath, None)

        # the test
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Man page not gzipped in compare Koji (VERIFY)
class TestManPageNotGzippedCompareKoji(TestCompareKoji):
    # Don't use add_manpage() here so we can disable automatic compression
    # of man pages and construct the correct %files section.
    def setUp(self):
        TestCompareKoji.setUp(self)

        # disable automatic man page compression in rpmbuild
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add an uncompressed man page
        beforeIndex = self.before_rpm.add_source(rpmfluff.SourceFile('foo.1', rpmfluff.sample_man_page))
        afterIndex = self.after_rpm.add_source(rpmfluff.SourceFile('foo.1', rpmfluff.sample_man_page))
        installPath = 'usr/share/man/man1/foo.1'
        self.before_rpm.create_parent_dirs(installPath)
        self.after_rpm.create_parent_dirs(installPath)
        self.before_rpm.section_install += 'cp %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n' % (beforeIndex, self.before_rpm.escape_path(installPath))
        self.after_rpm.section_install += 'cp %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n' % (afterIndex, self.after_rpm.escape_path(installPath))
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += '/%s\n' % installPath
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += '/%s\n' % installPath
        self.before_rpm.add_payload_check(installPath, None)
        self.after_rpm.add_payload_check(installPath, None)

        # the test
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Man page in wrong section subdirectory in RPM (VERIFY)
class TestManPageWrongSectionRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)
        self.rpm.add_manpage(sourceFileName='foo.8', installPath='usr/share/man/man1/foo.8')
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Man page in wrong section subdirectory in Koji build (VERIFY)
class TestManPageWrongSectionKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)
        self.rpm.add_manpage(sourceFileName='foo.8', installPath='usr/share/man/man1/foo.8')
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Man page in wrong section subdirectory in compare RPMs (VERIFY)
class TestManPageWrongSectionCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_manpage(sourceFileName='foo.8', installPath='usr/share/man/man1/foo.8')
        self.after_rpm.add_manpage(sourceFileName='foo.8', installPath='usr/share/man/man1/foo.8')
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Man page in wrong section subdirectory in compare Koji (VERIFY)
class TestManPageWrongSectionCompareRPMs(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_manpage(sourceFileName='foo.8', installPath='usr/share/man/man1/foo.8')
        self.after_rpm.add_manpage(sourceFileName='foo.8', installPath='usr/share/man/man1/foo.8')
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Invalid man page syntax in RPM (VERIFY)
class TestInvalidManPageRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # disable automatic compressing since we are simulating that
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add a bad man page
        self.rpm.add_installed_file('/usr/share/man/man1/foo.1', rpmfluff.GeneratedSourceFile('foo.1', rpmfluff.make_png()))

        # the test
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Invalid man page syntax in Koji build (VERIFY)
class TestInvalidManPageKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # disable automatic compressing since we are simulating that
        self.rpm.header += "%global __brp_compress /bin/true\n"

        # add a bad man page
        self.rpm.add_installed_file('/usr/share/man/man1/foo.1', rpmfluff.GeneratedSourceFile('foo.1', rpmfluff.make_png()))

        # the test
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Invalid man page syntax in compare RPMs (VERIFY)
class TestInvalidManPageCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # disable automatic compressing since we are simulating that
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add a bad man page
        self.before_rpm.add_installed_file('/usr/share/man/man1/foo.1', rpmfluff.GeneratedSourceFile('foo.1', rpmfluff.make_png()))
        self.after_rpm.add_installed_file('/usr/share/man/man1/foo.1', rpmfluff.GeneratedSourceFile('foo.1', rpmfluff.make_png()))

        # the test
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Invalid man page syntax in compare Koji (VERIFY)
class TestInvalidManPageCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # disable automatic compressing since we are simulating that
        self.before_rpm.header += "%global __brp_compress /bin/true\n"
        self.after_rpm.header += "%global __brp_compress /bin/true\n"

        # add a bad man page
        self.before_rpm.add_installed_file('/usr/share/man/man1/foo.1', rpmfluff.GeneratedSourceFile('foo.1', rpmfluff.make_png()))
        self.after_rpm.add_installed_file('/usr/share/man/man1/foo.1', rpmfluff.GeneratedSourceFile('foo.1', rpmfluff.make_png()))

        # the test
        self.inspection = 'manpage'
        self.label = 'man-pages'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'
