/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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

#include "rpminspect.h"
#include "inspect.h"

/*
 * Debugging mode toggle, set at runtime.
 */
bool debug_mode = false;

/*
 * Ensure the array of inspections is only defined once.
 */

struct inspect inspections[] = {
    { INSPECT_LICENSE, "license", true, &inspect_license },
    { INSPECT_EMPTYRPM, "emptyrpm", true, &inspect_emptyrpm },
    { INSPECT_METADATA, "metadata", true, &inspect_metadata },
    { INSPECT_MANPAGE, "manpage", true, &inspect_manpage },
    { INSPECT_XML, "xml", true, &inspect_xml },
    { INSPECT_ELF, "elf", true, &inspect_elf },
    { INSPECT_DESKTOP, "desktop", true, &inspect_desktop },
    { INSPECT_DISTTAG, "disttag", true, &inspect_disttag },
    { INSPECT_SPECNAME, "specname", true, &inspect_specname },
    { INSPECT_MODULARITY, "modularity", true, &inspect_modularity },
    { INSPECT_JAVABYTECODE, "javabytecode", true, &inspect_javabytecode },
    { INSPECT_CHANGEDFILES, "changedfiles", false, &inspect_changedfiles },
    { INSPECT_REMOVEDFILES, "removedfiles", false, &inspect_removedfiles },
    { INSPECT_ADDEDFILES, "addedfiles", false, &inspect_addedfiles },
    { INSPECT_UPSTREAM, "upstream", false, &inspect_upstream },
    { INSPECT_OWNERSHIP, "ownership", true, &inspect_ownership },
    { INSPECT_SHELLSYNTAX, "shellsyntax", true, &inspect_shellsyntax },
    { INSPECT_ANNOCHECK, "annocheck", true, &inspect_annocheck },
    { INSPECT_DT_NEEDED, "DT_NEEDED", false, &inspect_dt_needed },
    { INSPECT_FILESIZE, "filesize", false, &inspect_filesize },
    { INSPECT_PERMISSIONS, "permissions", false, &inspect_permissions },
    { INSPECT_CAPABILITIES, "capabilities", true, &inspect_capabilities },
    { INSPECT_KMOD, "kmod", false, &inspect_kmod },
    { INSPECT_ARCH, "arch", false, &inspect_arch },
    { INSPECT_SUBPACKAGES, "subpackages", false, &inspect_subpackages },
    { INSPECT_CHANGELOG, "changelog", false, &inspect_changelog },

    /*
     * { INSPECT_TYPE (add to inspect.h),
     *   "short name",
     *   bool--true if for single build, false is before&after required,
     *   &function_pointer,
     *   "Long description string" },
     */

    { 0, NULL, false, NULL }
};

/*
 * Inspect each "after" file in each peer of an inspection.
 * If the foreach_peer_file_func returns false for any file, the
 * result will be false.  foreach_peer_file_func is run on each file
 * even if an earlier file fails. This allows for multiple errors to
 * be collected for a single inspection.
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
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
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

/*
 * Return the long description for the specified inspection.
 */
