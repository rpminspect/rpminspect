#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import subprocess
import yaml

from baseclass import RequiresRpminspect


# Compare config file parsing
class RpminspectCfg(RequiresRpminspect):
    def runTest(self):
        RequiresRpminspect.configFile(self)

        yaml = os.environ["RPMINSPECT_YAML"]
        dson = yaml[:-4] + "dson"
        json = yaml[:-4] + "json"

        p = subprocess.Popen(
            [self.rpminspect, "-Dc", yaml],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        yaml_out, err = p.communicate()
        if p.returncode:
            print(yaml_out)
        self.assertEqual(p.returncode, 0)

        p = subprocess.Popen(
            [self.rpminspect, "-Dc", json],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        json_out, err = p.communicate()
        if p.returncode:
            print(json_out)
        self.assertEqual(p.returncode, 0)

        p = subprocess.Popen(
            [self.rpminspect, "-Dc", dson],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        dson_out, err = p.communicate()
        if p.returncode:
            print(dson_out)
        self.assertEqual(p.returncode, 0)

        self.assertEqual(yaml_out, json_out)
        self.assertEqual(yaml_out, dson_out)


# YAML format cruft
class RpminspectYamlHate(RequiresRpminspect):
    def configFile(self):
        super().configFile()

        # make a copy to run against
        self.conffile_old = self.conffile + "_old"
        outf = open(self.conffile_old, "w")
        inf = open(self.conffile)
        outf.write(inf.read())
        outf.close()
        inf.close()

        # customize our config file to check some deprecated things
        instream = open(self.conffile, "r")
        cfg = yaml.full_load(instream)
        instream.close()

        cfg["badfuncs"] = cfg["badfuncs"]["forbidden"]
        cfg["annocheck"] = cfg["annocheck"]["jobs"]
        cfg["javabytecode"] = [cfg["javabytecode"]]

        outstream = open(self.conffile, "w")
        outstream.write(yaml.dump(cfg).replace("- ", "  - "))
        outstream.close()

    def tearDown(self):
        super().tearDown()
        if not os.getenv("KEEP"):
            os.unlink(self.conffile_old)

    def runTest(self):
        self.configFile()

        p = subprocess.Popen(
            [self.rpminspect, "-Dc", self.conffile_old],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        normal, err = p.communicate()
        if p.returncode:
            print(normal)
        self.assertEqual(p.returncode, 0)

        p = subprocess.Popen(
            [self.rpminspect, "-Dc", self.conffile],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        retro, err = p.communicate()
        if p.returncode:
            print(retro)
        self.assertEqual(p.returncode, 0)

        self.assertEqual(normal, retro)
