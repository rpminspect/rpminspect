/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "rpminspect.h"

/*
 * Ensure the array of remedies is only defined once.
 */

struct remedy remedies[] = {
    /*
     * { REMEDY_TYPE (add to remedy.h),
     *   "short name",
     *   "default remedy message" },
     */
    { REMEDY_NULL, "", NULL },
    { REMEDY_ABIDIFF, "abidiff", NULL },
    { REMEDY_ADDEDFILES, "addedfiles", NULL },
    { REMEDY_ANNOCHECK_FORTIFY_SOURCE, "annocheck_fortify_source", NULL },
    { REMEDY_ANNOCHECK, "annocheck", NULL },
    { REMEDY_ARCH_GAIN, "arch_gain", NULL },
    { REMEDY_ARCH_LOST, "arch_lost", NULL },
    { REMEDY_BADFUNCS, "badfuncs", NULL },
    { REMEDY_BADWORDS, "badwords", NULL },
    { REMEDY_BUILDHOST, "buildhost", NULL },
    { REMEDY_CAPABILITIES, "capabilities", NULL },
    { REMEDY_CHANGEDFILES, "changedfiles", NULL },
    { REMEDY_CHANGELOG, "changelog", NULL },
    { REMEDY_CONFIG, "config", NULL },
    { REMEDY_DESKTOP, "desktop", NULL },
    { REMEDY_DISTTAG, "disttag", NULL },
    { REMEDY_DOC, "doc", NULL },
    { REMEDY_DSODEPS, "dsodeps", NULL },
    { REMEDY_ELF_EXECSTACK_EXECUTABLE, "elf_execstack_executable", NULL },
    { REMEDY_ELF_EXECSTACK_INVALID, "elf_execstack_invalid", NULL },
    { REMEDY_ELF_EXECSTACK_MISSING, "elf_execstack_missing", NULL },
    { REMEDY_ELF_FPIC, "elf_fpic", NULL },
    { REMEDY_ELF_GNU_RELRO, "elf_gnu_relro", NULL },
    { REMEDY_ELF_TEXTREL, "elf_textrel", NULL },
    { REMEDY_EMPTYRPM, "emptyrpm", NULL },
    { REMEDY_FILEINFO_RULE, "fileinfo_rule", NULL },
    { REMEDY_FILESIZE_BECAME_EMPTY, "filesize_became_empty", NULL },
    { REMEDY_FILESIZE_BECAME_NOT_EMPTY, "filesize_became_not_empty", NULL },
    { REMEDY_FILESIZE_GREW, "filesize_grew", NULL },
    { REMEDY_FILESIZE_SHRANK, "filesize_shrank", NULL },
    { REMEDY_FILESMATCH_EXCLUDE_FOUND, "excluded_file_found", NULL },
    { REMEDY_FILESMATCH_UNSPECIFIED_ENTRY, "unspecified_entry_found", NULL },
    { REMEDY_FILE_PATHS, "file_paths", NULL },
    { REMEDY_INVALID_BOOLEAN, "invalid_boolean", NULL },
    { REMEDY_JAVABYTECODE, "javabytecode", NULL },
    { REMEDY_KMIDIFF, "kmidiff", NULL },
    { REMEDY_KMOD_ALIAS, "kmod_alias", NULL },
    { REMEDY_KMOD_DEPS, "kmod_deps", NULL },
    { REMEDY_KMOD_PARM, "kmod_parm", NULL },
    { REMEDY_LICENSEDB, "licensedb", NULL },
    { REMEDY_LICENSE, "license", NULL },
    { REMEDY_LOSTPAYLOAD, "lostpayload", NULL },
    { REMEDY_LTO, "lto", NULL },
    { REMEDY_MAN_ERRORS, "man_errors", NULL },
    { REMEDY_MAN_PATH, "man_path", NULL },
    { REMEDY_MODULARITY_LABEL, "modularity_label", NULL },
    { REMEDY_MODULARITY_RELEASE, "modularity_release", NULL },
    { REMEDY_MODULARITY_STATIC_CONTEXT, "modularity_static_context", NULL },
    { REMEDY_MOVEDFILES, "movedfiles", NULL },
    { REMEDY_OWNERSHIP_BIN_GROUP, "ownership_bin_group", NULL },
    { REMEDY_OWNERSHIP_BIN_OWNER, "ownership_bin_owner", NULL },
    { REMEDY_OWNERSHIP_CHANGED, "ownership_changed", NULL },
    { REMEDY_OWNERSHIP_DEFATTR, "ownership_defattr", NULL },
    { REMEDY_OWNERSHIP_IWGRP, "ownership_iwgrp", NULL },
    { REMEDY_OWNERSHIP_IXOTH, "ownership_ixoth", NULL },
    { REMEDY_PATCHES_CORRUPT, "patches_corrupt", NULL },
    { REMEDY_PATCHES_MISMATCHED_MACRO, "patches_mismatched_macro", NULL },
    { REMEDY_PATCHES_MISSING_MACRO, "patches_missing_macro", NULL },
    { REMEDY_PATCHES_UNHANDLED_PATCH, "patches_unhandled_patch", NULL },
    { REMEDY_PATHMIGRATION, "pathmigration", NULL },
    { REMEDY_POLITICS, "politics", NULL },
    { REMEDY_REMOVEDFILES, "removedfiles", NULL },
    { REMEDY_RPMDEPS_CHANGED, "rpmdeps_changed", NULL },
    { REMEDY_RPMDEPS_EPOCH, "rpmdeps_epoch", NULL },
    { REMEDY_RPMDEPS_EXPLICIT, "rpmdeps_explicit", NULL },
    { REMEDY_RPMDEPS_EXPLICIT_EPOCH, "rpmdeps_explicit_epoch", NULL },
    { REMEDY_RPMDEPS_GAINED, "rpmdeps_gained", NULL },
    { REMEDY_RPMDEPS_LOST, "rpmdeps_lost", NULL },
    { REMEDY_RPMDEPS_MACROS, "rpmdeps_macros", NULL },
    { REMEDY_RPMDEPS_MULTIPLE, "rpmdeps_multiple", NULL },
    { REMEDY_RUNPATH_BOTH, "runpath_both", NULL },
    { REMEDY_RUNPATH, "runpath", NULL },
    { REMEDY_SHELLSYNTAX_BAD, "shellsyntax_bad", NULL },
    { REMEDY_SHELLSYNTAX, "shellsyntax", NULL },
    { REMEDY_SHELLSYNTAX_GAINED_SHELL, "shellsyntax_gained_shell", NULL },
    { REMEDY_SPECNAME, "specname", NULL },
    { REMEDY_SUBPACKAGES_GAIN, "subpackages_gain", NULL },
    { REMEDY_SUBPACKAGES_LOST, "subpackages_lost", NULL },
    { REMEDY_SYMLINKS_DIRECTORY, "symlinks_directory", NULL },
    { REMEDY_SYMLINKS, "symlinks", NULL },
    { REMEDY_TYPES, "types", NULL },
    { REMEDY_UDEVRULES, "udevrules", NULL },
    { REMEDY_UNAPPROVED_LICENSE, "unapproved_license", NULL },
    { REMEDY_UNICODE_PREP_FAILED, "unicode_prep_failed", NULL },
    { REMEDY_UNICODE, "unicode", NULL },
    { REMEDY_UPSTREAM, "upstream", NULL },
    { REMEDY_VENDOR, "vendor", NULL },
    { REMEDY_VIRUS, "virus", NULL },
    { REMEDY_XML, "xml", NULL },
    { REMEDY_MIXED_LICENSE_TAGS, "mixed_license_tags", NULL },
    { 0, NULL, NULL }
};

