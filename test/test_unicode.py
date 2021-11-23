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

import codecs
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
good_c_src = codecs.open(
    os.path.join(datadir, "unicode", "good.c"), encoding="utf-8"
).read()
bad_c_src = codecs.open(
    os.path.join(datadir, "unicode", "bad.c"), encoding="utf-8"
).read()
bad_asm_src = codecs.open(
    os.path.join(datadir, "unicode", "bad.s"), encoding="utf-8"
).read()
status_src = codecs.open(
    os.path.join(datadir, "unicode", "status.sh"), encoding="utf-8"
).read()
makefile_src = codecs.open(
    os.path.join(datadir, "unicode", "Makefile"), encoding="utf-8"
).read()

commenting_out_new_src = codecs.open(
    os.path.join(datadir, "unicode", "commenting-out-new.c"), encoding="utf-8"
).read()
early_return_new_src = codecs.open(
    os.path.join(datadir, "unicode", "early-return-new.c"), encoding="utf-8"
).read()
stretched_string_new_src = codecs.open(
    os.path.join(datadir, "unicode", "stretched-string-new.c"), encoding="utf-8"
).read()


# The following tests run the unicode inspection with a known good source
# file in the SRPM.  All of the results for these should be 'OK'.
class UnicodeGoodCSourceSRPM(TestSRPM):
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


class UnicodeGoodCSourceRPMs(TestRPMs):
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


class UnicodeGoodCSourceKoji(TestKoji):
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


class UnicodeGoodCSourceCompareSRPM(TestCompareSRPM):
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


class UnicodeGoodCSourceCompareRPMs(TestCompareRPMs):
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


class UnicodeGoodCSourceCompareKoji(TestCompareKoji):
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
class UnicodeGoodCSourceArchiveSRPM(TestSRPM):
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


class UnicodeGoodCSourceArchiveRPMs(TestRPMs):
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


class UnicodeGoodCSourceArchiveKoji(TestKoji):
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


class UnicodeGoodCSourceArchiveCompareSRPM(TestCompareSRPM):
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


class UnicodeGoodCSourceArchiveCompareRPMs(TestCompareRPMs):
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


class UnicodeGoodCSourceArchiveCompareKoji(TestCompareKoji):
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
class UnicodeBadCSourceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadCSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceCompareSRPM(TestCompareSRPM):
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
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceCompareRPMs(TestCompareRPMs):
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
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadCSourceCompareKoji(TestCompareKoji):
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
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


# The following tests run the unicode inspection with known bad source
# files in a tarball in the SRPM.  All of the results for these should be 'OK'
# except for the jobs that lack any SRPM file.
class UnicodeBadCSourceArchiveSRPM(TestSRPM):
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


class UnicodeBadCSourceArchiveRPMs(TestRPMs):
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


class UnicodeBadCSourceArchiveKoji(TestKoji):
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


class UnicodeBadCSourceArchiveCompareSRPM(TestCompareSRPM):
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


class UnicodeBadCSourceArchiveCompareRPMs(TestCompareRPMs):
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


class UnicodeBadCSourceArchiveCompareKoji(TestCompareKoji):
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


# The following tests run the unicode inspection with a known bad source file
# in the SRPM.  All of the results for these should be 'BAD' with a waiver
# authorization of "Security" except for the jobs that lack any SRPM file.
class UnicodeBadAsmSourceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadAsmSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadAsmSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadAsmSourceCompareSRPM(TestCompareSRPM):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadAsmSourceCompareRPMs(TestCompareRPMs):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadAsmSourceCompareKoji(TestCompareKoji):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.s", bad_asm_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


# The following tests run the unicode inspection with known bad source
# files in a tarball in the SRPM.  All of the results for these should be 'OK'
# except for the jobs that lack any SRPM file.
class UnicodeBadAsmSourceArchiveSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
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


class UnicodeBadAsmSourceArchiveRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
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


class UnicodeBadAsmSourceArchiveKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
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


class UnicodeBadAsmSourceArchiveCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
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


class UnicodeBadAsmSourceArchiveCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
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


class UnicodeBadAsmSourceArchiveCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
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


# The following tests run the unicode inspection with a known bad source file
# in the SRPM.  All of the results for these should be 'BAD' with a waiver
# authorization of "Security" except for the jobs that lack any SRPM file.
class UnicodeBadCommentingOutSourceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCommentingOutSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", bad_c_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadCommentingOutSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", commenting_out_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCommentingOutSourceCompareSRPM(TestCompareSRPM):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", commenting_out_new_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", commenting_out_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCommentingOutSourceCompareRPMs(TestCompareRPMs):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", commenting_out_new_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", commenting_out_new_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadCommentingOutSourceCompareKoji(TestCompareKoji):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", commenting_out_new_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", commenting_out_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


# The following tests run the unicode inspection with known bad source
# files in a tarball in the SRPM.  All of the results for these should be 'OK'
# except for the jobs that lack any SRPM file.
class UnicodeBadCommentingOutSourceArchiveSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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


class UnicodeBadCommentingOutSourceArchiveRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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


class UnicodeBadCommentingOutSourceArchiveKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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


class UnicodeBadCommentingOutSourceArchiveCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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


class UnicodeBadCommentingOutSourceArchiveCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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


class UnicodeBadCommentingOutSourceArchiveCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
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


