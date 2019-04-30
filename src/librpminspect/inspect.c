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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/queue.h>
#include "inspect.h"

/*
 * Ensure the array of inspections is only defined once.
 */

struct inspect inspections[] = {
    { INSPECT_LICENSE,
      "license",
      true,
      &inspect_license,
      "Verify the string specified in the License tag of the RPM metadata describes permissible software licenses as defined by the license database. Also checks to see if the License tag contains any unprofessional words as defined in the configuration file." },

    { INSPECT_EMPTYRPM,
      "emptyrpm",
      false,
      &inspect_emptyrpm,
      "Check all binary RPMs in the before and after builds for any empty payloads. Packages that lost payload data from the before build to the after build are reported as well as any packages in the after build that exist but have no payload data." },

    { INSPECT_METADATA,
      "metadata",
      false,
      &inspect_metadata,
      "Perform some RPM header checks. First, check that the Vendor contains the expected string as defined in the configuration file. Second, check that the build host is in the expected subdomain as defined in the configuration file. Third, check the Summary string for any unprofessional words. Fourth, check the Description for any unprofessional words. Lastly, if there is a before build specified, check for differences between the before and after build values of the previous RPM header values and report them." },

    { INSPECT_MANPAGE,
      "manpage",
      true,
      &inspect_manpage,
      "Perform some checks on man pages in the RPM payload. First, check that each man page is compressed. Second, check that each man page contains valid content. Lastly, check that each man page is installed to the correct path." },

    { INSPECT_XML,
      "xml",
      true,
      &inspect_xml,
      "Check that XML files included in the RPM payload are well-formed." },

    { INSPECT_ELF,
      "elf",
      true,
      &inspect_elf,
      "Perform several checks on ELF files. First, check that ELF objects do not contain an executable stack. Second, check that ELF objects do not contain text relocations. When comparing builds, check that the ELF objects in the after build did not lose a PT_GNU_RELRO segment. Lastly, when comparing builds, check that the ELF objects in the after build did not lose -D_FORTIFY_SOURCE." },

    { 0, NULL, false, NULL, NULL }
};

/*
 * Inspect each "after" file in each peer of an inspection.
 * If the foreach_peer_file_func returns false for any file, the result will be false.
 * foreach_peer_file_func is run on each file even if an earlier file fails. This allows
 * for multiple errors to be collected for a single inspection.
 */
bool foreach_peer_file(struct rpminspect *ri, foreach_peer_file_func check_fn)
{
    rpmpeer_entry_t *peer;
    rpmfile_entry_t *file;
    bool result = true;

    assert(ri != NULL);
    assert(check_fn != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_files == NULL) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            if (!check_fn(ri, file)) {
                result = false;
            }
        }
    }

    return result;
}
