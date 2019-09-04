/*
 * Copyright (C) 2018-2019  Red Hat, Inc.
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
 * This header includes constants and other defaults.
 */

#ifndef _LIBRPMINSPECT_CONSTANTS_H
#define _LIBRPMINSPECT_CONSTANTS_H

/*
 * Software name.  Used in Koji XMLRPC calls and logging.
 */
#define SOFTWARE_NAME "librpminspect"

/*
 * Constant pathnames we need to use throughout the source.
 */
#define CFGFILE "/etc/rpminspect/rpminspect.conf"

/*
 * Defaults, when the configuration file does not provide a
 * value we can use.
 */
#define DEFAULT_WORKDIR "/var/tmp/rpminspect"

/*
 * Standard location for the license database.  Can be changed
 * in the configuration file.
 */
#define LICENSE_DB_FILE "/usr/share/rpminspect/licenses/generic.json"

/*
 * Standard location for stat-whitelist files per product release.
 */
#define STAT_WHITELIST_DIR "/usr/share/rpminspect/stat-whitelist"

/*
 * Standard location for desktop entry files.
 */
#define DESKTOP_ENTRY_FILES_DIR "/usr/share/applications"

/*
 * Default location of desktop-file-validate(1)
 */
#define DESKTOP_FILE_VALIDATE "/usr/bin/desktop-file-validate"

/*
 * Architecture name of source RPMs (from Koji)
 */
#define SRPM_ARCH_NAME "src"

/*
 * File extensions
 */
#define RPM_FILENAME_EXTENSION ".rpm"
#define SPEC_FILENAME_EXTENSION ".spec"
#define JAR_FILENAME_EXTENSION ".jar"
#define CLASS_FILENAME_EXTENSION ".class"
#define EGGINFO_FILENAME_EXTENSION ".egg-info"
#define GZIPPED_FILENAME_EXTENSION ".gz"
#define DESKTOP_FILENAME_EXTENSION ".desktop"
#define DIRECTORY_FILENAME_EXTENSION ".directory"
#define MO_FILENAME_EXTENSION ".mo"

/*
 * Build ID
 */
#define BUILD_ID_DIR "/.build-id/"

/*
 * Debug packages
 */
#define DEBUGINFO_SUFFIX "-debuginfo"
#define DEBUGSOURCE_SUFFIX "-debugsource"

#endif
