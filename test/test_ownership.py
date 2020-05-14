#
# Copyright (C) 2020  Red Hat, Inc.
# Author(s):  David Cantrell <dcantrell@redhat.com>
#                   Jim Bair <jbair@redhat.com>
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

from baseclass import TestRPMs, TestKoji
from baseclass import TestCompareRPMs, TestCompareKoji

script_source = '''#!/bin/sh
echo This is a script
'''

#############
# bin owner #
#############

# File in /bin owned by 'root' is OK in RPMs
class SlashBinOwnedByRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by root in the /bin directory passes.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by 'root' is OK in RPMs
class SlashSbinOwnedByRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by root in the /sbin directory passes.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by 'root' is OK in RPMs
class SlashUsrSlashBinOwnedByRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by root in the /usr/bin directory passes.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by 'root' is OK in RPMs
class SlashUsrSlashSbinOwnedByRootRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by root in the /usr/sbin directory passes.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by 'bin' is BAD in RPMs
class SlashBinOwnedByBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by bin in the /bin directory fails.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /sbin owned by 'bin' is BAD in RPMs
class SlashSbinOwnedByBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by bin in the /sbin directory fails.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/bin owned by 'bin' is BAD in RPMs
class SlashUsrSlashBinOwnedByBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by bin in the /usr/bin directory fails.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/sbin owned by 'bin' is BAD in RPMs
class SlashUsrSlashSbinOwnedByBinRPMs(TestRPMs):
    """
    For binary RPM files, check that a file owned by bin in the /usr/sbin directory fails.
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /bin owned by 'root' is OK in Koji build
class SlashBinOwnedByRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by root in the /bin directory passes.
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by 'root' is OK in Koji build
class SlashSbinOwnedByRootKoji(TestKoji):
    """
    For RPMs in Koji builds, check that a file owned by root in the /bin directory passes.
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by 'root' is OK in Koji build
class SlashUsrSlashBinOwnedByRootKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by 'root' is OK in Koji build
class SlashUsrSlashSbinOwnedByRootKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by 'bin' is BAD in Koji build
class SlashBinOwnedByBinKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by bin
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /sbin owned by 'bin' is BAD in Koji build
class SlashSbinOwnedByBinKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by bin
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/bin owned by 'bin' is BAD in Koji build
class SlashUsrSlashBinOwnedByBinKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by bin
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/sbin owned by 'bin' is BAD in Koji build
class SlashUsrSlashSbinOwnedByBinKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by bin
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /bin owned by 'root' is OK when comparing RPMs
class SlashBinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.before_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by 'root' is OK when comparing RPMs
class SlashSbinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.before_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by 'root' is OK when comparing RPMs
class SlashUsrSlashBinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.before_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by 'root' is OK when comparing RPMs
class SlashUsrSlashSbinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.before_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by 'bin' is BAD when comparing RPMs
class SlashBinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /sbin owned by 'bin' is BAD when comparing RPMs
class SlashSbinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/bin owned by 'bin' is BAD when comparing RPMs
class SlashUsrSlashBinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/sbin owned by 'bin' is BAD when comparing RPMs
class SlashUsrSlashSbinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /bin owned by 'root' is OK when comparing Koji builds
class SlashBinOwnedByRootCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by 'root' is OK when comparing Koji builds
class SlashSbinOwnedByRootCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by 'root' is OK when comparing Koji builds
class SlashUsrSlashBinOwnedByRootCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by 'root' is OK when comparing Koji builds
class SlashUsrSlashSbinOwnedByRootCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by 'bin' is BAD when comparing Koji builds
class SlashBinOwnedByBinCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /sbin owned by 'bin' is BAD when comparing Koji builds
class SlashSbinOwnedByBinCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/bin owned by 'bin' is BAD when comparing Koji builds
class SlashUsrSlashBinOwnedByBinCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/sbin owned by 'bin' is BAD when comparing Koji builds
class SlashUsrSlashSbinOwnedByBinCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

#############
# bin group #
#############

