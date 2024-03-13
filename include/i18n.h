/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _LIBRPMINSPECT_I18N_H
#define _LIBRPMINSPECT_I18N_H

#ifdef GETTEXT_DOMAIN
#include <libintl.h>
#include <locale.h>

#define _(MSGID) gettext((MSGID))
#define N_(MSGID, MSGID_PLURAL, N) ngettext((MSGID), (MSGID_PLURAL), (N))
#else
#define _(MSGID) (MSGID)
#define N_(MSGID, MSGID_PLURAL, N) ((MSGID_PLURAL))
#endif

#endif /* _LIBRPMINSPECT_I18N_H */

#ifdef __cplusplus
}
#endif
