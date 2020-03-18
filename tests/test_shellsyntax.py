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

from baseclass import TestRPMs, TestSRPM, TestKoji
from baseclass import TestCompareRPMs, TestCompareKoji
import rpmfluff

# The 'shellsyntax' inspection looks at shell scripts in the
# packages and reports of syntax validation errors.  Use the rpmfluff
# Python module to add valid and invalid shell scripts to the test
# RPMs for each test.  rpmfluff isn't documented, but it's a single
# Python file installed by the 'python3-rpmfluff' package.  See other
# tests for examples on how to use it as well.

# NOTE: We are only going to test the list of default shells as
# specified in rpminspect.conf.  See src/rpminspect.conf for more info.

###################################################################
# sh - Bourne/POSIX shell syntax only                             #
# Fedora and derivatives use 'dash', which is a Debian maintained #
# port of the Almquist shell:                                     #
#     https://en.wikipedia.org/wiki/Almquist_shell                #
# Scripts using /bin/sh should adhere to POSIX shell syntax.      #
###################################################################


valid_sh = """#!/bin/sh
valid=true
count=1
while [ $valid ]
do
echo $count
if [ $count -eq 5 ];
then
break
fi
((count++))
done
"""


invalid_sh = """#!/bin/sh
valid=true
count=1
while [ $valid ]
do
echo $count
if count -eq 5 ]
then
"""


