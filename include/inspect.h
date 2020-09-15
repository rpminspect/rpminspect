/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
 * Author(s):  David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file inspect.h
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2019-2020
 * @brief Test functions provided by librpminspect and result constants.
 * @copyright LGPL-3.0-or-later
 *
 * All inspection drivers use the same prototype.  They return a bool
 * and take one parameter:  a struct rpminspect pointer.  Inspection
 * drivers are named as "inspect_NAME" where NAME is the short name
 * specified in the inspections array (see inspect.c).  The source
 * code filename should also match the driver function name.
 *
 * Add any helper functions you need in order to keep the inspection
 * driver simple.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <libelf.h>
#include <libkmod.h>

#include "types.h"

#ifndef _LIBRPMINSPECT_INSPECT_H
#define _LIBRPMINSPECT_INSPECT_H

/**
 * @defgroup INSPECT General inspection utility functions.
 *
 * @{
 */

/**
 * @brief Iterate over each file in each package in a build.
 *
 * Inspect each "after" file in each peer of an inspection.  If the
 * foreach_peer_file_func returns false for any file, the result will
 * be false.  foreach_peer_file_func is run on each file even if an
 * earlier file fails. This allows for multiple errors to be collected
 * for a single inspection.
 *
 * @param ri Pointer to the struct rpminspect used for the program.
 * @param callback Callback function to iterate over each file.
 * @param use_ignore True to skip files that match entries in the
 *        ignore section of the configuration file, false otherwise.
 * @return True if the check_fn passed for each file, false otherwise.
 */
bool foreach_peer_file(struct rpminspect *ri, foreach_peer_file_func callback, bool use_ignore);

/**
 * @brief Return inspection description string given its ID.
 *
 * Return the long description for the specified inspection.
 *
 * @param id Inspection ID constant.
 */
const char *inspection_desc(const uint64_t id);

/** @} */

/**
 * @defgroup INSPECTIONS Individual inspection functions
 *
 * @{
 */

/**
 * @brief Perform the 'elf' inspection.
 *
 * Perform several checks on ELF files. First, check that ELF objects
 * do not contain an executable stack. Second, check that ELF objects
 * do not contain text relocations. When comparing builds, check that
 * the ELF objects in the after build did not lose a PT_GNU_RELRO
 * segment. When comparing builds, check that the ELF objects in the
 * after build did not lose -D_FORTIFY_SOURCE. Lastly, if there is a
 * list of forbidden library functions, make sure nothing uses them.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_elf(struct rpminspect *ri);

/**
 * @brief Perform the 'license' inspection.
 *
 * Verify the string specified in the License tag of the RPM metadata
 * describes permissible software licenses as defined by the license
 * database. Also checks to see if the License tag contains any
 * unprofessional words as defined in the configuration file.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_license(struct rpminspect *ri);

/**
 * @brief Perform the 'emptyrpm' inspection.
 *
 * Check all binary RPMs in the before and after builds for any empty
 * payloads. Packages that lost payload data from the before build to
 * the after build are reported as well as any packages in the after
 * build that exist but have no payload data.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_emptyrpm(struct rpminspect *ri);

/**
 * @brief Perform the 'xml' inspection.
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_xml(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_manpage(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_metadata(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_desktop(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_disttag(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_specname(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_modularity(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_javabytecode(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_changedfiles(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_movedfiles(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_removedfiles(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_addedfiles(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_upstream(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_ownership(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_shellsyntax(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_annocheck(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_dt_needed(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_filesize(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_permissions(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_capabilities(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_kmod(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_arch(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_subpackages(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_changelog(struct rpminspect *ri);

/**
 * @brief
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 */
bool inspect_pathmigration(struct rpminspect *ri);