# File in /bin owned by group 'root' is OK in RPMs
class SlashBinOwnedByGroupRootRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by group 'root' is OK in RPMs
class SlashSbinOwnedByGroupRootRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by group 'root' is OK in RPMs
class SlashUsrSlashBinOwnedByGroupRootRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by group 'root' is OK in RPMs
class SlashUsrSlashSbinOwnedByGroupRootRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by group 'bin' is BAD in RPMs
class SlashBinOwnedByGroupBinRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /sbin owned by group 'bin' is BAD in RPMs
class SlashSbinOwnedByGroupBinRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/bin owned by group 'bin' is BAD in RPMs
class SlashiUsrSlashBinOwnedByGroupBinRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/sbin owned by group 'bin' is BAD in RPMs
class SlashUsrSlashSbinOwnedByGroupBinRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /bin owned by group 'root' is OK in Koji build
class SlashBinOwnedByGroupRootKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by group 'root' is OK in Koji build
class SlashSbinOwnedByGroupRootKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by group 'root' is OK in Koji build
class SlashUsrSlashBinOwnedByGroupRootKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by group 'root' is OK in Koji build
class SlashUsrSlashSbinOwnedByGroupRootKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by group 'bin' is BAD in Koji build
class SlashBinOwnedByGroupBinKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /sbin owned by group 'bin' is BAD in Koji build
class SlashSbinOwnedByGroupBinKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/bin owned by group 'bin' is BAD in Koji build
class SlashUsrSlashBinOwnedByGroupBinKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/sbin owned by group 'bin' is BAD in Koji build
class SlashUsrSlashSbinOwnedByGroupBinKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /bin owned by group 'root' is OK when comparing RPMs
class SlashBinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.before_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by group 'root' is OK when comparing RPMs
class SlashSbinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.before_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by group 'root' is OK when comparing RPMs
class SlashUsrSlashBinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.before_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by group 'root' is OK when comparing RPMs
class SlashUsrSlashSbinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.before_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by group 'bin' is BAD when comparing RPMs
class SlashBinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /sbin owned by group 'bin' is BAD when comparing RPMs
class SlashSbinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/bin owned by group 'bin' is BAD when comparing RPMs
class SlashUsrSlashBinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/sbin owned by group 'bin' is BAD when comparing RPMs
class SlashUsrSlashSbinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /bin owned by group 'root' is OK when comparing Koji builds
class SlashBinOwnedByGroupRootCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by group 'root' is OK when comparing Koji builds
class SlashSbinOwnedByGroupRootCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by group 'root' is OK when comparing Koji
# builds
class SlashUsrSlashBinOwnedByGroupRootCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by group 'root' is OK when comparing Koji
# builds
class SlashUsrSlashSbinOwnedByGroupRootCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by group 'bin' is BAD when comparing Koji builds
class SlashBinOwnedByGroupBinCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /sbin owned by group 'bin' is BAD when comparing Koji builds
class SlashSbinOwnedByGroupBinCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/bin owned by group 'bin' is BAD when comparing Koji
# builds
class SlashUsrSlashBinOwnedByGroupBinCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File in /usr/sbin owned by group 'bin' is BAD when comparing Koji
# builds
class SlashUsrSlashSbinOwnedByGroupBinCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

###################
# Forbidden owner #
###################

# File owned by 'mockbuild' is BAD in RPMs
class SlashBinOwnedByMockRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="mockbuild")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File owned by 'mockbuild' is BAD in Koji build
class SlashBinOwnedByMockKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="mockbuild")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File owned by 'mockbuild' is BAD when comparing RPMs
class SlashBinOwnedByMockCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="mockbuild")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File owned by 'mockbuild' is BAD when comparing Koji builds
class SlashBinOwnedByMockCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="mockbuild")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

###################
# Forbidden group #
###################

# File owned by group 'mockbuild' is BAD in RPMs
class SlashBinOwnedByGroupMockRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="mockbuild")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File owned by group 'mockbuild' is BAD in Koji build
class SlashBinOwnedByGroupMockKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="mockbuild")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File owned by group 'mockbuild' is BAD when comparing RPMs
class SlashBinOwnedByGroupMockCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="mockbuild")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File owned by group 'mockbuild' is BAD when comparing Koji builds
class SlashBinOwnedByGroupMockCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="mockbuild")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

###################################
# CAP_SETUID and world executable #
###################################

