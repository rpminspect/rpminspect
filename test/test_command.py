#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import shutil
import subprocess
import tempfile

from baseclass import RequiresRpminspect


# Verify --help gives help output
class RpminspectHelp(RequiresRpminspect):
    def runTest(self):
        super().configFile()
        p = subprocess.Popen(
            [self.rpminspect, "--help"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
        (out, err) = p.communicate()
        self.assertEqual(p.returncode, 0)

        out = out.splitlines()
        t1 = list(filter(lambda x: x.startswith(b"Usage:"), out))
        t2 = list(filter(lambda x: x.startswith(b"Options:"), out))

        self.assertEqual(len(t1), 1)
        self.assertEqual(len(t2), 1)


# Verify rpminspect doesn't segfault on release-less args
class RpminspectSegv(RequiresRpminspect):
    def runTest(self):
        super().configFile()
        p = subprocess.Popen(
            [self.rpminspect, "42"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        p.communicate()
        self.assertNotEqual(p.returncode, 139)


# Verify rpminspect can handle an empty peer list
class RpminspectEmptyPeerList(RequiresRpminspect):
    def runTest(self):
        super().configFile()
        self.emptybuild = tempfile.mkdtemp()
        p = subprocess.Popen(
            [
                self.rpminspect,
                "-D",
                "-d",
                "-c",
                self.conffile,
                "-b",
                self.buildtype,
                "-F",
                "json",
                "-r",
                "GENERIC",
                "-o",
                self.outputfile,
                self.emptybuild,
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        (out, err) = p.communicate()
        self.assertEqual(p.returncode, 0)

    def tearDown(self):
        super().tearDown()
        shutil.rmtree(self.emptybuild, ignore_errors=True)