/**
 * @brief Main driver for the 'lto' inspection.
 *
 * LTO (Link Time Optimization) bytecode is not stable from one
 * release of gcc to the next.  This inspection checks ELF .o and .a
 * files to ensure all LTO bytecode has been stripped.
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_lto(struct rpminspect *ri);

/**
 * @brief Main driver for the 'symlinks' inspection.
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_symlinks(struct rpminspect *ri);

/**
 * @brief Main driver for the 'lostpayload' inspection.
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_lostpayload(struct rpminspect *ri);

/**
 * @brief Main driver for the '%files' inspection.
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_files(struct rpminspect *ri);

/**
 * @brief Main driver for the 'types' inspection.
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_types(struct rpminspect *ri);

/**
 * @brief Main driver for the 'abidiff' inspection.
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_abidiff(struct rpminspect *ri);

/**
 * @brief Main driver for the 'kmidiff' inspection.
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_kmidiff(struct rpminspect *ri);

/**
 * @brief Main driver for the 'config' inspection.
 *
 *
 *
 * @param ri Pointer to the struct rpminspect for the program.
 * @return True if the inspection passed, false otherwise.
 */
bool inspect_config(struct rpminspect *ri);

/** @} */

/*
 * Inspections are referenced by flag.  These flags are set in bitfields
 * to indicate which ones we want to run.  When adding new ones, please
 * follow the existing convention.  Inspection names should be short but
 * descriptive.  Note that inspection names need to specified on the
 * command line.
 */

#define INSPECT_LICENSE                     (((uint64_t) 1) << 1)
#define INSPECT_EMPTYRPM                    (((uint64_t) 1) << 2)
#define INSPECT_METADATA                    (((uint64_t) 1) << 3)
#define INSPECT_MANPAGE                     (((uint64_t) 1) << 4)
#define INSPECT_XML                         (((uint64_t) 1) << 5)
#define INSPECT_ELF                         (((uint64_t) 1) << 6)
#define INSPECT_DESKTOP                     (((uint64_t) 1) << 7)
#define INSPECT_DISTTAG                     (((uint64_t) 1) << 8)
#define INSPECT_SPECNAME                    (((uint64_t) 1) << 9)
#define INSPECT_MODULARITY                  (((uint64_t) 1) << 10)
#define INSPECT_JAVABYTECODE                (((uint64_t) 1) << 11)
#define INSPECT_CHANGEDFILES                (((uint64_t) 1) << 12)
#define INSPECT_MOVEDFILES                  (((uint64_t) 1) << 13)
#define INSPECT_REMOVEDFILES                (((uint64_t) 1) << 14)
#define INSPECT_ADDEDFILES                  (((uint64_t) 1) << 15)
#define INSPECT_UPSTREAM                    (((uint64_t) 1) << 16)
#define INSPECT_OWNERSHIP                   (((uint64_t) 1) << 17)
#define INSPECT_SHELLSYNTAX                 (((uint64_t) 1) << 18)
#define INSPECT_ANNOCHECK                   (((uint64_t) 1) << 19)
#define INSPECT_DT_NEEDED                   (((uint64_t) 1) << 20)
#define INSPECT_FILESIZE                    (((uint64_t) 1) << 21)
#define INSPECT_PERMISSIONS                 (((uint64_t) 1) << 22)
#define INSPECT_CAPABILITIES                (((uint64_t) 1) << 23)
#define INSPECT_KMOD                        (((uint64_t) 1) << 24)
#define INSPECT_ARCH                        (((uint64_t) 1) << 25)
#define INSPECT_SUBPACKAGES                 (((uint64_t) 1) << 26)
#define INSPECT_CHANGELOG                   (((uint64_t) 1) << 27)
#define INSPECT_PATHMIGRATION               (((uint64_t) 1) << 28)
#define INSPECT_LTO                         (((uint64_t) 1) << 29)
#define INSPECT_SYMLINKS                    (((uint64_t) 1) << 30)
#define INSPECT_LOSTPAYLOAD                 (((uint64_t) 1) << 31)
#define INSPECT_FILES                       (((uint64_t) 1) << 32)
#define INSPECT_TYPES                       (((uint64_t) 1) << 33)
#define INSPECT_ABIDIFF                     (((uint64_t) 1) << 34)
#define INSPECT_KMIDIFF                     (((uint64_t) 1) << 35)
#define INSPECT_CONFIG                      (((uint64_t) 1) << 36)

