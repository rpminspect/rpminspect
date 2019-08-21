/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Author(s):  David Shea <dshea@redhat.com>
 *             David Cantrell <dcantrell@redhat.com>
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
#include <string.h>
#include <sys/stat.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <rpm/header.h>
#include <rpm/rpmtag.h>

#include "inspect.h"
#include "rpminspect.h"

/*
 * Return true if the given file is a well-formed XML document, false otherwise.
 * This only checks if the XML is well-formed. No validation is performed.
 */
bool is_xml_well_formed(const char *path, char **errors)
{
    static bool initialized = false;
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    bool result;

    if (!initialized) {
        LIBXML_TEST_VERSION
        initialized = true;
    }

    ctxt = xmlNewParserCtxt();
    assert(ctxt != NULL);
    doc = xmlCtxtReadFile(ctxt, path, NULL, XML_PARSE_PEDANTIC);

    if (!ctxt->valid) {
        if (errors != NULL) {
            *errors = strdup(ctxt->lastError.message);
        }

        result = false;
    } else {
        result = true;
    }

    if (doc != NULL) {
        xmlFreeDoc(doc);
    }

    xmlFreeParserCtxt(ctxt);

    return result;;
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
    char *errors = NULL;
    char *msg = NULL;
    bool result;

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

    result = is_xml_well_formed(file->fullpath, &errors);

    if (!result) {
        xasprintf(&msg, "File %s has become malformed XML on %s", file->localpath, headerGetString(file->rpm_header, RPMTAG_ARCH));

        add_result(&ri->results, RESULT_VERIFY, WAIVABLE_BY_ANYONE, HEADER_XML, msg, errors, REMEDY_XML);

        free(msg);
    }

    free(errors);
    return result;
}

bool inspect_xml(struct rpminspect *ri)
{
    bool result;

    assert(ri != NULL);
    result = foreach_peer_file(ri, xml_driver);

    if (result) {
        add_result(&ri->results, RESULT_OK, NOT_WAIVABLE, HEADER_XML, NULL, NULL, NULL);
    }

    return result;
}
