/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
 * Author(s):  David Shea <dshea@redhat.com>
 *             David Cantrell <dcantrell@redhat.com>
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

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <rpm/header.h>
#include <rpm/rpmtag.h>

#include "inspect.h"
#include "rpminspect.h"

/*
 * By default, libxml will send error messages to stderr.  Turn that off for
 * our purposes.
 */
static void xml_silence_errors(void *ctx __attribute__((unused)), const char *msg __attribute__((unused)), ...)
{
    return;
}

/*
 * Return true if the given file is a well-formed XML document, false otherwise.
 * This function first tries with DTD validation.  Failing that it tries to just
 * check the XML.  The tests get less and less strict.
 */
static bool is_xml_well_formed(const char *path, char **errors)
{
    static bool initialized = false;
    static xmlGenericErrorFunc silence = xml_silence_errors;
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    bool result;

    if (!initialized) {
        initGenericErrorDefaultFunc(&silence);
        LIBXML_TEST_VERSION
        initialized = true;
    }

    ctxt = xmlNewParserCtxt();
    assert(ctxt != NULL);
    doc = xmlCtxtReadFile(ctxt, path, NULL, XML_PARSE_RECOVER | XML_PARSE_NONET | XML_PARSE_DTDVALID);
    DEBUG_PRINT("path=|%s|, ctxt->valid=%d, ctxt->errNo=%d\n", path, ctxt->valid, ctxt->errNo);

    /* try again if no DTD specified */
    if (!ctxt->valid && ctxt->errNo == XML_DTD_NO_DTD) {
        xmlFreeDoc(doc);
        doc = xmlCtxtReadFile(ctxt, path, NULL, XML_PARSE_RECOVER | XML_PARSE_NONET);
        DEBUG_PRINT("path=|%s|, ctxt->valid=%d, ctxt->errNo=%d\n", path, ctxt->valid, ctxt->errNo);
    }

    /* an unparsed entity is ok in this check */
    if (ctxt->errNo == XML_ERR_UNDECLARED_ENTITY || ctxt->errNo == XML_WAR_UNDECLARED_ENTITY) {
        result = true;
    } else {
        if (!ctxt->valid) {
            if (errors != NULL) {
                *errors = strdup(ctxt->lastError.message);
            }

            result = false;
        } else {
            result = true;
        }
    }

    if (doc != NULL) {
        xmlFreeDoc(doc);
    }

    xmlFreeParserCtxt(ctxt);
    return result;
}

static bool is_xml(const char *path)
{
    FILE *input;
    unsigned char buffer[32];
    unsigned char *xml_data;
    size_t bytes_read;

    const char xml_ascii_prelude[] = "<?xml version=";
    const char xml_utf16_le_prelude[] = "<\0?\0x\0m\0l\0 \0v\0e\0r\0s\0i\0o\0n\0=\0";
    const char xml_utf16_be_prelude[] = "\0<\0?\0x\0m\0l\0 \0v\0e\0r\0s\0i\0o\0n\0=";
    const char *xml_prelude;
    size_t min_size;

    input = fopen(path, "r");

    if (input == NULL) {
        return false;
    }

    /* Look for an optional byte-order marker, followed by "<?xml version=" */
    bytes_read = fread(buffer, 1, sizeof(buffer), input);

    if (ferror(input)) {
        fclose(input);
        return false;
    }

    fclose(input);
    xml_data = buffer;

    /* Look for a byte-order marker */
    /* The XML spec says everyone has to deal with at least utf-8 and utf-16, so handle those */
    if ((bytes_read >= 3) && (buffer[0] == 0xEF) && (buffer[1] == 0xBB) && (buffer[2] == 0xBF)) {
        /* utf-8? */
        xml_data += 3;
        bytes_read -= 3;
        xml_prelude = xml_ascii_prelude;
        min_size = sizeof(xml_ascii_prelude) - 1;
    } else if ((bytes_read >= 2) && (buffer[0] == 0xFE) && (buffer[1] == 0xFF)) {
        /* utf-16 LE? */
        xml_data += 2;
        bytes_read -= 2;
        xml_prelude = xml_utf16_le_prelude;
        min_size = sizeof(xml_utf16_le_prelude) - 1;
    } else if ((bytes_read >= 2) && (buffer[0] == 0xFF) && (buffer[1] == 0xFE)) {
        /* utf-16 BE? */
        xml_data += 2;
        bytes_read -= 2;
        xml_prelude = xml_utf16_be_prelude;
        min_size = sizeof(xml_utf16_be_prelude) - 1;
    } else {
        /* otherwise just assume something close enough to ascii */
        xml_prelude = xml_ascii_prelude;
        min_size = sizeof(xml_ascii_prelude) - 1;
    }

    return (bytes_read >= min_size) && (memcmp(xml_data, xml_prelude, min_size) == 0);
}

static bool xml_driver(struct rpminspect *ri, rpmfile_entry_t *file)
{
    bool result = true;
    struct result_params params;

    /* Skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* Is this an XML file? */
    if (!file->fullpath || !S_ISREG(file->st.st_mode)) {
        return true;
    }

    if (!process_file_path(file, ri->xml_path_include, ri->xml_path_exclude)) {
        return true;
    }

    if (!is_xml(file->fullpath)) {
        return true;
    }

    if (strsuffix(file->fullpath, SVG_FILENAME_EXTENSION)) {
        /*
         * Skip SVG files which are XML, but don't specify a DTD
         * We don't validate other image files so we can probably do the
         * same for SVG.
         */
        return true;
    }

    /* Set up result parameters */
    init_result_params(&params);
    params.severity = RESULT_VERIFY;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.header = HEADER_XML;
    params.remedy = REMEDY_XML;
    params.arch = get_rpm_header_arch(file->rpm_header);
    params.file = file->localpath;

    result = is_xml_well_formed(file->fullpath, &params.details);

    if (!result) {
        xasprintf(&params.msg, _("File %s is a malformed XML file on %s"), file->localpath, params.arch);
        add_result(ri, &params);
        free(params.msg);
        free(params.details);
    }

    return result;
}

bool inspect_xml(struct rpminspect *ri)
{
    bool result;
    struct result_params params;

    assert(ri != NULL);
    result = foreach_peer_file(ri, xml_driver, true);

    if (result) {
        init_result_params(&params);
        params.severity = RESULT_OK;
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_XML;
        add_result(ri, &params);
    }

    return result;
}
