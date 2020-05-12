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
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by 'root' is OK in RPMs
class SlashSbinOwnedByRootRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by 'root' is OK in RPMs
class SlashUsrSlashBinOwnedByRootRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by 'root' is OK in RPMs
class SlashUserSlashSbinOwnedByRootRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by 'bin' is BAD in RPMs
class SlashBinOwnedByBinRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by 'bin' is BAD in RPMs
class SlashSbinOwnedByBinRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by 'bin' is BAD in RPMs
class SlashUsrSlashBinOwnedByBinRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by 'bin' is BAD in RPMs
class SlashUserSlashSbinOwnedByBinRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'


# File in /bin owned by 'root' is OK in Koji build
class SlashBinOwnedByRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by 'root' is OK in Koji build
class SlashSbinOwnedByRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by 'root' is OK in Koji build
class SlashUsrSlashBinOwnedByRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by 'root' is OK in Koji build
class SlashUsrSlashSbinOwnedByRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by 'bin' is BAD in Koji build
class SlashBinOwnedByBinKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by 'bin' is BAD in Koji build
class SlashSbinOwnedByRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by 'bin' is BAD in Koji build
class SlashUsrSlashBinOwnedByRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by 'bin' is BAD in Koji build
class SlashUsrSlashSbinOwnedByRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by 'root' is OK when comparing RPMs
class SlashBinOwnedByRootCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by 'root' is OK when comparing RPMs
class SlashSbinOwnedByRootCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by 'root' is OK when comparing RPMs
class SlashUsrSlashBinOwnedByRootCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by 'root' is OK when comparing RPMs
class SlashUsrSlashSbinOwnedByRootCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by 'bin' is BAD when comparing RPMs
class SlashBinOwnedByBinCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by 'bin' is BAD when comparing RPMs
class SlashSbinOwnedByBinCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by 'bin' is BAD when comparing RPMs
class SlashUsrSlashBinOwnedByBinCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by 'bin' is BAD when comparing RPMs
class SlashUsrSlashSbinOwnedByBinCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by 'root' is OK when comparing Koji builds
class SlashBinOwnedByRootCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by 'root' is OK when comparing Koji builds
class SlashSbinOwnedByRootCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by 'root' is OK when comparing Koji builds
class SlashUsrSlashBinOwnedByRootCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by 'root' is OK when comparing Koji builds
class SlashUsrSlashSbinOwnedByRootCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by 'bin' is BAD when comparing Koji builds
class SlashBinOwnedByBinCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by 'bin' is BAD when comparing Koji builds
class SlashSbinOwnedByBinCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by 'bin' is BAD when comparing Koji builds
class SlashUsrSlashBinOwnedByBinCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by 'bin' is BAD when comparing Koji builds
class SlashUsrSlashSbinOwnedByBinCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

#############
# bin group #
#############

# File in /bin owned by group 'root' is OK in RPMs
class SlashBinOwnedByGroupRootRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by group 'root' is OK in RPMs
class SlashSbinOwnedByGroupRootRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by group 'root' is OK in RPMs
class SlashUsrSlashBinOwnedByGroupRootRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by group 'root' is OK in RPMs
class SlashUsrSlashSbinOwnedByGroupRootRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by group 'bin' is BAD in RPMs
class SlashBinOwnedByGroupBinRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by group 'bin' is BAD in RPMs
class SlashSbinOwnedByGroupBinRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by group 'bin' is BAD in RPMs
class SlashiUsrSlashBinOwnedByGroupBinRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by group 'bin' is BAD in RPMs
class SlashUsrSlashSbinOwnedByGroupBinRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by group 'root' is OK in Koji build
class SlashBinOwnedByGroupRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by group 'root' is OK in Koji build
class SlashSbinOwnedByGroupRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by group 'root' is OK in Koji build
class SlashUsrSlashBinOwnedByGroupRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by group 'root' is OK in Koji build
class SlashUsrSlashSbinOwnedByGroupRootKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by group 'bin' is BAD in Koji build
class SlashBinOwnedByGroupBinKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by group 'bin' is BAD in Koji build
class SlashSbinOwnedByGroupBinKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by group 'bin' is BAD in Koji build
class SlashUsrSlashBinOwnedByGroupBinKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by group 'bin' is BAD in Koji build
class SlashUsrSlashSbinOwnedByGroupBinKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by group 'root' is OK when comparing RPMs
class SlashBinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by group 'root' is OK when comparing RPMs
class SlashSbinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by group 'root' is OK when comparing RPMs
class SlashUsrSlashBinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by group 'root' is OK when comparing RPMs
class SlashUsrSlashSbinOwnedByGroupRootCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by group 'bin' is BAD when comparing RPMs
class SlashBinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by group 'bin' is BAD when comparing RPMs
class SlashSbinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by group 'bin' is BAD when comparing RPMs
class SlashUsrSlashBinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by group 'bin' is BAD when comparing RPMs
class SlashUsrSlashSbinOwnedByGroupBinCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by group 'root' is OK when comparing Koji builds
class SlashBinOwnedByGroupRootCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by group 'root' is OK when comparing Koji builds
class SlashSbinOwnedByGroupRootCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by group 'root' is OK when comparing Koji
# builds
class SlashUsrSlashBinOwnedByGroupRootCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by group 'root' is OK when comparing Koji
# builds
class SlashUsrSlashSbinOwnedByGroupRootCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# File in /bin owned by group 'bin' is BAD when comparing Koji builds
class SlashBinOwnedByGroupBinCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /sbin owned by group 'bin' is BAD when comparing Koji builds
class SlashSbinOwnedByGroupBinCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/bin owned by group 'bin' is BAD when comparing Koji
# builds
class SlashUsrSlashBinOwnedByGroupBinCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File in /usr/sbin owned by group 'bin' is BAD when comparing Koji
# builds
class SlashUsrSlashSbinOwnedByGroupBinCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='usr/sbin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="bin")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

