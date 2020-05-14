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
    For RPMs in Koji builds, check that a file owned by root in the /sbin directory passes.
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
    For RPMs in Koji builds, check that a file owned by root in the /usr/bin directory passes.
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
    For RPMs in Koji builds, check that a file owned by root in the /usr/sbin directory passes.
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
    For RPMs in Koji builds, check that a file owned by bin in the /bin directory fails.
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
    For RPMs in Koji builds, check that a file owned by bin in the /sbin directory fails.
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
    For RPMs in Koji builds, check that a file owned by bin in the /usr/bin directory fails.
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
    For RPMs in Koji builds, check that a file owned by bin in the /usr/sbin directory fails.
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
    When comparing RPMs, check that a file in the new build owned by root in the /bin directory passes.
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by 'root' is OK when comparing RPMs
class SlashSbinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by root in the /sbin directory passes.
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by 'root' is OK when comparing RPMs
class SlashUsrSlashBinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by root in the /usr/bin directory passes.
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by 'root' is OK when comparing RPMs
class SlashUsrSlashSbinOwnedByRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by root in the /usr/sbin directory passes.
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by 'bin' is BAD when comparing RPMs
class SlashBinOwnedByBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build owned by bin in the /bin directory fails.
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
    When comparing RPMs, check that a file in the new build owned by bin in the /sbin directory fails.
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
    When comparing RPMs, check that a file in the new build owned by bin in the /usr/bin directory fails.
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
    When comparing RPMs, check that a file in the new build owned by bin in the /usr/sbin directory fails.
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
    When comparing RPMs in Koji builds, check that a file in the new build owned by root in the /bin directory passes.
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
    When comparing RPMs in Koji builds, check that a file in the new build owned by root in the /sbin directory passes.
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
    When comparing RPMs in Koji builds, check that a file in the new build owned by root in the /usr.bin directory passes.
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
    When comparing RPMs in Koji builds, check that a file in the new build owned by root in the /usr/sbin directory passes.
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
    When comparing RPMs in Koji builds, check that a file in the new build owned by bin in the /bin directory fails.
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
    When comparing RPMs in Koji builds, check that a file in the new build owned by bin in the /sbin directory fails.
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
    When comparing RPMs in Koji builds, check that a file in the new build owned by bin in the /usr/bin directory fails.
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
    When comparing RPMs in Koji builds, check that a file in the new build owned by bin in the /usr/sbin directory fails.
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
    For binary RPM files, check that a file with root group permission in the /bin directory passes.
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
    For binary RPM files, check that a file with root group permission in the /sbin directory passes.
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
    For binary RPM files, check that a file with root group permission in the /usr/bin directory passes.
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
    For binary RPM files, check that a file with root group permission in the /usr/sbin directory passes.
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
    For binary RPM files, check that a file with bin group permission in the /bin directory fails.
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
    For binary RPM files, check that a file with bin group permission in the /sbin directory fails.
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
    For binary RPM files, check that a file with bin group permission in the /usr/bin directory fails.
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
    For binary RPM files, check that a file with bin group permission in the /usr/sbin directory fails.
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
    For RPMs in Koji builds, check that a file with root group permission in the /bin directory passes.
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
    For RPMs in Koji builds, check that a file with root group permission in the /sbin directory passes.
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
    For RPMs in Koji builds, check that a file with root group permission in the /usr/bin directory passes.
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
    For RPMs in Koji builds, check that a file with root group permission in the /usr/sbin directory passes.
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
    For RPMs in Koji builds, check that a file with bin group permission in the /bin directory fails.
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
    For RPMs in Koji builds, check that a file with bin group permission in the /sbin directory fails.
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
    For RPMs in Koji builds, check that a file with bin group permission in the /usr/bin directory fails.
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
    For RPMs in Koji builds, check that a file with bin group permission in the /usr/sbin directory fails.
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
    When comparing RPMs, check that a file in the new build with root group permissions in the /bin directory passes.
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /sbin owned by group 'root' is OK when comparing RPMs
class SlashSbinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with root group permissions in the /sbin directory passes.
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/bin owned by group 'root' is OK when comparing RPMs
class SlashUsrSlashBinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with root group permissions in the /usr/bin directory passes.
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /usr/sbin owned by group 'root' is OK when comparing RPMs
class SlashUsrSlashSbinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with root group permissions in the /usr/sbin directory passes.
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.after_rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'

# File in /bin owned by group 'bin' is BAD when comparing RPMs
class SlashBinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    """
    When comparing RPMs, check that a file in the new build with bin group permissions in the /bin directory fails.
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
    When comparing RPMs, check that a file in the new build with bin group permissions in the /sbin directory fails.
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
    When comparing RPMs, check that a file in the new build with bin group permissions in the /usr/bin directory fails.
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
    When comparing RPMs, check that a file in the new build with bin group permissions in the /usr/sbin directory fails.
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
    When comparing RPMs in Koji builds, check that a file in the new build with root group permissions in the /bin directory passes.
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
    When comparing RPMs in Koji builds, check that a file in the new build with root group permissions in the /sbin directory passes.
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
    When comparing RPMs in Koji builds, check that a file in the new build with root group permissions in the /usr/bin directory passes.
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
    When comparing RPMs in Koji builds, check that a file in the new build with root group permissions in the /usr/sbin directory passes.
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
    When comparing RPMs in Koji builds, check that a file in the new build with bin group permissions in the /bin directory fails.
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
    When comparing RPMs in Koji builds, check that a file in the new build with bin group permissions in the /sbin directory fails.
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
    When comparing RPMs in Koji builds, check that a file in the new build with bin group permissions in the /usr/bin directory fails.
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
    When comparing RPMs in Koji builds, check that a file in the new build with bin group permissions in the /usr/sbin directory fails.
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
    For binary RPM files, check that a file owned by mockbuild fails.
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
    For RPMs in Koji builds, check that a file owned by mockbuild fails.
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
    When comparing RPMs, check that a file owned by mockbuild in the new build fails.
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
    When comparing RPMs in Koji builds, check that a file owned by mockbuild in the new build fails.
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
    For binary RPM files, check that a file with mockbuild group permission fails.
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
    For RPMs in Koji builds, check that a file with mockbuild group permission fails.
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
    When comparing RPMs, check that a file with mockbuild group permission in the new build fails.
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
    When comparing RPMs in Koji builds, check that a file with mockbuild group permission in the new build fails.
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
    For RPM binary files, check that a file with CAP_SETUID set and world execution permissions fails.
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
    For RPMs from Koji builds, check that a file with CAP_SETUID set and world execution permissions fails.
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
    When comparing RPMs, check that a file in the new build with CAP_SETUID set and world execution permissions fails.
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
    When comparing RPMs from Koji builds, check that a file in the new build with CAP_SETUID set and world execution permissions fails.
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
    For RPM binary files, check that a file with CAP_SETUID set and group execution permissions fails.
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
    For RPMs in Koji builds, check that a file with CAP_SETUID set and group execution permissions fails.
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
    When comparing RPMs, check that a file in the new build with CAP_SETUID set and world execution permissions fails.
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
    When comparing RPMs from Koji builds, check that a file in the new build with CAP_SETUID set and world execution permissions fails.
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
    When comparing RPMs, check that file ownership changing fails.
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
    When comparing RPMs from Koji builds, check that file ownership changing fails.
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
    When comparing RPMs, check that group ownership changing fails.
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
    When comparing RPMs from Koji builds, check that group ownership changing fails.
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
