/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Author(s):  David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <regex.h>
#include <stdlib.h>
#include <sys/queue.h>
#include "rpminspect.h"

static void _free_regex(regex_t *regex)
{
    if (regex == NULL) {
        return;
    }

    regfree(regex);
    free(regex);
}

/*
 * Free a struct rpminspect.  Called by applications using
 * librpminspect before they exit.
 */
void free_rpminspect(struct rpminspect *ri) {
    if (ri == NULL) {
        return;
    }

    free(ri->cfgfile);
    free(ri->workdir);
    free(ri->licensedb);
    free(ri->kojihub);
    free(ri->kojidownload);
    free(ri->worksubdir);

    list_free(ri->badwords, free);

    _free_regex(ri->elf_path_include);
    _free_regex(ri->elf_path_exclude);
    _free_regex(ri->manpage_path_include);
    _free_regex(ri->manpage_path_exclude);
    _free_regex(ri->xml_path_include);
    _free_regex(ri->xml_path_exclude);

    free(ri->vendor);
    free(ri->buildhost_subdomain);
    free(ri->before);
    free(ri->after);
    headerFree(ri->before_srpm_hdr);
    headerFree(ri->after_srpm_hdr);
    free(ri->before_srpm);
    free(ri->after_srpm);

    free_rpmpeer(ri->peers);

    free_results(ri->results);

    return;
}