/* Long descriptions for the inspections */
#define DESC_LICENSE _("Verify the string specified in the License tag of the RPM metadata describes permissible software licenses as defined by the license database. Also checks to see if the License tag contains any unprofessional words as defined in the configuration file.")

#define DESC_EMPTYRPM _("Check all binary RPMs in the build for any empty payloads. When comparing two builds, report new packages in the after build with empty payloads.")

#define DESC_LOSTPAYLOAD _("Check all binary RPMs in the before and after builds for any empty payloads. Packages that lost payload data from the before build to the after build are reported.")

#define DESC_METADATA _("Perform some RPM header checks. First, check that the Vendor contains the expected string as defined in the configuration file. Second, check that the build host is in the expected subdomain as defined in the configuration file. Third, check the Summary string for any unprofessional words. Fourth, check the Description for any unprofessional words. Lastly, if there is a before build specified, check for differences between the before and after build values of the previous RPM header values and report them.")

#define DESC_MANPAGE _("Perform some checks on man pages in the RPM payload. First, check that each man page is compressed. Second, check that each man page contains valid content. Lastly, check that each man page is installed to the correct path.")

#define DESC_XML _("Check that XML files included in the RPM payload are well-formed.")

#define DESC_ELF _("Perform several checks on ELF files. First, check that ELF objects do not contain an executable stack. Second, check that ELF objects do not contain text relocations. When comparing builds, check that the ELF objects in the after build did not lose a PT_GNU_RELRO segment. When comparing builds, check that the ELF objects in the after build did not lose -D_FORTIFY_SOURCE. Lastly, if there is a list of forbidden library functions, make sure nothing uses them.")

#define DESC_DESKTOP _("Perform syntax and file reference checks on *.desktop files. Syntax errors and invalid file references are reported as errors.")

#define DESC_DISTTAG _("Check that the 'Release' tag in the RPM spec file includes the %{?dist} directive.")

#define DESC_SPECNAME _("Ensure the spec file name conforms to the NAME.spec naming format.")

#define DESC_MODULARITY _("Ensure compliance with modularity build and packaging policies (only valid for module builds, no-op otherwise).")

#define DESC_JAVABYTECODE _("Check minimum required Java bytecode version in class files, report bytecode version changes between builds, and report if bytecode versions are exceeded.  The bytecode version is vendor specific to releases and defined in the configuration file.")

#define DESC_CHANGEDFILES _("Report changed files from the before build to the after build.  Certain file changes will raise additional warnings if the concern is more critical than just reporting changes (e.g., a suspected security impact).  Any gzip, bzip2, or xz compressed files will have their uncompressed content compared only, which will allow changes through in the compression level used.  Message catalog files (.mo) are unpacked and compared using diff(1).  Public C and C++ header files are preprocessed and compared using diff(1).  Any changes with diff output are included in the results.")

#define DESC_MOVEDFILES _("Report files that have moved installation paths or across subpackages between builds.  Files moved with a security path prefix generate special reporting in case a security review is required.  Rebased packages report these findings at the INFO level while non-rebased packages report them at the VERIFY level or higher.")

#define DESC_REMOVEDFILES _("Report removed files from the before build to the after build.  Shared libraries get additional reporting output as they may be unexpected dependency removals.  Files removed with a security path prefix generate special reporting in case a security review is required.  Source RPMs and debuginfo files are ignored by this inspection.  Rebased packages report these findings at the INFO level while non-rebased packages report them at the VERIFY level or higher.")

#define DESC_ADDEDFILES _("Report added files from the before build to the after build.  Debuginfo files are ignored as are files that match the patterns defined in the configuration file.  Files added to security paths generate special reporting in case a security review is required.  New setuid and setgid files raise a security warning unless the file is in the fileinfo list.  Rebased packages report these findings at the INFO level while non-rebased packages report them at the VERIFY level or higher.")

#define DESC_UPSTREAM _("Report Source archives defined in the RPM spec file changing content between the before and after build. If the source archives change and the package is on the rebaseable list, the change is reported as informational. Otherwise the change is reported as a rebase of the package and requires inspection.")