/*
 * Return the remedy string for the given remedy ID.
 */
const char *get_remedy(const unsigned int id)
{
    char *r = NULL;
    unsigned int i = 0;

    for (i = 0; remedies[i].name != NULL; i++) {
        if (remedies[i].id == id) {
            if (remedies[i].remedy != NULL) {
                r = remedies[i].remedy;
            } else {
                if (id == REMEDY_ABIDIFF) {
                    r = _("ABI changes introduced during maintenance updates can lead to problems for users.  See the abidiff(1) documentation and the distribution ABI policies to determine if this detected change is allowed.");
                } else if (id == REMEDY_ADDEDFILES) {
                    r = _("Unexpected file additions were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file additions between builds.  If they are correct, update the fileinfo list for this product release and send a patch to the rpminspect data project owning that file so rpminspect knows to expect this change.  You may also need to update the data package or local configuration file and change the forbidden_path_prefixes or forbidden_path_suffixes list.");
                } else if (id == REMEDY_ANNOCHECK_FORTIFY_SOURCE) {
                    r = _("Ensure all object files are compiled with '-O2 -D_FORTIFY_SOURCE=2', and that all appropriate headers are included (no implicit function declarations). Symbols may also appear as unfortified if the compiler is unable to determine the size of a buffer, which is not necessarily an error.");
                } else if (id == REMEDY_ANNOCHECK) {
                    r = _("See annocheck(1) for more information.");
                } else if (id == REMEDY_ARCH_GAIN) {
                    r = _("A new architecture has appeared in the after build.  This may indicate progress in the world of computing.");
                } else if (id == REMEDY_ARCH_LOST) {
                    r = _("An architecture present in the before build is now missing in the after build.  This may be deliberate, but check to make sure you do not have any unexpected ExclusiveArch lines in the spec file.");
                } else if (id == REMEDY_BADFUNCS) {
                    r = _("Forbidden symbols were found in an ELF file in the package.  The configuration settings for rpminspect indicate the named symbols are forbidden in packages.  If this is deliberate, you may want to disable the badfuncs inspection.  If it is not deliberate, check the man pages for the named symbols to see what API functions have replaced the forbidden symbols.  Usually a function is marked as deprecated but still provided in order to allow for backwards compatibility.  Whenever possible the deprecated functions should not be used.");
                } else if (id == REMEDY_BADWORDS) {
                    r = _("Unprofessional language as defined in the configuration file was found in the text shown.  Remove or change the offending words and rebuild.");
                } else if (id == REMEDY_BUILDHOST) {
                    r = _("Make sure the SRPM is built on a host within the expected subdomain.");
                } else if (id == REMEDY_CAPABILITIES) {
                    r = _("Unexpected capabilities were found on the indicated file.  Consult capabilities(7) and either adjust the files in the package or modify the capabilities list in the rpminspect vendor data package.  The security team may also be of help for this situation.  If necessary, update the capabilities file for this product release with the changes found here and send a patch to the project that owns the rpminspect data file.");
                } else if (id == REMEDY_CHANGEDFILES) {
                    r = _("File changes were found.  In most cases these are expected, but it is a good idea to verify the changes found are deliberate.");
                } else if (id == REMEDY_CHANGELOG) {
                    r = _("Make sure the spec file in the after build contains a valid %changelog section.");
                } else if (id == REMEDY_CONFIG) {
                    r = _("Changes to %config should be done carefully.  Make sure you have installed the correct file and in the correct location.  If a package is restructuring configuration files, make sure the package can handle upgrading an existing package -or- honor the old file locations.");
                } else if (id == REMEDY_DESKTOP) {
                    r = _("Refer to the Desktop Entry Specification at https://standards.freedesktop.org/desktop-entry-spec/latest/ for help correcting the errors and warnings.");
                } else if (id == REMEDY_DISTTAG) {
                    r = _("The Release: tag in the spec file must include a '%{?dist}' string.  Please add this to the spec file per the distribution packaging guidelines.");
                } else if (id == REMEDY_DOC) {
                    r = _("Changes found among the %doc files.  Verify these changes are intended if the package is not a rebase.  Sometimes upstream projects rename or move documentation files and the spec file needs to account for those changes.");
                } else if (id == REMEDY_DSODEPS) {
                    r = _("DT_NEEDED symbols have been added or removed.  This happens when the build environment has different versions of the required libraries.  Sometimes this is deliberate but sometimes not.  Verify these changes are expected.  If they are not, modify the package spec file to ensure the build links with the correct shared libraries.");
                } else if (id == REMEDY_ELF_EXECSTACK_EXECUTABLE) {
                    r = _("An ELF stack is marked as executable. Ensure that no execstack options are being passed to the linker, and that no functions are defined on the stack.");
                } else if (id == REMEDY_ELF_EXECSTACK_INVALID) {
                    r = _("The data in an ELF file appears to be corrupt; ensure that packaged ELF files are not being truncated or incorrectly modified.");
                } else if (id == REMEDY_ELF_EXECSTACK_MISSING) {
                    r = _("Ensure that the package is being built with the correct compiler and compiler flags.");
                } else if (id == REMEDY_ELF_FPIC) {
                    r = _("Ensure all object files are compiled with -fPIC.");
                } else if (id == REMEDY_ELF_GNU_RELRO) {
                    r = _("Ensure executables are linked with with '-z relro -z now'.");
                } else if (id == REMEDY_ELF_TEXTREL) {
                    r = _("Ensure all object files are compiled with -fPIC.");
                } else if (id == REMEDY_EMPTYRPM) {
                    r = _("Check to see if you eliminated a subpackage but still have the %package and/or the %files section for it.");
                } else if (id == REMEDY_FILEINFO_RULE) {
                    r = _("rpminspect is expecting a fileinfo rule from the vendor data package for this file. Usually this means the file carries a non-standard set of permissions (e.g., setuid) which is a condition where rpminspect would check the fileinfo list to ensure the package conforms to the vendor rules. To remedy, add a fileinfo rule for this file to the vendor data package under the appropriate product release file.");
                } else if (id == REMEDY_FILESIZE_BECAME_EMPTY) {
                    r = _("A previously non-empty file is now empty.  Make sure this change is intended and fix the package space file if necessary.");
                } else if (id == REMEDY_FILESIZE_BECAME_NOT_EMPTY) {
                    r = _("A previously empty file is no longer empty.  Make sure this change is intended and fix the package spec file if necessary.");
                } else if (id == REMEDY_FILESIZE_GREW) {
                    r = _("A file grew by a noticeable amount.  Ensure this change is intended.  If it is, you can adjust the filesize inspection settings in the rpminspect.yaml file.");
                } else if (id == REMEDY_FILESIZE_SHRANK) {
                    r = _("A file shrank by a noticeable amount.  Ensure this change is intended.  If it is, you can adjust the filesize inspection settings in the rpminspect.yaml file.");
                } else if (id == REMEDY_FILE_PATHS) {
                    r = _("Remove forbidden path references from the indicated line in the %files section.  In many cases you can use RPM macros to specify path locations.  See the RPM documentation or distribution package maintainer guide for more information.");
                } else if (id == REMEDY_INVALID_BOOLEAN) {
                    r = _("The License tag contains SPDX license identifiers.  When SPDX identifiers are used, the boolean joining terms must be written in all capital letters per the spec.  The actual SPDX identifiers are case-insensitive.  It is only the boolean terms that must be in all capital letters.");
                } else if (id == REMEDY_JAVABYTECODE) {
                    r = _("The Java bytecode version for one or more class files in the build was not met for the product release.  Ensure you are using the correct JDK for the build.");
                } else if (id == REMEDY_KMIDIFF) {
                    r = _("Kernel Module Interface introduced during maintenance updates can lead to problems for users.  See the libabigail documentation and the distribution KMI policy to determine if this detected change is allowed.");
                } else if (id == REMEDY_KMOD_ALIAS) {
                    r = _("Kernel module device aliases changed between builds.  This may present usability problems for users if module device aliases changed in a maintenance update.");
                } else if (id == REMEDY_KMOD_DEPS) {
                    r = _("Kernel module dependencies changed between builds.  This may present usability problems for users if module dependencies changed in a maintenance update.");
                } else if (id == REMEDY_KMOD_PARM) {
                    r = _("Kernel module parameters were removed between builds.  This may present usability problems for users if module parameters were removed in a maintenance update.");
                } else if (id == REMEDY_LICENSEDB) {
                    r = _("Make sure the licensedb setting in the rpminspect configuration is set to a valid licensedb file.  This is also commonly due to a missing vendor specific rpminspect-data package on the system.");
                } else if (id == REMEDY_LICENSE) {
                    r = _("The License tag must contain an approved license string as defined by the distribution (e.g., GPLv2+).  If the license in question is approved, the license database needs updating in the rpminspect-data package.");
                } else if (id == REMEDY_LOSTPAYLOAD) {
                    r = _("Check to see if you eliminated a subpackage but still have the %package and/or the %files section for it.");
                } else if (id == REMEDY_LTO) {
                    r = _("ELF .o and .a files should not carry LTO (Link Time Optimization) bytecode.  Make sure you have stripped LTO bytecode from those files at install time.");
                } else if (id == REMEDY_MAN_ERRORS) {
                    r = _("Correct the errors in the man page as reported by the libmandoc parser.");
                } else if (id == REMEDY_MAN_PATH) {
                    r = _("Correct the installation path for the man page. Man pages must be installed in the directory beneath /usr/share/man that matches the section number of the page.");
                } else if (id == REMEDY_MODULARITY_LABEL) {
                    r = _("This package is part of a module but is missing the %{modularitylabel} header tag.  Add this as a %define in the spec file and rebuild.");
                } else if (id == REMEDY_MODULARITY_RELEASE) {
                    r = _("This package is part of a module but lacks a conformant Release tag value.  A Release tag in a modular RPM needs to carry a substring that is more specific than a major release dist tag (e.g., el8.9.0 rather than el8) and must carry '+module' as a substring before that specific dist tag.");
                } else if (id == REMEDY_MODULARITY_STATIC_CONTEXT) {
                    r = _("This build either contains a valid or invalid /data/static_context setting.  Refer to the module rules for the product you are building to determine what the setting should be.  The rpminspect configuration settings also set the rules determining if the /data/static_context setting is required, forbidden, or recommend.");
                } else if (id == REMEDY_MOVEDFILES) {
                    r = _("Unexpected file moves were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file moves between builds.");
                } else if (id == REMEDY_OWNERSHIP_BIN_GROUP) {
                    r = _("Bin path files must be owned by the bin_group set in the rpminspect configuration, which is usually root. If this ownership is expected, update the fileinfo exception list for this product release and send a patch to the project that owns the rpminspect data files.");
                } else if (id == REMEDY_OWNERSHIP_BIN_OWNER) {
                    r = _("Bin path files must be owned by the bin_owner set in the rpminspect configuration, which is usually root. If this ownership is expected, update the fileinfo exception list for this product release and send a patch to the project that owns the rpminspect data files.");
                } else if (id == REMEDY_OWNERSHIP_CHANGED) {
                    r = _("Verify the ownership changes are expected. If not, adjust the package build process to set correct owner and group information. If expected, update the fileinfo exception list for this product release and send a patch to the project that owns the rpminspect data files.");
                } else if (id == REMEDY_OWNERSHIP_DEFATTR) {
                    r = _("Make sure the %files section includes the %defattr macro. If these permissions are expected, update the fileinfo exception list for this product release and send a patch to the project that owns the rpminspect data files.");
                } else if (id == REMEDY_OWNERSHIP_IWGRP) {
                    r = _("Either chgrp the file to the bin_group set in the rpminspect configuration or remove the group write bit on the file (chmod g-w). If this ownership is expected, update the fileinfo exception list for this product release and send a patch to the project that owns the rpminspect data files.");
                } else if (id == REMEDY_OWNERSHIP_IXOTH) {
                    r = _("Either chgrp the file to the bin_group set in the rpminspect configuration or remove the world execute bit on the file (chmod o-x). If this ownership is expected, update the fileinfo exception list for this product release and send a patch to the project that owns the rpminspect data files.");
                } else if (id == REMEDY_PATCHES_CORRUPT) {
                    r = _("An invalid patch file was found.  This is usually the result of generating a collection of patches by comparing two trees.  When files disappear that can lead to zero length patches in the resulting collection.  Check to see if the source package has any zero length or otherwise invalid patches and correct the problem.");
                } else if (id == REMEDY_PATCHES_MISMATCHED_MACRO) {
                    r = _("The named patch is defined but is mismatched by number with the %patch macro.  Make sure all numbered patches have corresponding %patch macros.  For example, Patch47 needs to have either a '%patch 47', '%patch -P 47', '%patch -P47', or '%patch47' macro.  Alternatively, you can use %autosetup or %autopatch to simplify unpacking source archives and applying patches.  Avoid invoking the patch(1) command directly or using the %__patch macro as rpminspect is unable to read these lines.");
                } else if (id == REMEDY_PATCHES_MISSING_MACRO) {
                    r = _("The named patch is defined in the source RPM header (this means it has a PatchN: definition in the spec file) but is not applied anywhere in the spec file.  It is missing a corresponding %patch macro and the spec file lacks the %autosetup or %autopatch macros.  You can fix this by adding the appropriate %patch macro in the spec file (usually in the %prep section).  The number specified with the %patch macro corresponds to the number used to define the patch at the top of the spec file.  So Patch47 is applied with either a '%patch 47', '%patch -P 47', '%patch -P47', or '%patch47' macro.  Avoid invoking the patch(1) command directly or using the %__patch macro as rpminspect is unable to read these lines.");
                } else if (id == REMEDY_PATCHES_UNHANDLED_PATCH) {
                    r = _("The defined patch file is not something rpminspect can handle.  This is likely a bug and should be reported to the upstream rpminspect project.");
                } else if (id == REMEDY_PATHMIGRATION) {
                    r = _("Files should not be installed in old directory names.  Modify the package to install the affected file to the preferred directory.");
                } else if (id == REMEDY_POLITICS) {
                    r = _("A file with potential politically sensitive content was found in the package.  If this file is permitted, it should be added to the rpminspect vendor data package for the product.  Modify the politics allow/deny list file for this product release and send a patch to the project that owns the rpminspect data files.");
                } else if (id == REMEDY_REMOVEDFILES) {
                    r = _("Unexpected file removals were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file removals between builds.");
                } else if (id == REMEDY_RPMDEPS_CHANGED) {
                    r = _("A dependency listed in the before build changed to the indicated dependency in the after build.  If this is a VERIFY result, it means rpminspect noticed the change in what it considers a maintenance update in a package.  An INFO result means it noticed this change, but deems it ok because it is comparing a rebased build.");
                } else if (id == REMEDY_RPMDEPS_EPOCH) {
                    r = _("The package has an Epoch value greater than zero, but the explicit subpackage dependencies are not consistently using it.  For the dependency reported, the '= %{version}-%{release}' needs to change to '= %{epoch}:%{version}-%{release}' to capture the package Epoch in the dependency.");
                } else if (id == REMEDY_RPMDEPS_EXPLICIT) {
                    r = _("Add the indicated explicit Requires to the spec file for the named subpackage.  Subpackages depending on shared libraries in another subpackage must carry an explicit 'Requires: SUBPACKAGE_NAME = %{version}-%{release}' in the spec file.");
                } else if (id == REMEDY_RPMDEPS_EXPLICIT_EPOCH) {
                    r = _("Add the indicated explicit Requires to the spec file for the named subpackage.  Subpackages depending on shared libraries in another subpackage must carry an explicit 'Requires: SUBPACKAGE_NAME = %{epoch}:%{version}-%{release}' in the spec file.");
                } else if (id == REMEDY_RPMDEPS_GAINED) {
                    r = _("A new dependency is seen in the after build that was not present in the before build.  If this is a VERIFY result, it means rpminspect noticed the change in what it considers a maintenance update in a package.  An INFO result means it noticed this change, but deems it ok because it is comparing a rebased build.");
                } else if (id == REMEDY_RPMDEPS_LOST) {
                    r = _("A dependency seen in the before build is not seen in the after build meaning it was removed or lost.  If this is a VERIFY result, it means rpminspect noticed the change in what it considers a maintenance update in a package.  An INFO result means it noticed this change, but deems it ok because it is comparing a rebased build.");
                } else if (id == REMEDY_RPMDEPS_MACROS) {
                    r = _("Unexpanded RPM spec file macros were found in the noted dependency rule.  Check the spec file for this dependency and ensure you have not misspelled a macro or used a macro name that does not exist.");
                } else if (id == REMEDY_RPMDEPS_MULTIPLE) {
                    r = _("Check subpackage %files sections and explicit Provides statements.  Only one subpackage should provide a given shared library.  Shared library names are automatically added as Provides, so there is no need to specify them in the spec file but you do need to make sure only one subpackage is packaging up the shared library in question.");
                } else if (id == REMEDY_RUNPATH_BOTH) {
                    r = _("Both DT_RPATH and DT_RUNPATH properties were found in an ELF shared object.  This indicates a linker error and should not happen.  ELF objects should only carry DT_RPATH or DT_RUNPATH, never both.");
                } else if (id == REMEDY_RUNPATH) {
                    r = _("Either DT_RPATH or DT_RUNPATH properties were found on ELF shared objects in this package.  The use of DT_RPATH and DT_RUNPATH is discouraged except in certain situations.  Check to see that you are disabling rpath during the %build stage of the spec file.  If you are unable to do this easily, you can try using a program such as patchelf to remove these properties from the ELF files.");
                } else if (id == REMEDY_SHELLSYNTAX_BAD) {
                    r = _("The referenced shell script is invalid. Consider debugging it with the '-n' option on the shell to find and fix the problem.");
                } else if (id == REMEDY_SHELLSYNTAX) {
                    r = _("Consult the shell documentation for proper syntax.");
                } else if (id == REMEDY_SHELLSYNTAX_GAINED_SHELL) {
                    r = _("The file referenced was not a known shell script in the before build but is now a shell script in the after build.");
                } else if (id == REMEDY_SPECNAME) {
                    r = _("The spec file name does not match the expected NAME.spec format.  Rename the spec file to conform to this policy.");
                } else if (id == REMEDY_SUBPACKAGES_GAIN) {
                    r = _("A new subpackage has appeared in the after build.  This may indicate progress in the world of computing.");
                } else if (id == REMEDY_SUBPACKAGES_LOST) {
                    r = _("A subpackage present in the before build is now missing in the after build.  This may be deliberate, but check to make sure you have correct syntax defining the subpackage in the spec file.");
                } else if (id == REMEDY_SYMLINKS_DIRECTORY) {
                    r = _("Make sure symlinks point to a valid destination in one of the subpackages of the build; dangling symlinks are not allowed.  If you are comparing builds and have a non-symlink turn in to a symlink, ensure this is deliberate.  NOTE:  You cannot turn a directory in to a symlink due to RPM limitations.  If you absolutely must do that, make sure you include the %pretrans scriptlet for replacing a directory.  See the packaging guidelines for 'Scriptlet to replace a directory' for more information.");
                } else if (id == REMEDY_SYMLINKS) {
                    r = _("Make sure symlinks point to a valid destination in one of the subpackages of the build; dangling symlinks are not allowed.  If you are comparing builds and have a non-symlink turn in to a symlink, ensure this is deliberate.  NOTE:  You cannot turn a directory in to a symlink due to RPM limitations.");
                } else if (id == REMEDY_TYPES) {
                    r = _("In many cases the changing MIME type is deliberate.  Verify that the change is intended and if necessary fix the spec file so the correct file is included in the built package.");
                } else if (id == REMEDY_UDEVRULES) {
                    r = _("Refer to the udev documentation at https://www.freedesktop.org/software/systemd/man/udev.html for help correcting the errors and warnings.");
                } else if (id == REMEDY_UNAPPROVED_LICENSE) {
                    r = _("The specified license abbreviation is not listed as approved in the license database.  The license database is specified in the rpminspect configuration file.  Check this file and send a pull request to the appropriate upstream project to update the database.  If the license is listed in the database but marked unapproved, you may need to work with the legal team regarding options for this software.");
                } else if (id == REMEDY_UNICODE_PREP_FAILED) {
                    r = _("The %prep section of the spec file could not be executed for some reason.  This usually results from a failure in librpmbuild, which is usually tied to archive extraction problems or the filesystem changing while rpminspect is running.  A common cause is removal of the working directory while the program is executing.");
                } else if (id == REMEDY_UNICODE) {
                    r = _("The rpminspect configuration file contains a list of forbidden Unicode code points.  One was found in the extracted and patched source tree or in one of the text source files in the source RPM.  Either remove this code point or discuss the situation with the Product Security Team to determine the correct course of action.");
                } else if (id == REMEDY_UPSTREAM) {
                    r = _("Unexpected changed source archive content. The version of the package did not change between builds, but the source archive content did. This may be deliberate, but needs inspection. If this change is expected, update the rebaseable exception list for this product release and send a patch to the project that owns the rpminspect data files.");
                } else if (id == REMEDY_VENDOR) {
                    r = _("Change the string specified on the 'Vendor:' line in the spec file.");
                } else if (id == REMEDY_VIRUS) {
                    r = _("ClamAV has found a virus in the named file.  This may be a false positive, but you should manually inspect the file in question to ensure it is clean.  This may be a problem with the ClamAV database or detection.  If you are sure the file in question is clean, please file a bug with rpminspect for further help.");
                } else if (id == REMEDY_XML) {
                    r = _("Correct the reported errors in the XML document.");
                } else if (id == REMEDY_MIXED_LICENSE_TAGS) {
                    r = _("The License tag contains mixed used of SPDX and legacy license identifiers.  You must use either all SPDX license identifiers or all legacy license identifiers; you cannot mix the two systems.");
                } else if (id == REMEDY_FILESMATCH_EXCLUDE_FOUND) {
                    r = _("A file or directory was found in the package that matches an %%exclude definition in the spec file.  You need to ensure the files and directories that should be in the package are properly specified in the %%files section(s) in the spec file.");
                } else if (id == REMEDY_FILESMATCH_UNSPECIFIED_ENTRY) {
                    r = _("An unexpected file or directory was found in the package.  This file or directory does not match any definition from the %%files section(s) in the spec file.  You need to remove it from the built package (by modifying the %%install section of the spec file and rebuilding the package) or update the appropriate %%files section to include it.");
                }
            }
        }

        if (r) {
            break;
        }
    }

    return r;
}

/*
 * Set a remedy override string from the config file.  Returns true if
 * the remedy identifier was valid, false otherwise.
 */
bool set_remedy(const char *name, char *remedy)
{
    unsigned int i = 0;

    if (name == NULL) {
        return false;
    }

    for (i = 0; remedies[i].name != NULL; i++) {
        if (!strcmp(remedies[i].name, name)) {
            free(remedies[i].remedy);
            remedies[i].remedy = remedy;
            return true;
        }
    }

    return false;
}

/*
 * Any remedy strings that were read in from config files need to be
 * freed before exit.
 */
void free_remedy_strings(void) {
    unsigned int i = 0;

    for (i = 0; remedies[i].name != NULL; i++) {
        free(remedies[i].remedy);
    }

    return;
}
