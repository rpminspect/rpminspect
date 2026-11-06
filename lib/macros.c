/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <ctype.h>
#include <regex.h>
#include <assert.h>
#include <err.h>
#include <rpm/rpmfileutil.h>
#include <rpm/rpmmacro.h>
#include "queue.h"
#include "rpminspect.h"

/**
 * Initialize macros for use by librpm calls.  The caller needs to
 * also call rpmFreeMacros() during cleanup as well.  This function
 * can be safely called multiple times as it is guarded by the
 * macros_loaded boolean.  reset_macros() clears that boolean and
 * calls this function again after each spec file read.
 */
void load_macros(struct rpminspect *ri)
{
    char *mf = NULL;
    char *macropath = NULL;

    assert(ri != NULL);

    if (ri->macros_loaded) {
        return;
    }

    if (ri->macrofiles) {
        macropath = list_to_string(ri->macrofiles, ":");
        mf = rpmGetPath(macropath, NULL);
        rpmInitMacros(rpmGlobalMacroContext, mf);
        free(macropath);
        free(mf);
    }

    ri->macros_loaded = true;
    return;
}

/**
 * Reset librpm macros to the state set up by init_librpm() and
 * load_macros().  rpmSpecParse() leaves macros such as %global
 * defined after it returns, and they carry over into the next spec
 * file read.  This also drops any macros the caller defined, such as
 * %dist.
 */
void reset_macros(struct rpminspect *ri)
{
    if (ri == NULL) {
        return;
    }

    ri->librpm_initialized = false;

    if (init_librpm(ri) != RPMRC_OK) {
        warnx(_("*** unable to read RPM configuration"));
    }

    ri->macros_loaded = false;
    load_macros(ri);
    return;
}
