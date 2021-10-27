#
# Copyright © 2021 Red Hat, Inc.
# Author(s): David Cantrell <dcantrell@redhat.com>
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
import rpmfluff

from baseclass import BEFORE_NAME, BEFORE_VER, AFTER_NAME, AFTER_VER

from baseclass import (
    TestSRPM,
    TestRPMs,
    TestKoji,
    TestCompareSRPM,
    TestCompareRPMs,
    TestCompareKoji,
)

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"]
good_c_src = open(os.path.join(datadir, "unicode", "good.c")).read()
bad_c_src = open(os.path.join(datadir, "unicode", "bad.c")).read()
bad_asm_src = open(os.path.join(datadir, "unicode", "bad.s")).read()
status_src = open(os.path.join(datadir, "unicode", "status.sh")).read()
makefile_src = open(os.path.join(datadir, "unicode", "Makefile")).read()


# The following tests run the unicode inspection with a known good source
# file in the SRPM.  All of the results for these should be 'OK'.
class UnicodeGoodSourceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("good.c", good_c_src))

        self.inspection = "unicode"
        self.result = "OK"


# The following tests run the unicode inspection with a known good source
# file in a tarball in the SRPM.  All of the results for these should be 'OK'.
class UnicodeGoodSourceArchiveSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceArchiveRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceArchiveKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceArchiveCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_build += "make\n"
        self.before_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.after_rpm.header += "\n%define debug_package %{nil}\n"
        self.after_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceArchiveCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_build += "make\n"
        self.before_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.after_rpm.header += "\n%define debug_package %{nil}\n"
        self.after_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodSourceArchiveCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_build += "make\n"
        self.before_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.after_rpm.header += "\n%define debug_package %{nil}\n"
        self.after_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("good.c", good_c_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.result = "OK"


# The following tests run the unicode inspection with a known bad source file
# in the SRPM.  All of the results for these should be 'BAD' with a waiver
# authorization of "Security" except for the jobs that lack any SRPM file.
class UnicodeBadSourceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadSourceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadSourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadSourceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


# The following tests run the unicode inspection with known bad source
# files in a tarball in the SRPM.  All of the results for these should be 'OK'
# except for the jobs that lack any SRPM file.
class UnicodeBadSourceArchiveSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadSourceArchiveRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadSourceArchiveKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadSourceArchiveCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_build += "make\n"
        self.before_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.after_rpm.header += "\n%define debug_package %{nil}\n"
        self.after_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadSourceArchiveCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_build += "make\n"
        self.before_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.after_rpm.header += "\n%define debug_package %{nil}\n"
        self.after_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadSourceArchiveCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_build += "make\n"
        self.before_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.before_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.after_rpm.header += "\n%define debug_package %{nil}\n"
        self.after_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", bad_c_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"
