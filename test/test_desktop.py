#
# Copyright (C) 2019-2020  Red Hat, Inc.
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
import rpmfluff
from baseclass import *

good_desktop_file = """[Desktop Entry]
Name=Hello World
Name[de]=Hello World
Name[pl]=Hello World
Comment=Lorem ipsum dolor sit amet
Comment[de]=Lorem ipsum dolor sit amet
GenericName=Greeting Application
GenericName[de]=Greeting Application
GenericName[pl]=Greeting Application
Exec=hello-world
Terminal=false
Type=Application
Icon=hello-world
Categories=Graphics;
StartupNotify=true
MimeType=application/x-extension-fcstd;
"""

bad_desktop_file = """[Desktop Entry]
Name=Hello World
Name[de]=Hello World
Name[pl]=Hello World
Comment=Lorem ipsum dolor sit amet
Comment[de]=Lorem ipsum dolor sit amet
GenericName=Greeting Application
GenericName[de]=Greeting Application
GenericName[pl]=Greeting Application
Exec=hello-world
Teeeeeerminal=false
Type=Application
Icon=hello-world
Catesdfghdrfhwesgories=Graphics;
StartupNotify=true
MimeType=application/x-extension-fcstd;
"""

good_exec_args_desktop_file = """[Desktop Entry]
Name=Hello World
Name[de]=Hello World
Name[pl]=Hello World
Comment=Lorem ipsum dolor sit amet
Comment[de]=Lorem ipsum dolor sit amet
GenericName=Greeting Application
GenericName[de]=Greeting Application
GenericName[pl]=Greeting Application
Exec=hello-world %F
Terminal=false
Type=Application
Icon=hello-world
Categories=Graphics;
StartupNotify=true
MimeType=application/x-extension-fcstd;
"""

# Valid desktop file passes in RPM (OK)
class ValidDesktopFileRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "OK"


# Valid desktop file passes in Koji build (OK)
class ValidDesktopFileKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "OK"


# Valid desktop file passes in RPM compare (OK)
class ValidDesktopFileCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "OK"


# Valid desktop file passes in Koji build compare (OK)
class ValidDesktopFileCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "OK"


# Desktop file with missing icon in RPM (VERIFY)
class DesktopFileMissingIconRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file with missing icon in RPM compare (VERIFY)
class DesktopFileMissingIconCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file with missing icon in Koji build (VERIFY)
class DesktopFileMissingIconKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file with missing icon in Koji build compare (VERIFY)
class DesktopFileMissingIconCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Invalid desktop file fails desktop-file-validate in RPM (BAD)
class DesktopFileValidateFailsRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", bad_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Invalid desktop file fails desktop-file-validate in Koji build (BAD)
class DesktopFileValidateFailsKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", bad_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Invalid desktop file fails desktop-file-validate in RPM compare (BAD)
class DesktopFileValidateFailsCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", bad_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", bad_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Invalid desktop file fails desktop-file-validate in Koji build compare (BAD)
class DesktopFileValidateFailsCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", bad_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", bad_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# Desktop file with Exec with arguments in RPM (OK)
class DesktopFileExecArgsRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_exec_args_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "OK"


# Desktop file with Exec with arguments in Koji build (OK)
class DesktopFileExecArgsKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_exec_args_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "OK"


# Desktop file with Exec with arguments in RPM compare (OK)
class DesktopFileExecArgsCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_exec_args_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_exec_args_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "OK"


# Desktop file with Exec with arguments in Koji build compare (OK)
class DesktopFileExecArgsCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_exec_args_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_exec_args_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "OK"


# Desktop file without world readable Icon in RPM (VERIFY)
class DesktopFileNotWorldReadableIconRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
            mode="0600",
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file without world readable Icon in Koji build (VERIFY)
class DesktopFileNotWorldReadableIconKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
            mode="0600",
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file without world readable Icon in RPM compare (VERIFY)
class DesktopFileNotWorldReadableIconCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
            mode="0600",
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
            mode="0600",
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file without world readable Icon in Koji build compare (VERIFY)
class DesktopFileNotWorldReadableIconCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.after_rpm.add_simple_compilation()

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
            mode="0600",
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
            mode="0600",
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file with invalid Exec file in RPM (VERIFY)
class DesktopFileInvalidExecRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file with invalid Exec file in Koji build (VERIFY)
class DesktopFileInvalidExecRPMs(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file with invalid Exec file in RPM compare (VERIFY)
class DesktopFileInvalidExecCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file with invalid Exec file in Koji compare (VERIFY)
class DesktopFileInvalidExecCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file without world executable Exec file in RPM (VERIFY)
class DesktopFileWithoutWorldExecutableExecRPM(TestRPMs):
    def setUp(self):
        TestRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()
        self.rpm.add_mode("/usr/bin/hello-world", "0700")

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file without world executable Exec file in Koji build (VERIFY)
class DesktopFileWithoutWorldExecutableExecKoji(TestKoji):
    def setUp(self):
        TestKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.rpm.add_simple_compilation()
        self.rpm.add_mode("/usr/bin/hello-world", "0700")

        # Adds /usr/share/icons/hello-world.png
        self.rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file without world executable Exec file in RPM compare (VERIFY)
class DesktopFileWithoutWorldExecutableExecCompareRPM(TestCompareRPMs):
    def setUp(self):
        TestCompareRPMs.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.before_rpm.add_mode("/usr/bin/hello-world", "0700")
        self.after_rpm.add_simple_compilation()
        self.after_rpm.add_mode("/usr/bin/hello-world", "0700")

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"


# Desktop file without world executable Exec file in Koji build compare (VERIFY)
class DesktopFileWithoutWorldExecutableExecCompareKoji(TestCompareKoji):
    def setUp(self):
        TestCompareKoji.setUp(self)

        # Adds /usr/bin/hello-world
        self.before_rpm.add_simple_compilation()
        self.before_rpm.add_mode("/usr/bin/hello-world", "0700")
        self.after_rpm.add_simple_compilation()
        self.after_rpm.add_mode("/usr/bin/hello-world", "0700")

        # Adds /usr/share/icons/hello-world.png
        self.before_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/icons/hello-world.png",
            rpmfluff.GeneratedSourceFile("hello-world.png", rpmfluff.make_png()),
        )

        # Adds /usr/share/applications/hello-world.desktop
        self.before_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )
        self.after_rpm.add_installed_file(
            "/usr/share/applications/hello-world.desktop",
            rpmfluff.SourceFile("hello-world.desktop", good_desktop_file),
        )

        self.inspection = "desktop"
        self.label = "desktop-entry-files"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"
