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

/*
 * This header defines the test functions provided by librpminspect.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <libelf.h>
#include <libkmod.h>

#include "types.h"

#ifndef _LIBRPMINSPECT_INSPECT_H
#define _LIBRPMINSPECT_INSPECT_H

/*
 * Inspection drivers (and their helpers)
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

/* inspect.c */
typedef bool (*foreach_peer_file_func)(struct rpminspect *, rpmfile_entry_t *);
bool foreach_peer_file(struct rpminspect *, foreach_peer_file_func);
const char *inspection_desc(const uint64_t);

/* inspect_elf.c */
void init_elf_data(void);
void free_elf_data(void);
bool has_executable_program(Elf *);
bool is_execstack_present(Elf *);
uint64_t get_execstack_flags(Elf *);
bool is_execstack_valid(Elf *, uint64_t);
bool is_stack_executable(Elf *, uint64_t);
bool has_textrel(Elf *);
bool has_relro(Elf *);
bool has_bind_now(Elf *);
string_list_t * get_fortified_symbols(Elf *);
string_list_t * get_fortifiable_symbols(Elf *);
bool is_pic_ok(Elf *);
bool inspect_elf(struct rpminspect *);

/* inspect_license.c */
void free_licensedb(void);
bool is_valid_license(const char *, const char *);
bool inspect_license(struct rpminspect *);

/* inspect_emptyrpm.c */
bool is_payload_empty(rpmfile_t *);
bool inspect_emptyrpm(struct rpminspect *);

/* inspect_xml.c */
bool is_xml_well_formed(const char *, char **);
bool inspect_xml(struct rpminspect *);

/* inspect_manpage.c */
bool inspect_manpage(struct rpminspect *);

/* inspect_metadata.c */
bool inspect_metadata(struct rpminspect *);

/* inspect_desktop.c */
bool inspect_desktop(struct rpminspect *);

/* inspect_disttag.c */
bool inspect_disttag(struct rpminspect *);

/* inspect_specname.c */
bool inspect_specname(struct rpminspect *);

/* inspect_modularity.c */
bool inspect_modularity(struct rpminspect *);

/* inspect_javabytecode.c */
bool inspect_javabytecode(struct rpminspect *);

/* inspect_changedfiles.c */
bool inspect_changedfiles(struct rpminspect *);

/* inspect_removedfiles.c */
bool inspect_removedfiles(struct rpminspect *);

/* inspect_addedfiles.c */
bool inspect_addedfiles(struct rpminspect *);

/* inspect_upstream.c */
bool inspect_upstream(struct rpminspect *);

/* inspect_ownership.c */
bool inspect_ownership(struct rpminspect *);

/* inspect_shellsyntax.c */
bool inspect_shellsyntax(struct rpminspect *);

/* inspect_annocheck.c */
bool inspect_annocheck(struct rpminspect *);

/* inspect_dt_needed.c */
bool inspect_dt_needed(struct rpminspect *);

/* inspect_filesize.c */
bool inspect_filesize(struct rpminspect *);

/* inspect_permissions.c */
bool inspect_permissions(struct rpminspect *);

/* inspect_capabilities.c */
bool inspect_capabilities(struct rpminspect *);

/* inspect_kmod.c */
bool inspect_kmod(struct rpminspect *);

/* inspect_arch.c */
bool inspect_arch(struct rpminspect *);

/* inspect_subpackages.c */
bool inspect_subpackages(struct rpminspect *);

/* inspect_changelog.c */
bool inspect_changelog(struct rpminspect *);

/* inspect_pathmigration.c */
bool inspect_pathmigration(struct rpminspect *);

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
#define INSPECT_REMOVEDFILES                (((uint64_t) 1) << 13)
#define INSPECT_ADDEDFILES                  (((uint64_t) 1) << 14)
#define INSPECT_UPSTREAM                    (((uint64_t) 1) << 15)
#define INSPECT_OWNERSHIP                   (((uint64_t) 1) << 16)
#define INSPECT_SHELLSYNTAX                 (((uint64_t) 1) << 17)
#define INSPECT_ANNOCHECK                   (((uint64_t) 1) << 18)
#define INSPECT_DT_NEEDED                   (((uint64_t) 1) << 19)
#define INSPECT_FILESIZE                    (((uint64_t) 1) << 20)
#define INSPECT_PERMISSIONS                 (((uint64_t) 1) << 21)
#define INSPECT_CAPABILITIES                (((uint64_t) 1) << 22)
#define INSPECT_KMOD                        (((uint64_t) 1) << 23)
#define INSPECT_ARCH                        (((uint64_t) 1) << 24)
#define INSPECT_SUBPACKAGES                 (((uint64_t) 1) << 25)
#define INSPECT_CHANGELOG                   (((uint64_t) 1) << 26)
#define INSPECT_PATHMIGRATION               (((uint64_t) 1) << 27)

/* Long descriptions for the inspections */
#define DESC_LICENSE "Verify the string specified in the License tag of the RPM metadata describes permissible software licenses as defined by the license database. Also checks to see if the License tag contains any unprofessional words as defined in the configuration file."

#define DESC_EMPTYRPM "Check all binary RPMs in the before and after builds for any empty payloads. Packages that lost payload data from the before build to the after build are reported as well as any packages in the after build that exist but have no payload data."