#define DESC_OWNERSHIP _("Report files and directories owned by unexpected users and groups. Check to make sure executables are owned by the correct user and group. If a before and after build have been specified, also report ownership changes.")

#define DESC_SHELLSYNTAX _("For all shell scripts in the build, perform a syntax check on it using the shell defined in its #! line (shell must also be listed in shell section of the configuration data). If the syntax check returns non-zero, report it to the user and return a combined stdout and stderr. If comparing two builds, perform the previous check but also report if a previously bad script is now passing the syntax check.")

#define DESC_ANNOCHECK _("Perform annocheck tests defined in the configuration file on all ELF files in the build.  A single build specified will perform an analysis only.  Two builds specified will compare the test results between the before and after build.  If no annocheck tests are defined in the configuration file, this inspection is skipped.")

#define DESC_DT_NEEDED _("Compare DT_NEEDED entries in dynamic ELF executables and shared libraries between the before and after build and report changes.")

#define DESC_FILESIZE _("Report file size changes between builds.  If empty files became non-empty or non-empty files became empty, report those as results needing verification.  Report file change percentages as info-only.")

#define DESC_PERMISSIONS _("Report stat(2) mode changes between builds.  Checks against the fileinfo list for the product release specified or determined.  Any setuid or setgid changes will raise a message requiring Security Team review.")

#define DESC_CAPABILITIES _("Report capabilities(7) changes between builds.  Checks against the capabilities list for the product release specified or determined.  Any capabilities changes not listed will raise a message requiring Security Team review.")

#define DESC_KMOD _("Report kernel module parameter, dependency, PCI ID, or symbol differences between builds.  Added and removed parameters are reported and if the package version is unchanged, these messages are reported as failures.  The same is true module dependencies, PCI IDs, and symbols.")

#define DESC_ARCH _("Report RPM architectures that appear and disappear between the before and after builds.")

#define DESC_SUBPACKAGES _("Report RPM subpackages that appear and disappear between the before and after builds.")

#define DESC_CHANGELOG _("Ensure packages contain an entry in the %changelog for the version built.  Reports any other differences in the existing changelog between builds and that the new entry contains new text entries.")

#define DESC_PATHMIGRATION _("Report files that are packaged in directories that are no longer used by the product.  Usually this means a package has not been updated to account for path migrations.  The main examples are /bin migrating to /usr/bin and /sbin migrating to /usr/sbin.")

#define DESC_LTO _("Link Time Optimization (LTO) produces smaller and faster shared ELF executables and libraries.  LTO bytecode is not stable from one release of gcc to the next.  As such, LTO bytecode should not be present in .a and .o ELF objects shipped in packages.  This inspection looks for LTO bytecode in ELF relocatable objects and reports if any is present.")

#define DESC_SYMLINKS _("Symbolic links must be resolvable on the installed system.  This inspection ensures absolute and relative symlinks are valid.  It also checks for any symlink usage that will cause problems for RPM.")

#define DESC_FILES _("Check %files sections in the spec file for any forbidden path references.")

#define DESC_TYPES _("Compare MIME types of files between builds and report any changes for verification.")

#define DESC_ABIDIFF _("When comparing two builds or two packages, compare ELF files using abidiff(1) from the libabigail project.  Differences are reported.  If the package is a rebase and not on the rebaseable list and the rebase inspection is enabled, ABI differences are reported as failures.  The assumption here is that rpminspect is comparing builds for maintenance purposes and you do not want to introduce any ABI changes for users.  If you do not care about that, turn off the abidiff inspection or add the package name to the rebaseable list.")

#define DESC_KMIDIFF _("kmidiff compares the binary Kernel Module Interfaces of two Linux kernel trees.  The binary KMI is the interface that the Linux kernel exposes to its modules.  The trees we are interested in here are the result of the build of the Linux kernel source tree.  If the builds compared are not considered a rebase, an incompatible change reported by kmidiff is reported for verification.")

#define DESC_CONFIG _("Check for and report differences in configuration files marked with %config in the spec file.  If changes are whitespace or formatting only, the result is reported at the INFO level.  Content and location changes are reporting at the VERIFY level unless the comparison is between rebased packages.")

#endif
