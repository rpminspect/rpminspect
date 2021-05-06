/*
 * Copyright © 2018 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
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
 * @file constants.h
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2018-2020
 * @brief Constants and other defaults for librpminspect.
 * @copyright LGPL-3.0-or-later
 */

#ifndef _LIBRPMINSPECT_CONSTANTS_H
#define _LIBRPMINSPECT_CONSTANTS_H

/**
 * @defgroup CONSTANTS Constants and Other Defaults
 *
 * @{
 */

/**
 * @defgroup CONFIG Configuration defaults
 *
 * @{
 */

/**
 * @def COMMAND_NAME
 * The standard name for the rpminspect command.
 */
#define COMMAND_NAME "rpminspect"

/**
 * @def SOFTWARE_NAME
 * Software name.  Used in Koji XMLRPC calls and logging.
 */
#define SOFTWARE_NAME "librpminspect"

/**
 * @def CFGFILE_DIR
 * Configuration file directory.
 */
#define CFGFILE_DIR "/usr/share/rpminspect"

/**
 * @def CFGFILE
 * Default configuration file.
 */
#define CFGFILE "rpminspect.yaml"

/**
 * @def DEFAULT_WORKDIR
 * Default working directory location.  rpminspect will create
 * subdirectories within this directory so multiple concurrent jobs
 * can run.
 */
#define DEFAULT_WORKDIR "/var/tmp/rpminspect"

/**
 * @def VENDOR_DATA_DIR
 * Default location for the vendor-specific data.  These files are
 * provided by the vendor-specific rpminspect-data package.
 */
#define VENDOR_DATA_DIR "/usr/share/rpminspect"

/**
 * @def INSPECTIONS
 * Name of the [inspections] section in the config file.
 */
#define INSPECTIONS "inspections"

/** @} */

/**
 * @defgroup VENDOR_DATA_SUBDIRS Vendor Data Subdirectories
 *
 * @{
 */

/**
 * @def ABI_DIR
 * Name of the ABI checking subdirectory in VENDOR_DATA_DIR.
 */
#define ABI_DIR "abi"

/**
 * @def CAPABILITIES_DIR
 * Name of the capabilities(7) subdirectory in VENDOR_DATA_DIR.
 */
#define CAPABILITIES_DIR "capabilities"

/**
 * @def LICENSES_DIR
 * Name of the license database subdirectory in VENDOR_DATA_DIR.
 */
#define LICENSES_DIR "licenses"

/**
 * @def FILEINFO_DIR
 * Name of the fileinfo (stat(2)) list subdirectory in VENDOR_DATA_DIR.
 */
#define FILEINFO_DIR "fileinfo"

/**
 * @def REBASEABLE_DIR
 * Name of the VENDOR_DATA_DIR subdirectory listing rebaseable packages.
 */
#define REBASEABLE_DIR "rebaseable"

/**
 * @def POLITICS_DIR
 * Name of the VENDOR_DATA_DIR subdirectory with political related
 * file inclusion and exclusion rules.
 */
#define POLITICS_DIR "politics"

/** @} */

/**
 * @defgroup DESKTOP Desktop Constants
 * Constants related to desktop files.  Paths and where to find icons,
 * for instance.
 * @{
 */

/**
 * @def DESKTOP_ENTRY_FILES_DIR
 * Standard location for desktop entry files.
 */
#define DESKTOP_ENTRY_FILES_DIR "/usr/share/applications"

/** @} */

/**
 * @defgroup COMMANDS Commands used by different inspections
 * Avoid explicit paths, rpminspect assumes these commands are
 * available in the $PATH.
 *
 * @{
 */

/**
 * @def MSGUNFMT_CMD
 * Executable providing msgunfmt(1)
 */
#define MSGUNFMT_CMD "msgunfmt"

/**
 * @def DIFF_CMD
 * Executable providing diff(1).  NOTE: This should be GNU diff or
 * 100% compatible.
 */
#define DIFF_CMD "diff"