# The following tests run the unicode inspection with a known bad source file
# in the SRPM.  All of the results for these should be 'BAD' with a waiver
# authorization of "Security" except for the jobs that lack any SRPM file.
class UnicodeBadEarlyReturnSourceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadEarlyReturnSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadEarlyReturnSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadEarlyReturnSourceCompareSRPM(TestCompareSRPM):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadEarlyReturnSourceCompareRPMs(TestCompareRPMs):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadEarlyReturnSourceCompareKoji(TestCompareKoji):
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
        self.before_rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))
        self.after_rpm.add_source(rpmfluff.SourceFile("bad.c", early_return_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


# The following tests run the unicode inspection with known bad source
# files in a tarball in the SRPM.  All of the results for these should be 'OK'
# except for the jobs that lack any SRPM file.
class UnicodeBadEarlyReturnSourceArchiveSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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


class UnicodeBadEarlyReturnSourceArchiveRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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


class UnicodeBadEarlyReturnSourceArchiveKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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


class UnicodeBadEarlyReturnSourceArchiveCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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


class UnicodeBadEarlyReturnSourceArchiveCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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


class UnicodeBadEarlyReturnSourceArchiveCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
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


# The following tests run the unicode inspection with a known bad source file
# in the SRPM.  All of the results for these should be 'BAD' with a waiver
# authorization of "Security" except for the jobs that lack any SRPM file.
class UnicodeBadStretchStringSourceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", stretched_string_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadStretchStringSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", stretched_string_new_src))

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadStretchStringSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status", rpmfluff.SourceFile("status.sh", status_src), mode=755
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(rpmfluff.SourceFile("bad.c", stretched_string_new_src))

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadStretchStringSourceCompareSRPM(TestCompareSRPM):
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
        self.before_rpm.add_source(
            rpmfluff.SourceFile("bad.c", stretched_string_new_src)
        )
        self.after_rpm.add_source(
            rpmfluff.SourceFile("bad.c", stretched_string_new_src)
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadStretchStringSourceCompareRPMs(TestCompareRPMs):
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
        self.before_rpm.add_source(
            rpmfluff.SourceFile("bad.c", stretched_string_new_src)
        )
        self.after_rpm.add_source(
            rpmfluff.SourceFile("bad.c", stretched_string_new_src)
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadStretchStringSourceCompareKoji(TestCompareKoji):
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
        self.before_rpm.add_source(
            rpmfluff.SourceFile("bad.c", stretched_string_new_src)
        )
        self.after_rpm.add_source(
            rpmfluff.SourceFile("bad.c", stretched_string_new_src)
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


# The following tests run the unicode inspection with known bad source
# files in a tarball in the SRPM.  All of the results for these should be 'OK'
# except for the jobs that lack any SRPM file.
class UnicodeBadStretchStringSourceArchiveSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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


class UnicodeBadStretchStringSourceArchiveRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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


class UnicodeBadStretchStringSourceArchiveKoji(TestKoji):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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


class UnicodeBadStretchStringSourceArchiveCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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


class UnicodeBadStretchStringSourceArchiveCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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


class UnicodeBadStretchStringSourceArchiveCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
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


# The following tests run the unicode inspection with a known good source
# file in a tarball in the SRPM, but the %prep section in the spec
# file is bad so the rpmSpecBuild() call will fail and rpminspect will have
# to fall back on manual unpacking.  All of the results for these should be
# 'OK'.  We have to perform the test using the TestSRPM and TestCompareSRPM
# classes because we need to construct a known-bad spec file.
class UnicodeGoodCSourceArchiveBadPrepSRPM(TestSRPM):
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
        self.rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodCSourceArchiveCompareBadPrepSRPM(TestCompareSRPM):
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
        self.before_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
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
        self.after_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.result = "OK"


# The following tests run the unicode inspection with a known bad source
# file in a tarball in the SRPM, but the %prep section in the spec
# file is bad so the rpmSpecBuild() call will fail and rpminspect will have
# to fall back on manual unpacking.  All of the results for these should be
# 'BAD'.  We have to perform the test using the TestSRPM and TestCompareSRPM
# classes because we need to construct a known-bad spec file.
class UnicodeBadCSourceArchiveBadPrepSRPM(TestSRPM):
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
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceArchiveCompareBadPrepSRPM(TestCompareSRPM):
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
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
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
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadAsmSourceArchiveBadPrepSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadAsmSourceArchiveCompareBadPrepSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
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
                    rpmfluff.SourceFile("bad.s", bad_asm_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCommentingOutSourceArchiveBadPrepSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCommentingOutSourceArchiveCompareBadPrepSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
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
                    rpmfluff.SourceFile("bad.c", commenting_out_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadEarlyReturnSourceArchiveBadPrepSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadEarlyReturnSourceArchiveCompareBadPrepSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
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
                    rpmfluff.SourceFile("bad.c", early_return_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadStretchStringSourceArchiveBadPrepSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.rpm.section_prep = "%setup\n"
        self.rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadStretchStringSourceArchiveCompareBadPrepSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.header += "\n%define debug_package %{nil}\n"
        self.before_rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (BEFORE_NAME, BEFORE_VER),
                "%s-%s" % (BEFORE_NAME, BEFORE_VER),
                [
                    rpmfluff.SourceFile("Makefile", makefile_src),
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.before_rpm.section_prep = "%setup\n"
        self.before_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
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
                    rpmfluff.SourceFile("bad.c", stretched_string_new_src),
                    rpmfluff.SourceFile("status.sh", status_src),
                ],
            )
        )
        self.after_rpm.section_prep = "%setup\n"
        self.after_rpm.section_prep += "/usr/bin/missing-command --do-some-stuff\n\n"
        self.after_rpm.section_build += "make\n"
        self.after_rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.after_rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"
