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
 * This header defines the results constants.
 */

#ifndef _LIBRPMINSPECT_RESULTS_H
#define _LIBRPMINSPECT_RESULTS_H

/*
 * Inspection headers
 */
#define HEADER_RPMINSPECT    "rpminspect"
#define HEADER_METADATA      "header-metadata"
#define HEADER_EMPTYRPM      "empty-payload"
#define HEADER_LOSTPAYLOAD   "lost-payload"
#define HEADER_LICENSE       "license"
#define HEADER_ELF           "elf-object-properties"
#define HEADER_MAN           "man-pages"
#define HEADER_XML           "xml-files"
#define HEADER_DESKTOP       "desktop-entry-files"
#define HEADER_DISTTAG       "dist-tag"
#define HEADER_SPECNAME      "spec-file-name"
#define HEADER_MODULARITY    "modularity"
#define HEADER_JAVABYTECODE  "java-bytecode"
#define HEADER_CHANGEDFILES  "changed-files"
#define HEADER_MOVEDFILES    "moved-files"
#define HEADER_REMOVEDFILES  "removed-files"
#define HEADER_ADDEDFILES    "added-files"
#define HEADER_UPSTREAM      "upstream"
#define HEADER_OWNERSHIP     "ownership"
#define HEADER_SHELLSYNTAX   "shell-syntax"
#define HEADER_ANNOCHECK     "annocheck"
#define HEADER_DT_NEEDED     "DT_NEEDED"
#define HEADER_FILESIZE      "filesize"
#define HEADER_PERMISSIONS   "permissions"
#define HEADER_CAPABILITIES  "capabilities"
#define HEADER_KMOD          "kernel-modules"
#define HEADER_ARCH          "architectures"
#define HEADER_SUBPACKAGES   "subpackages"
#define HEADER_CHANGELOG     "changelog"
#define HEADER_PATHMIGRATION "path-migration"
#define HEADER_LTO           "LTO"
#define HEADER_SYMLINKS      "symlinks"
#define HEADER_FILES         "%files"
#define HEADER_TYPES         "types"

/*
 * Inspection remedies
 */
/* metadata */
#define REMEDY_VENDOR _("Change the string specified on the 'Vendor:' line in the spec file.")
#define REMEDY_BUILDHOST _("Make sure the SRPM is built on a host within the expected subdomain.")
#define REMEDY_BADWORDS _("Unprofessional language as defined in the configuration file was found in the text shown.  Remove or change the offending words and rebuild.")

/* emptyrpm */
#define REMEDY_EMPTYRPM _("Check to see if you eliminated a subpackage but still have the %package and/or the %files section for it.")

/* lostpayload */
#define REMEDY_LOSTPAYLOAD _("Check to see if you eliminated a subpackage but still have the %package and/or the %files section for it.")

/* license */
#define REMEDY_LICENSE _("The License tag must contain an approved license string as defined by the distribution (e.g., GPLv2+).  If the license in question is approved, the license database needs updating in the rpminspect-data package.")
#define REMEDY_LICENSEDB _("Make sure the licensedb setting in the rpminspect configuration is set to a valid licensedb file.  This is also commonly due to a missing vendor specific rpminspect-data package on the system.")
#define REMEDY_UNAPPROVED_LICENSE _("The specified license abbreviation is not listed as approved in the license database.  The license database is specified in the rpminspect configuration file.  Check this file and send a pull request to the appropriate upstream project to update the database.  If the license is listed in the database but marked unapproved, you may need to work with the legal team regarding options for this software.")

