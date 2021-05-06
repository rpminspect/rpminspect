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

import yaml
import rpmfluff

from baseclass import TestSRPM, TestCompareSRPM

# test data
patch_file = """--- fortify.c.orig      2020-09-22 14:45:41.592625821 -0400
+++ fortify.c   2020-09-29 10:43:21.829954365 -0400
@@ -1,13 +1,20 @@
 #include <stdio.h>
+#include <stdlib.h>
 #include <string.h>
+#include <errno.h>
+#include <err.h>
+
 int main (int argc, char **argv)
 {
     /* https://access.redhat.com/blogs/766093/posts/1976213 */
     char buffer[5];
+
     printf("Buffer Contains: %s , Size Of Buffer is %lu\n",
            buffer, sizeof(buffer));
     strcpy(buffer, argv[1]);
+    assert(buffer);
     printf("Buffer Contains: %s , Size of Buffer is %lu\n",
            buffer, sizeof(buffer));
-    return 0;
+
+    return EXIT_SUCCESS;
 }
"""

patch_file_changed = """--- fortify.c.orig      2020-09-22 14:45:41.592625821 -0400
+++ fortify.c   2020-09-29 10:51:05.922483632 -0400
@@ -1,13 +1,18 @@
 #include <stdio.h>
+#include <stdlib.h>
 #include <string.h>
+
 int main (int argc, char **argv)
 {
     /* https://access.redhat.com/blogs/766093/posts/1976213 */
     char buffer[5];
+
     printf("Buffer Contains: %s , Size Of Buffer is %lu\n",
            buffer, sizeof(buffer));
     strcpy(buffer, argv[1]);
+    assert(buffer);
     printf("Buffer Contains: %s , Size of Buffer is %lu\n",
            buffer, sizeof(buffer));
-    return 0;
+
+    return EXIT_SUCCESS;
 }
"""

patch_file_threshold = """diff --git a/test/data/forbidden-ipv6.c b/test/data/forbidden-ipv6.c
index 12295de..dd2a9e9 100644
--- a/test/data/forbidden-ipv6.c
+++ b/test/data/forbidden-ipv6.c
@@ -1,11 +1,18 @@
 #include <stdio.h>
+#include <stdlib.h>
+#include <assert.h>
+#include <errno.h>
+#include <err.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
+
 int main (int argc, char **argv)
 {
     const char *cp = "forbidden";
     struct in_addr in;
+
     int ret = inet_aton(cp, &in);
-    return 0;
+
+    return EXIT_SUCCESS;
 }
diff --git a/test/data/fortify.c b/test/data/fortify.c
index d81ab48..d6f5c88 100644
--- a/test/data/fortify.c
+++ b/test/data/fortify.c
@@ -1,13 +1,18 @@
 #include <stdio.h>
+#include <stdlib.h>
 #include <string.h>
+
 int main (int argc, char **argv)
 {
     /* https://access.redhat.com/blogs/766093/posts/1976213 */
     char buffer[5];
+
     printf("Buffer Contains: %s , Size Of Buffer is %lu\n",
            buffer, sizeof(buffer));
     strcpy(buffer, argv[1]);
+    assert(buffer);
     printf("Buffer Contains: %s , Size of Buffer is %lu\n",
            buffer, sizeof(buffer));
-    return 0;
+
+    return EXIT_SUCCESS;
 }
diff --git a/test/data/mathlib.c b/test/data/mathlib.c
index ffc4206..12e7808 100644
--- a/test/data/mathlib.c
+++ b/test/data/mathlib.c
@@ -1,19 +1,19 @@
 double add(double x, double y)
 {
-    return x + y;
+    return (x + y);
 }
 
 double subtract(double x, double y)
 {
-    return x - y;
+    return (x - y);
 }
 
 double multiply(double x, double y)
 {
-    return x * y;
+    return (x * y);
 }
 
 double divide(double x, double y)
 {
-    return x / y;
+    return (x / y);
 }
"""  # noqa: W293


# patch under 4 bytes (BAD)
class PatchUnderFourBytes(TestSRPM):
    def setUp(self):
        super().setUp()

        # rpmbuild will usually catch malformed patches, but only if
        # they are applied in the spec file.  rpminspect will catch
        # malformed patches that are defined but not used.  The second
        # parameter of add_patch() is a boolean indicating whether or
        # not to apply the patch in the spec file, so disable that for
        # this test to see if rpminspect works.
        self.rpm.add_patch(rpmfluff.SourceFile("some.patch", "Qx"), False)

        self.inspection = "patches"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


class PatchUnderFourBytesCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # rpmbuild will usually catch malformed patches, but only if
        # they are applied in the spec file.  rpminspect will catch
        # malformed patches that are defined but not used.  The second
        # parameter of add_patch() is a boolean indicating whether or
        # not to apply the patch in the spec file, so disable that for
        # this test to see if rpminspect works.
        self.before_rpm.add_patch(rpmfluff.SourceFile("some.patch", "Qx"), False)
        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", "Qx"), False)

        self.inspection = "patches"
        self.result = "BAD"
        self.waiver_auth = "Anyone"


# patch changed (INFO)
class PatchChangedNotRebaseCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.before_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_changed), False
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


class PatchChangedInRebaseCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.before_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_changed), False
        )

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch added not a rebase (INFO)
class PatchAddedNotRebaseCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch added in a rebase (INFO)
class PatchAddedInRebaseCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp(rebase=True)

        self.after_rpm.add_patch(rpmfluff.SourceFile("some.patch", patch_file), False)

        self.inspection = "patches"
        self.result = "INFO"
        self.waiver_auth = "Not Waivable"


# patch touches too many files (VERIFY)
class PatchTouchesTooManyFiles(TestSRPM):
    def setUp(self):
        super().setUp()

        # add the large patch
        self.rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_threshold), False
        )

        self.inspection = "patches"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["patches"]["file_count_threshold"] = "2"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class PatchTouchesTooManyFilesCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add the large patch
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_threshold), False
        )

        self.inspection = "patches"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["patches"]["file_count_threshold"] = "2"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


# patch touches too many lines (VERIFY)
class PatchTouchesTooManyLines(TestSRPM):
    def setUp(self):
        super().setUp()

        # add the large patch
        self.rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_threshold), False
        )

        self.inspection = "patches"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["patches"]["line_count_threshold"] = "3"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()


class PatchTouchesTooManyLinesCompare(TestCompareSRPM):
    def setUp(self):
        super().setUp()

        # add the large patch
        self.after_rpm.add_patch(
            rpmfluff.SourceFile("some.patch", patch_file_threshold), False
        )

        self.inspection = "patches"
        self.result = "VERIFY"
        self.waiver_auth = "Anyone"

    def configFile(self):
        super().configFile()

        # modify the threshold for the test run (set it to two files)
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["patches"]["line_count_threshold"] = "3"

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()
