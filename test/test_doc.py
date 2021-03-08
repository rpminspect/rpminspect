#
# Copyright © 2020 Red Hat, Inc.
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

import rpmfluff

from baseclass import TestCompareRPMs, TestCompareKoji
from baseclass import AFTER_NAME

readme = """This is a README file.
With multiple lines.
And one last line.
"""

readme_with_content_changes = """This is a README file.
With multiple lines.
And one last line.
And even an additional last line.
"""

readme_with_whitespace_changes = """This is a README file.
With multiple lines.             
And one last line.
"""  # noqa: W291


# %doc becomes not a %doc file in non-rebase comparison (INFO)
class DocBecomesNonDocCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=False,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class DocBecomesNonDocCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=False,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# %doc becomes not a %doc file in rebase comparison (INFO)
class DocBecomesNonDocRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=False,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class DocBecomesNonDocRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=False,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# not a %doc becomes a %doc file in non-rebase comparison (INFO)
class NonDocBecomesDocCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=False,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


class NonDocBecomesDocCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=False,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# not a %doc becomes a %doc file in rebase comparison (INFO)
class NonDocBecomesDocRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=False,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class NonDocBecomesDocRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=False,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# whitespace %doc differences (OK)
class DocChangesWhitespaceCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme_with_whitespace_changes),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class DocChangesWhitespaceCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme_with_whitespace_changes),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class DocChangesWhitespaceRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme_with_whitespace_changes),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class DocChangesWhitespaceRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme_with_whitespace_changes),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# %doc content differences (INFO)
class DocChangesContentCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme_with_content_changes),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class DocChangesContentCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme_with_content_changes),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class DocChangesContentRebaseCompareRPMs(TestCompareRPMs):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme_with_content_changes),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class DocChangesContentRebaseCompareKoji(TestCompareKoji):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme),
            isDoc=True,
        )

        self.after_rpm.add_installed_file(
            "/opt/sfw/documentation/%s/README" % AFTER_NAME,
            rpmfluff.SourceFile("README", readme_with_content_changes),
            isDoc=True,
        )

        self.inspection = "doc"
        self.label = "doc"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"