/* elf */
#define REMEDY_ELF_TEXTREL _("Ensure all object files are compiled with -fPIC")
#define REMEDY_ELF_EXECSTACK_MISSING _("Ensure that the package is being built with the correct compiler and compiler flags")
#define REMEDY_ELF_EXECSTACK_INVALID _("The data in an ELF file appears to be corrupt; ensure that packaged ELF files are not being truncated or incorrectly modified")
#define REMEDY_ELF_EXECSTACK_EXECUTABLE _("An ELF stack is marked as executable. Ensure that no execstack options are being passed to the linker, and that no functions are defined on the stack.")
#define REMEDY_ELF_GNU_RELRO _("Ensure executables are linked with with '-z relro -z now'")
#define REMEDY_ELF_FORTIFY_SOURCE _("Ensure all object files are compiled with '-O2 -D_FORTIFY_SOURCE=2', and that all appropriate headers are included (no implicit function declarations). Symbols may also appear as unfortified if the compiler is unable to determine the size of a buffer, which is not necessarily an error.")
#define REMEDY_ELF_FPIC _("Ensure all object files are compiled with -fPIC")
#define REMEDY_ELF_IPV6 _("Please review all usages of IPv4-only functions and ensure IPv6 compliance.")

/* man */
#define REMEDY_MAN_ERRORS _("Correct the errors in the man page as reported by the libmandoc parser.")
#define REMEDY_MAN_PATH _("Correct the installation path for the man page. Man pages must be installed in the directory beneath /usr/share/man that matches the section number of the page.")

/* xml */
#define REMEDY_XML _("Correct the reported errors in the XML document")

/* desktop */
#define REMEDY_DESKTOP _("Refer to the Desktop Entry Specification at https://standards.freedesktop.org/desktop-entry-spec/latest/ for help correcting the errors and warnings")

/* disttag */
#define REMEDY_DISTTAG _("The Release: tag in the spec file must include a '%{?dist}' string.  Please add this to the spec file per the distribution packaging guidelines.")

/* specname */
#define REMEDY_SPECNAME _("The spec file name does not match the expected NAME.spec format.  Rename the spec file to conform to this policy.")

/* modularity */
#define REMEDY_MODULARITY _("This package is part of a module but is missing the %{modularitylabel} header tag.  Add this as a %define in the spec file and rebuild.")

/* javabytecode */
#define REMEDY_JAVABYTECODE _("The Java bytecode version for one or more class files in the build was not met for the product release.  Ensure you are using the correct JDK for the build.")

/* changedfiles */
#define REMEDY_CHANGEDFILES _("Unexpected file changes were found.  Verify these changes are correct.  If they are not, adjust the build to prevent file changes.")

/* movedfiles */
#define REMEDY_MOVEDFILES _("Unexpected file moves were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file moves between builds.")

/* removedfiles */
#define REMEDY_REMOVEDFILES _("Unexpected file removals were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file removals between builds.")

/* addedfiles */
#define REMEDY_ADDEDFILES _("Unexpected file additions were found.  Verify these changes are correct.  If they are not, adjust the build to prevent the file additions between builds.")

/* upstream */
#define REMEDY_UPSTREAM _("Unexpected changed source archive content.  The version of the package did not change between builds, but the source archive content did.  This may be deliberate, but needs inspection.")

/* ownership */
#define REMEDY_OWNERSHIP_DEFATTR _("Make sure the %files section includes the %defattr macro.")
#define REMEDY_OWNERSHIP_BIN_OWNER _("Bin path files must be owned by the bin_owner set in the rpminspect configuration, which is usually root.")
#define REMEDY_OWNERSHIP_BIN_GROUP _("Bin path files must be owned by the bin_group set in the rpminspect configuration, which is usually root.")
#define REMEDY_OWNERSHIP_IXOTH _("Either chgrp the file to the bin_group set in the rpminspect configuration or remove the world execute bit on the file (chmod o-x).")
#define REMEDY_OWNERSHIP_IWGRP _("Either chgrp the file to the bin_group set in the rpminspect configuration or remove the group write bit on the file (chmod g-w).")
#define REMEDY_OWNERSHIP_CHANGED _("Verify the ownership changes are expected. If not, adjust the package build process to set correct owner and group information.")

/* shellsyntax */
#define REMEDY_SHELLSYNTAX _("Consult the shell documentation for proper syntax.")
#define REMEDY_SHELLSYNTAX_GAINED_SHELL _("The file referenced was not a known shell script in the before build but is now a shell script in the after build.")
#define REMEDY_SHELLSYNTAX_BAD _("The referenced shell script is invalid. Consider debugging it with the '-n' option on the shell to find and fix the problem.")

