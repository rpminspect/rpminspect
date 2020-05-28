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
import subprocess
import unittest
import rpmfluff
from baseclass import TestRPMs, TestKoji, TestCompareRPMs, TestCompareKoji

class RpmException(Exception):
    """For failures running rpm(1)."""
    pass

# more recent versions of RPM prevent dangling and too long symlinks
args = ["rpm", "--version"]
proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
(out, err) = proc.communicate()
if proc.returncode != 0:
    raise RpmException

# b'RPM version 4.15.90' -> (4, 15, 90)
(rpm_major, rpm_minor, rpm_update) = tuple(map(lambda x: int(x), out.decode(encoding='UTF-8').strip().split()[-1].split('.')))

if rpm_major < 4 or (rpm_major == 4 and rpm_minor < 15) or (rpm_major == 4 and rpm_minor == 15 and rpm_update < 90):
    rpm_handles_symlinks = False
else:
    rpm_handles_symlinks = True

# Read in the built rpminspect executable for use in these test RPMs
with open(os.environ['RPMINSPECT'], mode='rb') as f:
    ri_bytes = f.read()

# Absolute symlink exists (OK)
class AbsoluteSymlinkExistsRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/sbin/testscript', '/usr/bin/testscript')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class AbsoluteSymlinkExistsKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/sbin/rpminspect', '/usr/bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class AbsoluteSymlinkExistsCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/sbin/rpminspect', '/usr/bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class AbsoluteSymlinkExistsCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/sbin/rpminspect', '/usr/bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Relative symlink with ../ exists (OK)
class RelativeSymlinkExistsParentDirRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/sbin/rpminspect', '../bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class RelativeSymlinkExistsParentDirKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/sbin/rpminspect', '../bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class RelativeSymlinkExistsParentDirCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/sbin/rpminspect', '../bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class RelativeSymlinkExistsParentDirCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/sbin/rpminspect', '../bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Relative symlink in current directory exists (OK)
class RelativeSymlinkExistsCurrentDirRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class RelativeSymlinkExistsCurrentDirKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class RelativeSymlinkExistsCurrentDirCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class RelativeSymlinkExistsCurrentDirCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Symlink that exists spans subpackages (OK)
class SymlinkExistsMultiplePackagesRPMS(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        subpackage = self.rpm.add_subpackage('symlinks')
        self.rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect', subpackageSuffix='symlinks')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class SymlinkExistsMultiplePackagesKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        subpackage = self.rpm.add_subpackage('symlinks')
        self.rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect', subpackageSuffix='symlinks')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class SymlinkExistsMultiplePackagesCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        subpackage = self.after_rpm.add_subpackage('symlinks')
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect', subpackageSuffix='symlinks')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class SymlinkExistsMultiplePackagesCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        subpackage = self.after_rpm.add_subpackage('symlinks')
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect', subpackageSuffix='symlinks')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Absolute symlink is dangling (VERIFY)
class AbsoluteSymlinkDangingRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/sbin/rpminspect', '/usr/bin/anotherrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class AbsoluteSymlinkDangingKoji(TestKoji):
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/sbin/rpminspect', '/usr/bin/anotherrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class AbsoluteSymlinkDangingCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/sbin/rpminspect', '/usr/bin/anotherrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class AbsoluteSymlinkDangingCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/sbin/rpminspect', '/usr/bin/anotherrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Relative symlink with ../ is dangling (VERIFY)
class RelativeSymlinkDanglingParentDirRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/sbin/rpminspect', '../bin/anotherrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class RelativeSymlinkDanglingParentDirKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/sbin/rpminspect', '../bin/anotherrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class RelativeSymlinkDanglingParentDirRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/sbin/rpminspect', '../bin/anotherrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class RelativeSymlinkDanglingParentDirKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/sbin/rpminspect', '../bin/anotherrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Relative symlink in current directory is dangling (VERIFY)
class RelativeSymlinkDanglingCurrentDirRPMs(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'originalrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class RelativeSymlinkDanglingCurrentDirKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'originalrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class RelativeSymlinkDanglingCurrentDirCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'originalrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class RelativeSymlinkDanglingCurrentDirCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'originalrpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Too many symlink cycles (VERIFY)
# To trigger an ELOOP on symlink resolution, you need to have more
# than 40 levels of symlink redirection, per path_resolution(7).  I use 47
# levels here just to make sure.
class TooManySymlinkLevelsRPMs(TestRPMs):
    @unittest.skipIf(rpm_handles_symlinks, "rpm %d.%d.%d detected, prevents ELOOP symlink errors" % (rpm_major, rpm_minor, rpm_update))
    def setUp(self):
        TestRPMs.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/bin/bin', '.')
        self.rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

class TooManySymlinkLevelsKoji(TestKoji):
    @unittest.skipIf(rpm_handles_symlinks, "rpm %d.%d.%d detected, prevents ELOOP symlink errors" % (rpm_major, rpm_minor, rpm_update))
    def setUp(self):
        TestKoji.setUp(self)

        # add file and symlink
        self.rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                    sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                    mode="0755")
        self.rpm.add_installed_symlink('usr/bin/bin', '.')
        self.rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

class TooManySymlinkLevelsCompareRPMs(TestCompareRPMs):
    @unittest.skipIf(rpm_handles_symlinks, "rpm %d.%d.%d detected, prevents ELOOP symlink errors" % (rpm_major, rpm_minor, rpm_update))
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/bin/bin', '.')
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/rpminspect')

        # disable check-buildroot rpmbuild script to avoid stat errors
        # on our big symlink
        self.after_rpm.header += "\n%global __arch_install_post %{nil}\n"

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

class TooManySymlinkLevelsCompareKoji(TestCompareKoji):
    @unittest.skipIf(rpm_handles_symlinks, "rpm %d.%d.%d detected, prevents ELOOP symlink errors" % (rpm_major, rpm_minor, rpm_update))
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file and symlink
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                          sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                          mode="0755")
        self.after_rpm.add_installed_symlink('usr/bin/bin', '.')
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/bin/rpminspect')

        # disable check-buildroot rpmbuild script to avoid stat errors
        # on our big symlink
        self.after_rpm.header += "\n%global __arch_install_post %{nil}\n"

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# Directory becomes a symlink (VERIFY)
class DirectoryBecomesSymlinkCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add directories and symlinks
        self.before_rpm.add_installed_directory('usr/share/testdirectory')
        self.before_rpm.add_installed_directory('usr/share/actualdirectory')
        self.after_rpm.add_installed_symlink('usr/share/testdirectory', 'actualdirectory')
        self.after_rpm.add_installed_directory('usr/share/actualdirectory')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class DirectoryBecomesSymlinkCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add directories and symlinks
        self.before_rpm.add_installed_directory('usr/share/testdirectory')
        self.before_rpm.add_installed_directory('usr/share/actualdirectory')
        self.after_rpm.add_installed_symlink('usr/share/testdirectory', 'actualdirectory')
        self.after_rpm.add_installed_directory('usr/share/actualdirectory')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Non-directory becomes a symlink (VERIFY)
class NonDirectoryBecomesSymlinkCompareRPMs(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add files and symlinks
        self.before_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")
        self.before_rpm.add_installed_file(installPath='usr/bin/anotherrpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

class NonDirectoryBecomesSymlinkCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add files and symlinks
        self.before_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")
        self.before_rpm.add_installed_file(installPath='usr/bin/anotherrpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")
        self.after_rpm.add_installed_file(installPath='usr/bin/rpminspect',
                                           sourceFile=rpmfluff.SourceFile('rpminspect', ri_bytes),
                                           mode="0755")
        self.after_rpm.add_installed_symlink('usr/bin/anotherrpminspect', 'rpminspect')

        self.inspection = 'symlinks'
        self.label = 'symlinks'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'