#define DESC_METADATA "Perform some RPM header checks. First, check that the Vendor contains the expected string as defined in the configuration file. Second, check that the build host is in the expected subdomain as defined in the configuration file. Third, check the Summary string for any unprofessional words. Fourth, check the Description for any unprofessional words. Lastly, if there is a before build specified, check for differences between the before and after build values of the previous RPM header values and report them."

#define DESC_MANPAGE "Perform some checks on man pages in the RPM payload. First, check that each man page is compressed. Second, check that each man page contains valid content. Lastly, check that each man page is installed to the correct path."

#define DESC_XML "Check that XML files included in the RPM payload are well-formed."

#define DESC_ELF "Perform several checks on ELF files. First, check that ELF objects do not contain an executable stack. Second, check that ELF objects do not contain text relocations. When comparing builds, check that the ELF objects in the after build did not lose a PT_GNU_RELRO segment. When comparing builds, check that the ELF objects in the after build did not lose -D_FORTIFY_SOURCE. Lastly, if there is a list of forbidden library functions, make sure nothing uses them."

#define DESC_DESKTOP "Perform syntax and file reference checks on *.desktop files. Syntax errors and invalid file references are reported as errors."

#define DESC_DISTTAG "Check that the 'Release' tag in the RPM spec file includes the %{?dist} directive."

#define DESC_SPECNAME "Ensure the spec file name conforms to the NAME.spec naming format."

#define DESC_MODULARITY "Ensure compliance with modularity build and packaging policies (only valid for module builds, no-op otherwise)."

#define DESC_JAVABYTECODE "Check minimum required Java bytecode version in class files, report bytecode version changes between builds, and report if bytecode versions are exceeded.  The bytecode version is vendor specific to releases and defined in the configuration file."

#define DESC_CHANGEDFILES "Report changed files from the before build to the after build.  Certain file changes will raise additional warnings if the concern is more critical than just reporting changes (e.g., a suspected security impact).  Any gzip, bzip2, or xz compressed files will have their uncompressed content compared only, which will allow changes through in the compression level used.  Message catalog files (.mo) are unpacked and compared using diff(1).  Public C and C++ header files are preprocessed and compared using diff(1).  Any changes with diff output are included in the results."

#define DESC_REMOVEDFILES "Report removed files from the before build to the after build.  Shared libraries get additional reporting output as they may be unexpected dependency removals.  Files removed with a security path prefix generated special reporting in case a security review is required.  Source RPMs and debuginfo files are ignored by this inspection."

#define DESC_ADDEDFILES "Report added files from the before build to the after build.  Debuginfo files are ignored as are files that match the patterns defined in the configuration file.  Files added to security paths generate special reporting in case a security review is required.  New setuid and setgid files raise a security warning unless the file is in the whitelist."

#define DESC_UPSTREAM "Report Source archives defined in the RPM spec file changing content between the before and after build. If the source archives change and the package is on the version-whitelist, the change is reported as informational. Otherwise the change is reported as a rebase of the package and requires inspection."

#define DESC_OWNERSHIP "Report files and directories owned by unexpected users and groups. Check to make sure executables are owned by the correct user and group. If a before and after build have been specified, also report ownership changes."

#define DESC_SHELLSYNTAX "For all shell scripts in the build, perform a syntax check on it using the shell defined in its #! line (shell must also be listed in rpminspect.conf's shell setting). If the syntax check returns non-zero, report it to the user and return a combined stdout and stderr. If comparing two builds, perform the previous check but also report if a previously bad script is now passing the syntax check."

#define DESC_ANNOCHECK "Perform annocheck tests defined in the configuration file on all ELF files in the build.  A single build specified will perform an analysis only.  Two builds specified will compare the test results between the before and after build.  If no annocheck tests are defined in the configuration file, this inspection is skipped."

#define DESC_DT_NEEDED "Compare DT_NEEDED entries in dynamic ELF executables and shared libraries between the before and after build and report changes."

#define DESC_FILESIZE "Report file size changes between builds.  If empty files became non-empty or non-empty files became empty, report those as results needing verification.  Report file change percentages as info-only."

#define DESC_PERMISSIONS "Report stat(2) mode changes between builds.  Checks against the stat-whitelist for the product release specified or determined.  Any setuid or setgid changes will raise a message requiring Security Team review."

#define DESC_CAPABILITIES "Report capabilities(7) changes between builds.  Checks against the capabilities whitelist for the product release specified or determined.  Any capabilities changes not whitelisted will raise a message requiring Security Team review."

#define DESC_KMOD "Report kernel module parameter, dependency, PCI ID, or symbol differences between builds.  Added and removed parameters are reported and if the package version is unchanged, these messages are reported as failures.  The same is true module dependencies, PCI IDs, and symbols"

#define DESC_ARCH "Report RPM architectures that appear and disappear between the before and after builds."

#define DESC_SUBPACKAGES "Report RPM subpackages that appear and disappear between the before and after builds."

#define DESC_CHANGELOG "Ensure packages contain an entry in the %changelog for the version built.  Reports any other differences in the existing changelog between builds and that the new entry contains new text entries."

#endif