/* annocheck */
#define REMEDY_ANNOCHECK _("See annocheck(1) for more information.")

/* DT_NEEDED */
#define REMEDY_DT_NEEDED _("DT_NEEDED symbols have been added or removed.  This happens when the build environment has different versions of the required libraries.  Sometimes this is deliberate but sometimes not.  Verify these changes are expected.  If they are not, modify the package spec file to ensure the build links with the correct shared libraries.")

/* filesize */
#define REMEDY_FILESIZE_BECAME_NOT_EMPTY _("A previously empty file is no longer empty.  Make sure this change is intended and fix the package spec file if necessary.")
#define REMEDY_FILESIZE_BECAME_EMPTY _("A previously non-empty file is now empty.  Make sure this change is intended and fix the package space file if necessary.")

/* capabilities */
#define REMEDY_CAPABILITIES _("Unexpected capabilities were found on the indicated file.  Consult capabilities(7) and either adjust the files in the package or modify the capabilities list in the rpminspect vendor data package.  The security team may also be of help for this situation.")

/* kmod */
#define REMEDY_KMOD_PARM _("Kernel module parameters were removed between builds.  This may present usability problems for users if module parameters were removed in a maintenance update.")
#define REMEDY_KMOD_DEPS _("Kernel module dependencies changed between builds.  This may present usability problems for users if module dependencies changed in a maintenance update.")
#define REMEDY_KMOD_ALIAS _("Kernel module device aliases changed between builds.  This may present usability problems for users if module device aliases changed in a maintenance update.")

/* arch */
#define REMEDY_ARCH_LOST _("An architecture present in the before build is now missing in the after build.  This may be deliberate, but check to make sure you do not have any unexpected ExclusiveArch lines in the spec file.")
#define REMEDY_ARCH_GAIN _("A new architecture has appeared in the after build.  This may indicate progress in the world of computing.")

/* subpackages */
#define REMEDY_SUBPACKAGES_LOST _("A subpackage present in the before build is now missing in the after build.  This may be deliberate, but check to make sure you have correct syntax defining the subpackage in the spec file")
#define REMEDY_SUBPACKAGES_GAIN _("A new subpackage has appeared in the after build.  This may indicate progress in the world of computing.")

/* changelog */
#define REMEDY_CHANGELOG _("Make sure the spec file in the after build contains a valid %changelog section.")

/* pathmigration */
#define REMEDY_PATHMIGRATION _("Files should not be installed in old directory names.  Modify the package to install the affected file to the preferred directory.")

/* lto */
#define REMEDY_LTO _("ELF .o and .a files should not carry LTO (Link Time Optimization) bytecode.  Make sure you have stripped LTO bytecode from those files at install time.")

/* symlinks */
#define REMEDY_SYMLINKS _("Make sure symlinks point to a valid destination in one of the subpackages of the build; dangling symlinks are not allowed.  If you are comparing builds and have a non-symlink turn in to a symlink, ensure this is deliberate.  NOTE:  You cannot turn a directory in to a symlink due to RPM limitations.");
#define REMEDY_SYMLINKS_DIRECTORY _("Make sure symlinks point to a valid destination in one of the subpackages of the build; dangling symlinks are not allowed.  If you are comparing builds and have a non-symlink turn in to a symlink, ensure this is deliberate.  NOTE:  You cannot turn a directory in to a symlink due to RPM limitations.  If you absolutely must do that, make sure you include the %pretrans scriptlet for replacing a directory.  See the packaging guidelines for 'Scriptlet to replace a directory' for more information.");

/* %files */
#define REMEDY_FILES _("Remove forbidden path references from the indicated line in the %files section.  In many cases you can use RPM macros to specify path locations.  See the RPM documentation or distribution package maintainer guide for more information.")

/* types */
#define REMEDY_TYPES _("In many cases the changing MIME type is deliberate.  Verify that the change is intended and if necessary fix the spec file so the correct file is included in the built package.")

#endif