/**
 * @def DIFFSTAT_CMD
 * Executable providing diffstat(1).
 */
#define DIFFSTAT_CMD "diffstat"

/**
 * @def DESKTOP_FILE_VALIDATE_CMD
 * Executable providing desktop-file-validate(1)
 */
#define DESKTOP_FILE_VALIDATE_CMD "desktop-file-validate"

/**
 * @def ANNOCHECK_CMD
 * Executable providing annocheck(1)
 */
#define ANNOCHECK_CMD "annocheck"

/**
 * @def ABIDIFF_CMD
 * Executable providing abidiff(1)
 */
#define ABIDIFF_CMD "abidiff"
#define ABI_SUPPRESSIONS "--suppressions"
#define ABI_DEBUG_INFO_DIR1 "--debug-info-dir1"
#define ABI_DEBUG_INFO_DIR2 "--debug-info-dir2"
#define ABI_HEADERS_DIR1 "--headers-dir1"
#define ABI_HEADERS_DIR2 "--headers-dir2"

/**
 * @def KMIDIFF_CMD
 * Executable providing kmidiff(1)
 */
#define KMIDIFF_CMD "kmidiff"
#define KMIDIFF_VMLINUX1 "--vmlinux1"
#define KMIDIFF_VMLINUX2 "--vmlinux2"
#define KMIDIFF_KMI_WHITELIST "--kmi-whitelist"

/** @} */

/**
 * @defgroup SYSTEM System defaults
 *
 * @{
 */

/**
 * @def SRPM_ARCH_NAME
 * The architecture name Koji uses for source RPMs.
 */
#define SRPM_ARCH_NAME "src"

/**
 * @def RPM_NOARCH_NAME
 * The architecture name Koji uses for binary RPMs built with
 * 'BuildArch: noarch'
 */
#define RPM_NOARCH_NAME "noarch"

/**
 * @def BIN_OWNER
 * Executable file owner
 */
#define BIN_OWNER "root"

/**
 * @def BIN_GROUP
 * Executable file group
 */
#define BIN_GROUP "root"

/**
 * @def BUILD_ID_DIR
 * Build ID subdirectory name
 */
#define BUILD_ID_DIR "/.build-id/"

/*
 * @def DEBUGINFO_SUFFIX
 * debuginfo package name suffix string
 */
#define DEBUGINFO_SUFFIX "-debuginfo"

/**
 * @def DEBUGSOURCE_SUFFIX
 * debugsource package name suffix string
 */
#define DEBUGSOURCE_SUFFIX "-debugsource"

/**
 * @def DEBUG_PATH
 * debuginfo installed path
 */
#define DEBUG_PATH "/usr/lib/debug/"

/**
 * @def DEBUG_SRC_PATH
 * debugsource installed path
 */
#define DEBUG_SRC_PATH "/usr/src/debug/"

/**
 * @def DEBUG_SUBSTRING
 * The substring that can appear in filenames installed with debugging
 * information.
 */
#define DEBUG_SUBSTRING "debug"

/**
 * @def KERNEL_MODULES_DIR
 * Linux loadable kernel modules subdirectory
 */
#define KERNEL_MODULES_DIR "/lib/modules/"

/**
 * @def ABIDIFF_SUPPRESSION_FILE
 * Default ABI suppression file in SRPMs
 */
#define ABI_SUPPRESSION_FILE ".abignore"

/**
 * @def INCLUDE_PATH
 * Default header file path.
 */
#define INCLUDE_PATH "/usr/include/"

/**
 * @def KERNEL_FILENAMES
 * Default array of kernel executable filename possibilities.
 */
#define KERNEL_FILENAMES { "vmlinux", "vmlinuz", NULL }

/**
 * @def KMI_IGNORE_PATTERN
 * File pattern used to find the KMI ignore list for kmidiff.
 * The substring '${ARCH}' will be replaced with the package architecture.
 */
#define KMI_IGNORE_PATTERN "/lib/modules/kabi-current/kabi_whitelist_${ARCH}"

/** @} */

