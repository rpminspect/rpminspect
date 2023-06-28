#
# Copyright The rpminspect Project Authors
# SPDX-License-Identifier: GPL-3.0-or-later
#

import os
import unittest
from baseclass import TestModule, TestCompareModules

have_modularitylabel = False
if os.system("grep -q MODULARITYLABEL /usr/include/rpm/rpmtag.h 2>/dev/null") == 0:
    have_modularitylabel = True


# Module has ModularityLabel defined (OK)
class ModuleHasModularityLabel(TestModule):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(modularitylabel=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "required"

        self.inspection = "modularity"
        self.result = "OK"


class ModulesHaveModularityLabel(TestCompareModules):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(modularitylabel=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "required"

        self.inspection = "modularity"
        self.result = "OK"


# Module does not have ModularityLabel defined (BAD)
class ModuleLacksModularityLabel(TestModule):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(modularitylabel=False)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "required"

        self.inspection = "modularity"
        self.result = "BAD"
        self.waiverauth = "Anyone"


class ModulesLacksModularityLabel(TestCompareModules):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(modularitylabel=False)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "required"

        self.inspection = "modularity"
        self.result = "BAD"
        self.waiverauth = "Anyone"


# Module has static_context and rules require it (OK)
class ModuleHasStaticContext(TestModule):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(static_context=True)

        self.inspection = "modularity"
        self.result = "OK"


class ModulesHaveStaticContext(TestCompareModules):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(static_context=True)

        self.inspection = "modularity"
        self.result = "OK"


# Module has static_context and rules recommend it (OK)
class ModuleHasStaticContextAndRecommended(TestModule):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(static_context=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "recommend"

        self.inspection = "modularity"
        self.result = "OK"


class ModulesHaveStaticContextAndRecommended(TestCompareModules):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(static_context=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "recommend"

        self.inspection = "modularity"
        self.result = "OK"


# Module has static_context and rules forbid it (VERIFY)
class ModuleHasStaticContextAndForbidden(TestModule):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(static_context=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "forbidden"

        self.inspection = "modularity"
        self.result = "VERIFY"
        self.waiverauth = "Anyone"


class ModulesHaveStaticContextAndForbidden(TestCompareModules):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(static_context=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["static_context"] = "forbidden"

        self.inspection = "modularity"
        self.result = "VERIFY"
        self.waiverauth = "Anyone"


# Module has static_context and rules do not exist (INFO)
class ModuleHasStaticContextWithNoRule(TestModule):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(static_context=True)

        self.inspection = "modularity"
        self.result = "OK"


class ModulesHaveStaticContextWithNoRule(TestCompareModules):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(static_context=True)

        self.inspection = "modularity"
        self.result = "OK"


# Module has correct Release tag value (OK)
class ModuleHasReleaseTagSubstring(TestModule):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(release_substring=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["release_regexp"] = {}
        self.extra_cfg["modularity"]["release_regexp"][
            "GENERIC"
        ] = "\+module\.[a-z0-9]+\.[0-9]+\.[0-9]+"  # noqa: W605

        self.inspection = "modularity"
        self.result = "OK"
        self.waiverauth = "Anyone"


class ModulesHaveReleaseTagSubstring(TestCompareModules):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(release_substring=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["release_regexp"] = {}
        self.extra_cfg["modularity"]["release_regexp"][
            "GENERIC"
        ] = "\+module\.[a-z0-9]+\.[0-9]+\.[0-9]+"  # noqa: W605

        self.inspection = "modularity"
        self.result = "OK"
        self.waiverauth = "Anyone"


# Module has non-conformant Release tag value (BAD)
class ModuleHasBadReleaseTagSubstring(TestModule):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(release_substring=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["release_regexp"] = {}
        self.extra_cfg["modularity"]["release_regexp"]["GENERIC"] = "foobar"

        self.inspection = "modularity"
        self.result = "BAD"
        self.waiverauth = "Anyone"


class ModulesHaveBadReleaseTagSubstring(TestCompareModules):
    @unittest.skipUnless(
        have_modularitylabel, "rpm lacks RPMTAG_MODULARITYLABEL support"
    )
    def setUp(self):
        super().setUp(release_substring=True)

        self.extra_cfg = {}
        self.extra_cfg["modularity"] = {}
        self.extra_cfg["modularity"]["release_regexp"] = {}
        self.extra_cfg["modularity"]["release_regexp"]["GENERIC"] = "foobar"

        self.inspection = "modularity"
        self.result = "BAD"
        self.waiverauth = "Anyone"