class ShWellFormedRPM(TestRPMs):
    """
    Valid /bin/sh script is OK for RPMs
    """
    def setUp(self):
        TestRPMs.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/valid_sh.sh',
                                    rpmfluff.SourceFile('valid_sh.sh', valid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'OK'


class ShMalformedRPM(TestRPMs):
    """
    Invalid /bin/sh script is BAD for RPMs
    """
    def setUp(self):
        TestRPMs.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                    rpmfluff.SourceFile('invalid_sh.sh', invalid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class ShWellFormedKoji(TestKoji):
    """
    Valid /bin/sh script is OK for Koji build
    """
    def setUp(self):
        TestKoji.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/valid_sh.sh',
                                    rpmfluff.SourceFile('valid_sh.sh', valid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'OK'


class ShMalformedKoji(TestKoji):
    """
    Invalid /bin/sh script is BAD for Koji build
    """
    def setUp(self):
        TestKoji.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                    rpmfluff.SourceFile('invalid_sh.sh', invalid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class ShWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/sh script is OK for comparing RPMs
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_sh.sh',
                                           rpmfluff.SourceFile('valid_sh.sh', valid_sh))
        self.after_rpm.add_installed_file('/usr/share/data/valid_sh.sh',
                                          rpmfluff.SourceFile('valid_sh.sh', valid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'OK'


class ShMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/sh script is BAD for comparing RPMs
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                           rpmfluff.SourceFile('invalid_sh.sh', invalid_sh))
        self.after_rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                          rpmfluff.SourceFile('invalid_sh.sh', invalid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class ShWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/sh script is OK for comparing Koji builds
    """
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_sh.sh',
                                           rpmfluff.SourceFile('valid_sh.sh', valid_sh))
        self.after_rpm.add_installed_file('/usr/share/data/valid_sh.sh',
                                          rpmfluff.SourceFile('valid_sh.sh', valid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'OK'


class ShMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/sh script is BAD for comparing Koji builds
    """
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                           rpmfluff.SourceFile('invalid_sh.sh', invalid_sh))
        self.after_rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                          rpmfluff.SourceFile('invalid_sh.sh', invalid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class ShWellMalformedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/sh script in before, invalid in after is BAD when comparing RPMs
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_sh.sh',
                                           rpmfluff.SourceFile('valid_sh.sh', valid_sh))
        self.after_rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                          rpmfluff.SourceFile('invalid_sh.sh', invalid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class ShWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/sh script in before, invalid in after is BAD when comparing Koji builds
    """
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_sh.sh',
                                           rpmfluff.SourceFile('valid_sh.sh', valid_sh))
        self.after_rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                          rpmfluff.SourceFile('invalid_sh.sh', invalid_sh))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


##################################################################
# ksh - Korn shell                                               #
# There are two main varieties of ksh:  ksh88 and ksh93          #
# Both are from David Korn at AT&T and have been released in     #
# various forms for a long time.  The most widely used version   #
# in commercial Unix settings was ksh88.  ksh88 inspired shells  #
# like bash, zsh, and others with advanced editing capabilities. #
#     https://en.wikipedia.org/wiki/KornShell                    #
#                                                                #
# There is also the Public Domain Korn shell (pdksh) and mksh    #
# from MirOS BSD.  Neither of these are 100% compatible, but     #
# close.  Red Hat used to ship pdksh to provide something close  #
# to ksh for users.  Fedora and derivatives now ship ksh93 which #
# was open sourced in 2000 and is still maintained.  The tests   #
# here should assume /bin/ksh is AT&T ksh93.                     #
##################################################################

# Valid /bin/ksh script is OK for RPMs

# Invalid /bin/ksh script is BAD for RPMs

# Valid /bin/ksh script is OK for Koji build

# Invalid /bin/ksh script is BAD for Koji build

# Valid /bin/ksh script is OK for comparing RPMs

# Invalid /bin/ksh script is BAD for comparing RPMs

# Valid /bin/ksh script is OK for comparing Koji builds

# Invalid /bin/ksh script is BAD for comparing Koji builds

# Valid /bin/ksh script in before, invalid in after is BAD when
# comparing RPMs

# Valid /bin/ksh script in before, invalid in after is BAD when
# comparing Koji builds

###################################################################
# zsh - Z shell                                                   #
# Z shell began life at Princeton University in 1990 and has lead #
# an exciting life as a powerful interactive shell and scripting  #
# language.  It shares some compatibility with sh, ksh, and bash  #
# but should be treated as distinctly separate.                   #
#     https://en.wikipedia.org/wiki/Z_shell                       #
###################################################################

# Valid /bin/zsh script is OK for RPMs

# Invalid /bin/zsh script is BAD for RPMs

# Valid /bin/zsh script is OK for Koji build

# Invalid /bin/zsh script is BAD for Koji build

# Valid /bin/zsh script is OK for comparing RPMs

# Invalid /bin/zsh script is BAD for comparing RPMs

# Valid /bin/zsh script is OK for comparing Koji builds

# Invalid /bin/zsh script is BAD for comparing Koji builds

# Valid /bin/zsh script in before, invalid in after is BAD when
# comparing RPMs

# Valid /bin/zsh script in before, invalid in after is BAD when
# comparing Koji builds

#################################################################
# csh - The C Shell                                             #
# Bill Joy gave us the C shell while at UCB.  The BSD operating #
# system included it as a result and it's why BSD-derivatives   #
# tend to favor csh varieties by default.  No one should write  #
# in csh, but that doesn't stop people from doing so.  Be sure  #
# to use csh syntax in these scripts.                           #
#     https://en.wikipedia.org/wiki/C_shell                     #
#################################################################

# Valid /bin/csh script is OK for RPMs

# Invalid /bin/csh script is BAD for RPMs

# Valid /bin/csh script is OK for Koji build

# Invalid /bin/csh script is BAD for Koji build

# Valid /bin/csh script is OK for comparing RPMs

# Invalid /bin/csh script is BAD for comparing RPMs

# Valid /bin/csh script is OK for comparing Koji builds

# Invalid /bin/csh script is BAD for comparing Koji builds

# Valid /bin/csh script in before, invalid in after is BAD when
# comparing RPMs

# Valid /bin/csh script in before, invalid in after is BAD when
# comparing Koji builds

################################################################
# tcsh - TENEX C shell                                         #
# Inspired by TENEX, Ken Greer at Carnegie Mellon University   #
# wrote tcsh give TENEX-style completion to csh.               #
#     https://en.wikipedia.org/wiki/Tcsh                       #
################################################################

# Valid /bin/tcsh script is OK for RPMs

# Invalid /bin/tcsh script is BAD for RPMs

# Valid /bin/tcsh script is OK for Koji build

# Invalid /bin/tcsh script is BAD for Koji build

# Valid /bin/tcsh script is OK for comparing RPMs

# Invalid /bin/tcsh script is BAD for comparing RPMs

# Valid /bin/tcsh script is OK for comparing Koji builds

# Invalid /bin/tcsh script is BAD for comparing Koji builds

# Valid /bin/tcsh script in before, invalid in after is BAD when
# comparing RPMs

# Valid /bin/tcsh script in before, invalid in after is BAD when
# comparing Koji builds

#################################################################
# rc - Plan 9 shell                                             #
# From the Plan 9 operating system at Bell Labs comes 'rc', the #
# shell inspired by Bourne shell syntax but a little different. #
# It is unlikely a Linux user would encounter an rc script in   #
# the wild, but the shell is provided so we should check for    #
# those scripts.                                                #
#     https://en.wikipedia.org/wiki/Rc                          #
#################################################################

# Valid /bin/rc script is OK for RPMs

# Invalid /bin/rc script is BAD for RPMs

# Valid /bin/rc script is OK for Koji build

# Invalid /bin/rc script is BAD for Koji build

# Valid /bin/rc script is OK for comparing RPMs

# Invalid /bin/rc script is BAD for comparing RPMs

# Valid /bin/rc script is OK for comparing Koji builds

# Invalid /bin/rc script is BAD for comparing Koji builds

# Valid /bin/rc script in before, invalid in after is BAD when
# comparing RPMs

# Valid /bin/rc script in before, invalid in after is BAD when
# comparing Koji builds

#################################################################
# bash - Bourne again shell from GNU                            #
# Probably the most used shell these days, but also one of the  #
# most irritable with regards to version compatibility.  It is  #
# well known there exist different "bash-isms" that even vary   #
# by version.  When writing shell scripts, one should favor sh  #
# unless it is entirely unusable.  In those cases, maybe going  #
# with a different language is a better choice.                 #
#     https://en.wikipedia.org/wiki/Bash_(Unix_shell)
# NOTE: rpminspect checks syntax on bash scripts, but if it     #
# fails, it will try again with 'extglob' enabled.  If that     #
# passes it reports that the script needs 'extglob' enabled.    #
#################################################################


valid_bash = """#!/bin/bash
valid=true
count=1
while [ $valid ]
do
echo $count
if [ $count -eq 5 ];
then
break
fi
((count++))
done
"""


invalid_bash = """#!/bin/bash
valid=true
count=1
while [ $valid ]
do
echo $count
if count -eq 5 ]
then
"""


valid_bash_extglob = """#!/bin/bash
echo !(*/)
"""


class BashWellFormedRPM(TestRPMs):
    """
    Valid /bin/bash script is OK for RPMs
    """
    def setUp(self):
        TestRPMs.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/valid_bash.sh',
                                    rpmfluff.SourceFile('valid_bash.sh', valid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'OK'


class BashMalformedRPM(TestRPMs):
    """
    Invalid /bin/bash script is BAD for RPMs
    """
    def setUp(self):
        TestRPMs.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/invalid_bash.sh',
                                    rpmfluff.SourceFile('invalid_bash.sh', invalid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class BashExtglobWellFormedRPM(TestRPMs):
    """
    Valid but requires 'extglob' script is OK for RPMs
    """
    def setUp(self):
        TestRPMs.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/valid_bash_extglob.sh',
                                    rpmfluff.SourceFile('valid_bash_extglob.sh', valid_bash_extglob))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'INFO'


class BashWellFormedKoji(TestKoji):
    """
    Valid /bin/bash script is OK for Koji build
    """
    def setUp(self):
        TestKoji.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/valid_bash.sh',
                                    rpmfluff.SourceFile('valid_bash.sh', valid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'OK'


class BashMalformedKoji(TestKoji):
    """
    Invalid /bin/bash script is BAD for Koji build
    """
    def setUp(self):
        TestKoji.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/invalid_bash.sh',
                                    rpmfluff.SourceFile('invalid_bash.sh', invalid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class BashExtglobWellFormedKoji(TestKoji):
    """
    Valid but requires 'extglob' script is OK for Koji build
    """
    def setUp(self):
        TestKoji.setUp(self)

        self.rpm.add_installed_file('/usr/share/data/valid_bash_extglob.sh',
                                    rpmfluff.SourceFile('valid_bash_extglob.sh', valid_bash_extglob))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'INFO'


class BashWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/bash script is OK for comparing RPMs
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_bash.sh',
                                           rpmfluff.SourceFile('valid_bash.sh', valid_bash))
        self.after_rpm.add_installed_file('/usr/share/data/valid_bash.sh',
                                          rpmfluff.SourceFile('valid_sh.sh', valid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'OK'


class BashMalformedCompareRPMs(TestCompareRPMs):
    """
    Invalid /bin/bash script is BAD for comparing RPMs
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/invalid_bash.sh',
                                           rpmfluff.SourceFile('invalid_bash.sh', invalid_bash))
        self.after_rpm.add_installed_file('/usr/share/data/invalid_bash.sh',
                                          rpmfluff.SourceFile('invalid_bash.sh', invalid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class BashExtglobWellFormedCompareRPMs(TestCompareRPMs):
    """
    Valid but requires 'extglob' script is OK for comparing RPMs
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_bash_extglob.sh',
                                           rpmfluff.SourceFile('valid_bash_extglob.sh', valid_bash_extglob))
        self.after_rpm.add_installed_file('/usr/share/data/valid_bash_extglob.sh',
                                          rpmfluff.SourceFile('valid_bash_extglob.sh', valid_bash_extglob))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'INFO'


class BshWellFormedCompareKoji(TestCompareKoji):
    """
    Valid /bin/bash script is OK for comparing Koji builds
    """
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_bash.sh',
                                           rpmfluff.SourceFile('valid_bash.sh', valid_bash))
        self.after_rpm.add_installed_file('/usr/share/data/valid_bash.sh',
                                          rpmfluff.SourceFile('valid_bash.sh', valid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'OK'


class BahMalformedCompareKoji(TestCompareKoji):
    """
    Invalid /bin/bash script is BAD for comparing Koji builds
    """
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/invalid_bash.sh',
                                           rpmfluff.SourceFile('invalid_bash.sh', invalid_bash))
        self.after_rpm.add_installed_file('/usr/share/data/invalid_bash.sh',
                                          rpmfluff.SourceFile('invalid_bash.sh', invalid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class BashExtglobWellFormedCompareKoji(TestCompareKoji):
    """
    Valid but requires 'extglob' script is OK for comparing Koji builds
    """
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_bash_extglob.sh',
                                           rpmfluff.SourceFile('valid_bash_extglob.sh', valid_bash_extglob))
        self.after_rpm.add_installed_file('/usr/share/data/valid_bash_extglob.sh',
                                          rpmfluff.SourceFile('valid_bash_extglob.sh', valid_bash_extglob))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'INFO'


class BashWellMalformedCompareRPMs(TestCompareRPMs):
    """
    Valid /bin/bash script in before, invalid in after is BAD when comparing RPMs
    """
    def setUp(self):
        TestCompareRPMs.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_bash.sh',
                                           rpmfluff.SourceFile('valid_bash.sh', valid_bash))
        self.after_rpm.add_installed_file('/usr/share/data/invalid_bash.sh',
                                          rpmfluff.SourceFile('invalid_bash.sh', invalid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'


class BashWellMalformedCompareKoji(TestCompareKoji):
    """
    Valid /bin/bash script in before, invalid in after is BAD when comparing Koji builds
    """
    def setUp(self):
        TestCompareKoji.setUp(self)
        self.before_rpm.add_installed_file('/usr/share/data/valid_bash.sh',
                                           rpmfluff.SourceFile('valid_bash.sh', valid_bash))
        self.after_rpm.add_installed_file('/usr/share/data/invalid_sh.sh',
                                          rpmfluff.SourceFile('invalid_bash.sh', invalid_bash))
        self.inspection = 'shellsyntax'
        self.label = 'shell-syntax'
        self.result = 'BAD'
        self.waiver_auth = 'Anyone'