/**
 * @defgroup EXTENSIONS File extensions
 *
 * @{
 */

/**
 * @def RPM_FILENAME_EXTENSION
 * RPM filename extension
 */
#define RPM_FILENAME_EXTENSION ".rpm"

/**
 * @def SPEC_FILENAME_EXTENSION
 * RPM spec filename extension
 */
#define SPEC_FILENAME_EXTENSION ".spec"

/**
 * @def JAR_FILENAME_EXTENSION
 * Java jar filename extension
 */
#define JAR_FILENAME_EXTENSION ".jar"

/**
 * @def CLASS_FILENAME_EXTENSION
 * Java class filename extension
 */
#define CLASS_FILENAME_EXTENSION ".class"

/**
 * @def EGGINFO_FILENAME_EXTENSION
 * Python egg-info filename extension
 */
#define EGGINFO_FILENAME_EXTENSION ".egg-info"

/**
 * @def GZIPPED_FILENAME_EXTENSION
 * Gzip filename extension
 */
#define GZIPPED_FILENAME_EXTENSION ".gz"

/**
 * @def DESKTOP_FILENAME_EXTENSION
 * Desktop filename extension
 */
#define DESKTOP_FILENAME_EXTENSION ".desktop"

/**
 * @def DIRECTORY_FILENAME_EXTENSION
 * Directory filename extension
 */
#define DIRECTORY_FILENAME_EXTENSION ".directory"

/**
 * @def MO_FILENAME_EXTENSION
 * Machine object filename extension (compiled translation data)
 */
#define MO_FILENAME_EXTENSION ".mo"

/**
 * @def PYTHON_PYC_FILE_EXTENSION
 * Python bytecode filename extension
 */
#define PYTHON_PYC_FILE_EXTENSION ".pyc"

/**
 * @def PYTHON_PYO_FILE_EXTENSION
 * Python optimized bytecode filename extension
 */
#define PYTHON_PYO_FILE_EXTENSION ".pyo"

/**
 * @def KERNEL_MODULE_FILENAME_EXTENSION
 * Linux loadable kernel module filename extension
 */
#define KERNEL_MODULE_FILENAME_EXTENSION ".ko"

/**
 * @def SVG_FILENAME_EXTENSION
 * Scalable vector graphics filename extension
 */
#define SVG_FILENAME_EXTENSION ".svg"

/**
 * @def STATIC_LIB_FILENAME_EXTENSION
 * Static ELF library filename extension
 */
#define STATIC_LIB_FILENAME_EXTENSION ".a"

/**
 * @def ELF_LIB_EXTENSION
 * ELF shared library extension.  Note that this extension appears in
 * the middle of ELF library filenames because a version number comes
 * after it.  For files that end with just '.so', we do not care so
 * much because rpminspect will match the files correctly.  That's why
 * this extension ends with a period.
 */
#define ELF_LIB_EXTENSION ".so."

/** @} */

#define SPEC_MACRO_DEFINE "%define"
#define SPEC_MACRO_GLOBAL "%global"
#define SPEC_SECTION_CHANGELOG "%changelog"
#define SPEC_SECTION_FILES "%files"
#define SPEC_TAG_RELEASE "Release:"
#define SPEC_DISTTAG "%{?dist}"

/** @} */

/*
 * Default ABI compat level security reporting threshold
 */
#define DEFAULT_ABI_SECURITY_THRESHOLD 2

/*
 * Default file count reporting threshold for 'patches'
 */
#define DEFAULT_PATCH_FILE_THRESHOLD 20

/*
 * Default line count reporting threshold for 'patches'
 */
#define DEFAULT_PATCH_LINE_THRESHOLD 5000

/*
 * Default message digest to use internally.  The definition comes
 * from an enum in rpminspect.h
 */
#define DEFAULT_MESSAGE_DIGEST SHA256SUM

/*
 * The value of the DT_RPATH or DT_RUNPATH $ORIGIN string.
 */
#define RUNPATH_ORIGIN_STR "$ORIGIN"

#endif