const char *inspection_desc(const uint64_t inspection)
{
    switch (inspection) {
        case INSPECT_LICENSE:
            return _("Verify the string specified in the License tag of the RPM metadata describes permissible software licenses as defined by the license database. Also checks to see if the License tag contains any unprofessional words as defined in the configuration file.");
        case INSPECT_EMPTYRPM:
            return _("Check all binary RPMs in the before and after builds for any empty payloads. Packages that lost payload data from the before build to the after build are reported as well as any packages in the after build that exist but have no payload data.");
        case INSPECT_METADATA:
            return _("Perform some RPM header checks. First, check that the Vendor contains the expected string as defined in the configuration file. Second, check that the build host is in the expected subdomain as defined in the configuration file. Third, check the Summary string for any unprofessional words. Fourth, check the Description for any unprofessional words. Lastly, if there is a before build specified, check for differences between the before and after build values of the previous RPM header values and report them.");
        case INSPECT_MANPAGE:
            return _("Perform some checks on man pages in the RPM payload. First, check that each man page is compressed. Second, check that each man page contains valid content. Lastly, check that each man page is installed to the correct path.");
        case INSPECT_XML:
            return _("Check that XML files included in the RPM payload are well-formed.");
        case INSPECT_ELF:
            return _("Perform several checks on ELF files. First, check that ELF objects do not contain an executable stack. Second, check that ELF objects do not contain text relocations. When comparing builds, check that the ELF objects in the after build did not lose a PT_GNU_RELRO segment. When comparing builds, check that the ELF objects in the after build did not lose -D_FORTIFY_SOURCE. Lastly, if there is a list of forbidden library functions, make sure nothing uses them.");
        case INSPECT_DESKTOP:
            return _("Perform syntax and file reference checks on *.desktop files. Syntax errors and invalid file references are reported as errors.");
        case INSPECT_DISTTAG:
            return _("Check that the 'Release' tag in the RPM spec file includes the %{?dist} directive.");
        case INSPECT_SPECNAME:
            return _("Ensure the spec file name conforms to the NAME.spec naming format.");
        case INSPECT_MODULARITY:
            return _("Ensure compliance with modularity build and packaging policies (only valid for module builds, no-op otherwise).");
        case INSPECT_JAVABYTECODE:
            return _("Check minimum required Java bytecode version in class files, report bytecode version changes between builds, and report if bytecode versions are exceeded.  The bytecode version is vendor specific to releases and defined in the configuration file.");
        case INSPECT_CHANGEDFILES:
            return _("Report changed files from the before build to the after build.  Certain file changes will raise additional warnings if the concern is more critical than just reporting changes (e.g., a suspected security impact).  Any gzip, bzip2, or xz compressed files will have their uncompressed content compared only, which will allow changes through in the compression level used.  Message catalog files (.mo) are unpacked and compared using diff(1).  Public C and C++ header files are preprocessed and compared using diff(1).  Any changes with diff output are included in the results.");
        case INSPECT_REMOVEDFILES:
            return _("Report removed files from the before build to the after build.  Shared libraries get additional reporting output as they may be unexpected dependency removals.  Files removed with a security path prefix generated special reporting in case a security review is required.  Source RPMs and debuginfo files are ignored by this inspection.");
        case INSPECT_ADDEDFILES:
            return _("Report added files from the before build to the after build.  Debuginfo files are ignored as are files that match the patterns defined in the configuration file.  Files added to security paths generate special reporting in case a security review is required.  New setuid and setgid files raise a security warning unless the file is in the whitelist.");
        case INSPECT_UPSTREAM:
            return _("Report Source archives defined in the RPM spec file changing content between the before and after build. If the source archives change and the package is on the version-whitelist, the change is reported as informational. Otherwise the change is reported as a rebase of the package and requires inspection.");
        case INSPECT_OWNERSHIP:
            return _("Report files and directories owned by unexpected users and groups. Check to make sure executables are owned by the correct user and group. If a before and after build have been specified, also report ownership changes.");
        case INSPECT_SHELLSYNTAX:
            return _("For all shell scripts in the build, perform a syntax check on it using the shell defined in its #! line (shell must also be listed in rpminspect.conf's shell setting). If the syntax check returns non-zero, report it to the user and return a combined stdout and stderr. If comparing two builds, perform the previous check but also report if a previously bad script is now passing the syntax check.");
        case INSPECT_ANNOCHECK:
            return _("Perform annocheck tests defined in the configuration file on all ELF files in the build.  A single build specified will perform an analysis only.  Two builds specified will compare the test results between the before and after build.  If no annocheck tests are defined in the configuration file, this inspection is skipped.");
        case INSPECT_DT_NEEDED:
            return _("Compare DT_NEEDED entries in dynamic ELF executables and shared libraries between the before and after build and report changes.");
        case INSPECT_FILESIZE:
            return _("Report file size changes between builds.  If empty files became non-empty or non-empty files became empty, report those as results needing verification.  Report file change percentages as info-only.");
        case INSPECT_PERMISSIONS:
            return _("Report stat(2) mode changes between builds.  Checks against the stat-whitelist for the product release specified or determined.  Any setuid or setgid changes will raise a message requiring Security Team review.");
        case INSPECT_CAPABILITIES:
            return _("Report capabilities(7) changes between builds.  Checks against the capabilities whitelist for the product release specified or determined.  Any capabilities changes not whitelisted will raise a message requiring Security Team review.");
        case INSPECT_KMOD:
            return _("Report kernel module parameter, dependency, PCI ID, or symbol differences between builds.  Added and removed parameters are reported and if the package version is unchanged, these messages are reported as failures.  The same is true module dependencies, PCI IDs, and symbols");
        case INSPECT_ARCH:
            return _("Report RPM architectures that appear and disappear between the before and after builds.");
        case INSPECT_SUBPACKAGES:
            return _("Report RPM subpackages that appear and disappear between the before and after builds.");
        case INSPECT_CHANGELOG:
            return _("Ensure packages contain an entry in the %changelog for the version built.  Reports any other differences in the existing changelog between builds and that the new entry contains new text entries.");
        default:
            return NULL;
    }
}