# File with CAP_SETUID and o+x is BAD in RPMs
class CapSETUIDWithOtherExecRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = 'bin/testscript'
        sourceId = self.rpm.add_source(rpmfluff.SourceFile('testscript.sh', script_source))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (sourceId, self.rpm.escape_path(installPath))
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File with CAP_SETUID and o+x is BAD in Koji build
class CapSETUIDWithOtherExecKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = 'bin/testscript'
        sourceId = self.rpm.add_source(rpmfluff.SourceFile('testscript.sh', script_source))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (sourceId, self.rpm.escape_path(installPath))
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File with CAP_SETUID and o+x is BAD when comparing RPMs
class CapSETUIDWithOtherExecCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = 'bin/testscript'
        sourceId = self.after_rpm.add_source(rpmfluff.SourceFile('testscript.sh', script_source))
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (sourceId, self.after_rpm.escape_path(installPath))
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File with CAP_SETUID and o+x is BAD when comparing Koji builds
class CapSETUIDWithOtherExecCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root w/ setuid capability and o+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = 'bin/testscript'
        sourceId = self.after_rpm.add_source(rpmfluff.SourceFile('testscript.sh', script_source))
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (sourceId, self.after_rpm.escape_path(installPath))
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(0707,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

#################################
# CAP_SETUID and group writable #
#################################

# File with CAP_SETUID and g+w is BAD in RPMs
# The installed file needs to be owned by a group other than
# bin_group, have execute permissions for owner/group/other, carry
# group write permissions, and carry the CAP_SETUID effective
# capability.
class CapSETUIDWithGroupExecRPMs(TestRPMs):
    """
    """
    def setUp(self):
        TestRPMs.setUp(self)

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = 'bin/testscript'
        sourceId = self.rpm.add_source(rpmfluff.SourceFile('testscript.sh', script_source))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (sourceId, self.rpm.escape_path(installPath))
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File with CAP_SETUID and g+w is BAD in Koji build
class CapSETUIDWithGroupExecKoji(TestKoji):
    """
    """
    def setUp(self):
        TestKoji.setUp(self)

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = 'bin/testscript'
        sourceId = self.rpm.add_source(rpmfluff.SourceFile('testscript.sh', script_source))
        self.rpm.create_parent_dirs(installPath)
        self.rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (sourceId, self.rpm.escape_path(installPath))
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File with CAP_SETUID and g+w is BAD when comparing RPMs
class CapSETUIDWithGroupExecCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = 'bin/testscript'
        sourceId = self.after_rpm.add_source(rpmfluff.SourceFile('testscript.sh', script_source))
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (sourceId, self.after_rpm.escape_path(installPath))
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File with CAP_SETUID and g+w is BAD when comparing Koji builds
class CapSETUIDWithGroupExecCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # add file owned by root w/ setuid capability and g+x mode
        # (basically duplicate add_installed_file() here because we
        # need to use the %caps macro)
        installPath = 'bin/testscript'
        sourceId = self.after_rpm.add_source(rpmfluff.SourceFile('testscript.sh', script_source))
        self.after_rpm.create_parent_dirs(installPath)
        self.after_rpm.section_install += "install -D %%{SOURCE%i} $RPM_BUILD_ROOT/%s\n" % (sourceId, self.after_rpm.escape_path(installPath))
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "%%attr(0770,root,bin) %%caps(cap_setuid=ep) /%s\n" % installPath

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

#################
# Owner changed #
#################

# File owner changed is BAD when comparing RPMs
class FileOwnerChangedCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Start with root owner
        self.before_rpm.add_installed_file('bin/testscript', rpmfluff.SourceFile('testscript.sh', script_source), mode="0755", owner="root")

        # Switch to bin owner
        self.after_rpm.add_installed_file('bin/testscript', rpmfluff.SourceFile('testscript.sh', script_source), mode="0755", owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File owner changed is BAD when comparing Koji builds
class FileOwnerChangedCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Start with root owner
        self.before_rpm.add_installed_file('bin/testscript', rpmfluff.SourceFile('testscript.sh', script_source), mode="0755", owner="root")

        # Switch to bin owner
        self.after_rpm.add_installed_file('bin/testscript', rpmfluff.SourceFile('testscript.sh', script_source), mode="0755", owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

#################
# Group changed #
#################

# File group changed is BAD when comparing RPMs
class FileGroupChangedCompareRPMs(TestCompareRPMs):
    """
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Start with root owner
        self.before_rpm.add_installed_file('bin/testscript', rpmfluff.SourceFile('testscript.sh', script_source), mode="0755", group="root")

        # Switch to bin owner
        self.after_rpm.add_installed_file('bin/testscript', rpmfluff.SourceFile('testscript.sh', script_source), mode="0755", group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'

# File group changed is BAD when comparing Koji builds
class FileGroupChangedCompareKoji(TestCompareKoji):
    """
    """
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Start with root owner
        self.before_rpm.add_installed_file('bin/testscript', rpmfluff.SourceFile('testscript.sh', script_source), mode="0755", group="root")

        # Switch to bin owner
        self.after_rpm.add_installed_file('bin/testscript', rpmfluff.SourceFile('testscript.sh', script_source), mode="0755", group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'
