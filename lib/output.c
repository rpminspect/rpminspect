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

#include <stddef.h>
#include "rpminspect.h"

/*
 * Ensure the array of output formats is only defined once.
 */

struct format formats[] = {
    { FORMAT_TEXT, "text", &output_text },
    { FORMAT_JSON, "json", &output_json },
    { -1, NULL, NULL }
};

const char *format_desc(unsigned int format)
{
    switch (format) {
        case FORMAT_TEXT:
            return _("Plain text suitable for the console and piping through paging programs.");
        case FORMAT_JSON:
            return _("Results organized as a JSON data structure suitable for reading by web applications and other frontend tools.");
        default:
            return NULL;
    }
}
