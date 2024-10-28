#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import rpmfluff
import yaml

from baseclass import BEFORE_NAME, BEFORE_VER, AFTER_NAME, AFTER_VER

from baseclass import (
    ProvidedSourceFile,
    TestSRPM,
    TestRPMs,
    TestKoji,
    TestCompareSRPM,
    TestCompareRPMs,
    TestCompareKoji,
)

datadir = os.environ["RPMINSPECT_TEST_DATA_PATH"]


# The following tests run the unicode inspection with a known good source
# file in the SRPM.  All of the results for these should be 'OK'.
class UnicodeGoodCSourceSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodCSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodCSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodCSourceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodCSourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodCSourceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("good.c", os.path.join(datadir, "unicode", "good.c"))
        )

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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadCSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadCSourceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadAsmSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadAsmSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("bad.s", os.path.join(datadir, "unicode", "bad.s"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadAsmSourceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("bad.s", os.path.join(datadir, "unicode", "bad.s"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("bad.s", os.path.join(datadir, "unicode", "bad.s"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadAsmSourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("bad.s", os.path.join(datadir, "unicode", "bad.s"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("bad.s", os.path.join(datadir, "unicode", "bad.s"))
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadAsmSourceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("bad.s", os.path.join(datadir, "unicode", "bad.s"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("bad.s", os.path.join(datadir, "unicode", "bad.s"))
        )

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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCommentingOutSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("bad.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadCommentingOutSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "commenting-out-new.c")
            )
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCommentingOutSourceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "commenting-out-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "commenting-out-new.c")
            )
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCommentingOutSourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "commenting-out-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "commenting-out-new.c")
            )
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadCommentingOutSourceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "commenting-out-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "commenting-out-new.c")
            )
        )

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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadEarlyReturnSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadEarlyReturnSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadEarlyReturnSourceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadEarlyReturnSourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadEarlyReturnSourceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
            )
        )

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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadStretchStringSourceRPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
        )

        self.inspection = "unicode"

        # the result here is OK because the unicode inspection only works on SRPMs and
        # if we don't have those it is a no-op inspection that returns OK
        self.result = "OK"


class UnicodeBadStretchStringSourceKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadStretchStringSourceCompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadStretchStringSourceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
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
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "bad.c", os.path.join(datadir, "unicode", "stretched-string-new.c")
            )
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "good.c", os.path.join(datadir, "unicode", "good.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.s", os.path.join(datadir, "unicode", "bad.s")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "commenting-out-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "early-return-new.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c",
                        os.path.join(datadir, "unicode", "stretched-string-new.c"),
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
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


# The following tests handle the unicode= security rules that may be present
class UnicodeBadCSourceSkipSecRuleSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("badskip.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        # the known bad file is skipped here, so unicode result should be 'OK'
        self.inspection = "unicode"
        self.result = "OK"


class UnicodeBadCSourceSkipSecRuleKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("badskip.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        # the known bad file is skipped here, so unicode result should be 'OK'
        self.inspection = "unicode"
        self.result = "OK"


class UnicodeBadCSourceCompareSkipSecRuleSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("badskip.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("badskip.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        # the known bad file is skipped here, so unicode result should be 'OK'
        self.inspection = "unicode"
        self.result = "OK"


class UnicodeBadCSourceCompareSkipSecRuleKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("badskip.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("badskip.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        # the known bad file is skipped here, so unicode result should be 'OK'
        self.inspection = "unicode"
        self.result = "OK"


class UnicodeBadCSourceInformSecRuleSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("badinform.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Not Waivable"
        self.result = "INFO"


class UnicodeBadCSourceInformSecRuleKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("badinform.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Not Waivable"
        self.result = "INFO"


class UnicodeBadCSourceCompareInformSecRuleSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("badinform.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("badinform.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Not Waivable"
        self.result = "INFO"


class UnicodeBadCSourceCompareInformSecRuleKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("badinform.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("badinform.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Not Waivable"
        self.result = "INFO"


class UnicodeBadCSourceVerifySecRuleSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("badverify.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class UnicodeBadCSourceVerifySecRuleKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("badverify.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class UnicodeBadCSourceCompareVerifySecRuleSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("badverify.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("badverify.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class UnicodeBadCSourceCompareVerifySecRuleKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("badverify.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("badverify.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "VERIFY"


class UnicodeBadCSourceFailSecRuleSRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("badfail.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceFailSecRuleKoji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile("badfail.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceCompareFailSecRuleSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("badfail.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("badfail.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


class UnicodeBadCSourceCompareFailSecRuleKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known bad source files directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile("badfail.c", os.path.join(datadir, "unicode", "bad.c"))
        )
        self.after_rpm.add_source(
            ProvidedSourceFile("badfail.c", os.path.join(datadir, "unicode", "bad.c"))
        )

        self.inspection = "unicode"
        self.waiver_auth = "Security"
        self.result = "BAD"


# The following test is for a reported bug that showed a
# bi-directional character present in 'table.txt', which was
# incorrect.  All of these tests should pass as 'OK'.
class UnicodeGoodIssue1418SRPM(TestSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodIssue1418RPMs(TestRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodIssue1418Koji(TestKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodIssue1418CompareSRPM(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodIssue1418CompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )

        self.inspection = "unicode"
        self.result = "OK"


class UnicodeGoodIssue1418CompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        # this creates enough of a spec file to appease rpmbuild
        self.before_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )
        self.after_rpm.add_installed_file(
            "/usr/bin/status",
            ProvidedSourceFile(
                "status.sh", os.path.join(datadir, "unicode", "status.sh")
            ),
            mode=755,
        )

        # drop our known good source file directly in the SRPM
        self.before_rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )
        self.after_rpm.add_source(
            ProvidedSourceFile(
                "table.txt", os.path.join(datadir, "unicode", "table.txt")
            )
        )

        self.inspection = "unicode"
        self.result = "OK"


# The following test check that "ignore" rule for bad files work.
# This also tests that filepaths from manual prepped archives got the
# unpack_base prefix stripped.
class UnicodeBadIgnoredCSourceArchiveSRPM(TestSRPM):
    def configFile(self):
        super().configFile()

        # Setting self.extra_cfg['unicode']['ignore'] in setUp() would overwrite
        # the default 'unicode' config from super().configFile(). Hence we need
        # to allow the parent class method to create the self.conffile, then
        # load it here and update the part that we are interested in.
        with open(self.conffile, "r") as instream:
            cfg = yaml.full_load(instream)

        cfg["unicode"]["ignore"] = ["%s-*/bad.c" % (BEFORE_NAME)]

        with open(self.conffile, "w") as outstream:
            outstream.write(yaml.dump(cfg).replace("- ", "  - "))

    def setUp(self):
        super().setUp()
        self.rpm.header += "\n%define debug_package %{nil}\n"
        self.rpm.add_source(
            rpmfluff.GeneratedTarball(
                "%s-%s.tar.gz" % (AFTER_NAME, AFTER_VER),
                "%s-%s" % (AFTER_NAME, AFTER_VER),
                [
                    ProvidedSourceFile(
                        "Makefile", os.path.join(datadir, "unicode", "Makefile")
                    ),
                    ProvidedSourceFile(
                        "bad.c", os.path.join(datadir, "unicode", "bad.c")
                    ),
                    ProvidedSourceFile(
                        "status.sh", os.path.join(datadir, "unicode", "status.sh")
                    ),
                ],
            )
        )
        self.rpm.section_prep = "%setup"
        self.rpm.section_build += "make\n"
        self.rpm.section_install += "make install DESTDIR=%{buildroot}\n"
        sub = self.rpm.get_subpackage(None)
        sub.section_files += "/usr/bin/status\n"

        self.inspection = "unicode"
        # The expected result is OK because the bad.c file is explicitly ignored
        self.result = "OK"
