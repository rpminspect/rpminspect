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

    { INSPECT_DESKTOP,
      "desktop",
      false,
      &inspect_desktop,
      "Perform syntax and file reference checks on *.desktop files. Syntax errors and invalid file references are reported as errors." },

    { INSPECT_DISTTAG,
      "disttag",
      true,
      &inspect_disttag,
      "Check that the 'Release' tag in the RPM spec file includes the %{?dist} directive." },

    { INSPECT_SPECNAME,
      "specname",
      true,
      &inspect_specname,
      "Ensure the spec file name conforms to the NAME.spec naming format." },

    { INSPECT_MODULARITY,
      "modularity",
      true,
      &inspect_modularity,
      "Ensure compliance with modularity build and packaging policies (only valid for module builds, no-op otherwise)." },

    { INSPECT_JAVABYTECODE,
      "javabytecode",
      true,
      &inspect_javabytecode,
      "Check minimum required Java bytecode version in class files, report bytecode version changes between builds, and report if bytecode versions are exceeded.  The bytecode version is vendor specific to releases and defined in the configuration file." },

    { INSPECT_CHANGEDFILES,
      "changedfiles",
      false,
      &inspect_changedfiles,
      "Report changed files from the before build to the after build.  Certain file changes will raise additional warnings if the concern is more critical than just reporting changes (e.g., a suspected security impact).  Any gzip, bzip2, or xz compressed files will have their uncompressed content compared only, which will allow changes through in the compression level used.  Message catalog files (.mo) are unpacked and compared using diff(1).  Public C and C++ header files are preprocessed and compared using diff(1).  Any changes with diff output are included in the results." },

    { INSPECT_REMOVEDFILES,
      "removedfiles",
      false,
      &inspect_removedfiles,
      "Report removed files from the before build to the after build.  Shared libraries get additional reporting output as they may be unexpected dependency removals.  Files removed with a security path prefix generated special reporting in case a security review is required.  Source RPMs and debuginfo files are ignored by this inspection." },

    { INSPECT_ADDEDFILES,
      "addedfiles",
      false,
      &inspect_addedfiles,
      "Report added files from the before build to the after build.  Debuginfo files are ignored as are files that match the patterns defined in the configuration file.  Files added to security paths generate special reporting in case a security review is required.  New setuid and setgid files raise a security warning unless the file is in the whitelist." },

    { INSPECT_UPSTREAM,
      "upstream",
      false,
      &inspect_upstream,
      "Report Source archives defined in the RPM spec file changing content between the before and after build. If the source archives change and the package is on the version-whitelist, the change is reported as informational. Otherwise the change is reported as a rebase of the package and requires inspection." },

    { INSPECT_OWNERSHIP,
      "ownership",
      true,
      &inspect_ownership,
      "Report files and directories owned by unexpected users and groups. Check to make sure executables are owned by the correct user and group. If a before and after build have been specified, also report ownership changes." },

    /*
     * { INSPECT_TYPE (add to inspect.h),
     *   "short name",
     *   bool--true if for single build, false is before&after required,
     *   &function_pointer,
     *   "Long description string" },
     */

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
        if (TAILQ_EMPTY(peer->after_files)) {
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
