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
import shutil
import unittest
import rpmfluff
from baseclass import TestCompareRPMs, TestCompareKoji

# Kernel version where the tests are running, will be used
# for module building during the tests.
kver = os.uname()[2]

# All of these tests require the kernel devel package installed
have_kernel_devel = os.path.isfile(os.path.join('/lib/modules', kver, 'build/Makefile'))

# Support functions to build the kernel modules we need
def build_module(rpminspect, build_ext=None, extra_cflags=None):
    build = os.path.dirname(rpminspect)
    srcdir = os.path.realpath(os.path.join(build, '..', '..', 'test', 'data', 'derp-kmod'))

    if build_ext is None:
        moddir = os.path.join(build, 'derp-kmod')
    else:
        moddir = os.path.join(build, 'derp-kmod' + build_ext)

    kmod = os.path.join(moddir, 'derp.ko')

    if os.path.isdir(moddir) and os.path.isfile(kmod):
        f = open(kmod, 'rb')
        kmodbuf = f.read()
        f.close()
        return kmodbuf

    shutil.rmtree(moddir, ignore_errors=True);
    shutil.copytree(srcdir, moddir)

    cwd = os.getcwd()
    os.chdir(moddir)

    if extra_cflags is None:
        cmd = "make -s"
    else:
        cmd = "make -s EXTRA_CFLAGS=" + extra_cflags

    os.system(cmd)
    os.chdir(cwd)

    # read in the module and return it
    f = open(kmod, 'rb')
    kmodbuf = f.read()
    f.close()
    return kmodbuf

def get_derp_kmod(rpminspect):
    return build_module(rpminspect)

def get_derp_kmod_params(rpminspect):
    return build_module(rpminspect, build_ext='-params', extra_cflags='-D_USE_MODULE_PARAMETERS')

def get_derp_kmod_depends(rpminspect):
    return build_module(rpminspect, build_ext='-depends', extra_cflags='-D_USE_MODULE_DEPENDS')

def get_derp_kmod_aliases(rpminspect):
    return build_module(rpminspect, build_ext='-aliases', extra_cflags='-D_USE_MODULE_ALIASES')

############################
# kernel module parameters #
############################

# Verify kmod params are good between before and after RPMs
class GoodKmodParamsRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_params(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_params(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Report loss of kmod params between before and after RPMs
class LostKmodParmsRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_params(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Verify kmod params are good between before and after Koji builds
class GoodKmodParamsKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareKoji.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_params(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_params(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Report loss of kmod params between before and after Koji builds
class LostKmodParamsKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareKoji.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_params(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

#########################
# kernel module depends #
#########################

# Verify kmod depends are good between before and after RPMs
class GoodKmodDependsRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Report loss of kmod depends between before and after RPMs
class LostKmodDependsRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Verify kmod depends are good between before and after Koji builds
class GoodKmodDependsKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareKoji.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Report loss of kmod depends between before and after Koji builds
class LostKmodDependsKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareKoji.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

#########################
# kernel module aliases #
#########################

# Verify kmod aliases are good between before and after RPMs
class GoodKmodAliasesRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_aliases(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_aliases(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Report loss of kmod aliases between before and after RPMs
class LostKmodAliasesRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_aliases(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Verify kmod aliases are good between before and after Koji builds
class GoodKmodAliasesKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareKoji.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_aliases(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_aliases(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

# Report loss of kmod aliases between before and after Koji builds
class LostKmodAliasesKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareKoji.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_aliases(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'VERIFY'
        self.waiver_auth = 'Anyone'

# Kernel module changing paths
class KmodChangingPathCompareRPMs(TestCompareRPMs):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareRPMs.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derpy-stuff/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'

class KmodChangingPathCompareKoji(TestCompareKoji):
    @unittest.skipUnless(have_kernel_devel, "Need kernel devel files")
    def setUp(self):
        TestCompareKoji.setUp(self)

        self.before_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))
        self.after_rpm.add_installed_file('/usr/lib/modules/' + kver + '/extra/drivers/derpy-stuff/derp.ko', rpmfluff.SourceFile('derp.ko', get_derp_kmod_depends(self.rpminspect)))

        self.inspection = 'kmod'
        self.label = 'kernel-modules'
        self.result = 'OK'
        self.waiver_auth = 'Not Waivable'