###################
# Forbidden owner #
###################

# File owned by 'mockbuild' is BAD in RPMs
class SlashBinOwnedByMockRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="mock")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File owned by 'mockbuild' is BAD in Koji build
class SlashBinOwnedByMockKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="mock")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File owned by 'mockbuild' is BAD when comparing RPMs
class SlashBinOwnedByMockCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="mock")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File owned by 'mockbuild' is BAD when comparing Koji builds
class SlashBinOwnedByMockCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="mock")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

###################
# Forbidden group #
###################

# File owned by group 'mockbuild' is BAD in RPMs
class SlashBinOwnedByGroupMockRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="mock")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File owned by group 'mockbuild' is BAD in Koji build
class SlashBinOwnedByGroupMockKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="mock")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File owned by group 'mockbuild' is BAD when comparing RPMs
class SlashBinOwnedByGroupMockCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="mock")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File owned by group 'mockbuild' is BAD when comparing Koji builds
class SlashBinOwnedByGroupMockCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="mock")

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

###################################
# CAP_SETUID and world executable #
###################################

# File with CAP_SETUID and o+x is BAD in RPMs
class CapSETUIDWithOtherExecRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # now set the setuid capability and set o+x
        self.rpm.section_install += "( setcap 'cap_setuid+ep' /bin/testscript ; chmod o+x /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File with CAP_SETUID and o+x is BAD in Koji build
class CapSETUIDWithOtherExecKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # now set the setuid capability and set o+x
        self.rpm.section_install += "( setcap 'cap_setuid+ep' /bin/testscript ; chmod o+x /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File with CAP_SETUID and o+x is BAD when comparing RPMs
class CapSETUIDWithOtherExecCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # now set the setuid capability and set o+x
        self.after_rpm.section_install += "( setcap 'cap_setuid+ep' /bin/testscript ; chmod o+x /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File with CAP_SETUID and o+x is BAD when comparing Koji builds
class CapSETUIDWithOtherExecCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # now set the setuid capability and set o+x
        self.after_rpm.section_install += "( setcap 'cap_setuid+ep' /bin/testscript ; chmod o+x /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

#################################
# CAP_SETUID and group writable #
#################################

# File with CAP_SETUID and g+w is BAD in RPMs
class CapSETUIDWithGroupExecRPMs(TestRPMs):
    def setUP(self):
        TestRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # now set the setuid capability and set g+w
        self.rpm.section_install += "( setcap 'cap_setuid+ep' /bin/testscript ; chmod g+w /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File with CAP_SETUID and g+w is BAD in Koji build
class CapSETUIDWithGroupExecKoji(TestKoji):
    def setUP(self):
        TestKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # now set the setuid capability and set g+w
        self.rpm.section_install += "( setcap 'cap_setuid+ep' /bin/testscript ; chmod g+w /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File with CAP_SETUID and g+w is BAD when comparing RPMs
class CapSETUIDWithGroupExecCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # now set the setuid capability and set g+w
        self.after_rpm.section_install += "( setcap 'cap_setuid+ep' /bin/testscript ; chmod g+w /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

# File with CAP_SETUID and g+w is BAD when comparing Koji builds
class CapSETUIDWithGroupExecCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # now set the setuid capability and set g+w
        self.after_rpm.section_install += "( setcap 'cap_setuid+ep' /bin/testscript ; chmod g+w /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'BAD'
        self.waiver_auth = 'Not Waivable'

#################
# Owner changed #
#################

# File owner changed is VERIFY when comparing RPMs
class FileOwnerChangedCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # Change ownership to bin
        self.after_rpm.section_install += "( chown bin /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'VERIFY'
        self.waiver_auth = 'Not Waivable'

# File owner changed is VERIFY when comparing Koji builds
class FileOwnerChangedCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file owned by root
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    owner="root")

        # Change ownership to bin
        self.after_rpm.section_install += "( chown bin /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'VERIFY'
        self.waiver_auth = 'Not Waivable'

#################
# Group changed #
#################

# File group changed is VERIFY when comparing RPMs
class FileOwnerChangedCompareRPMs(TestCompareRPMs):
    def setUP(self):
        TestCompareRPMs.setUp(self)

        # add file with root group
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        # change group to bin
        self.after_rpm.section_install += "( chgrp bin /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'VERIFY'
        self.waiver_auth = 'Not Waivable'

# File group changed is VERIFY when comparing Koji builds
class FileOwnerChangedCompareKoji(TestCompareKoji):
    def setUP(self):
        TestCompareKoji.setUp(self)

        # add file with root group
        self.rpm.add_installed_file(installPath='bin/testscript',
                                    sourceFile=rpmfluff.SourceFile('testscript.sh', script_source),
                                    group="root")

        # change group to bin
        self.after_rpm.section_install += "( chgrp bin /bin/testscript )\n"

        self.inspection = 'ownership'
        self.label = 'ownership'
        self.result = 'VERIFY'
        self.waiver_auth = 'Not Waivable'
