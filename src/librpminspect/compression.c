/*
 * Copyright (C) 2019  Red Hat, Inc.
 * Author(s):  David Shea <dshea@redhat.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <zlib.h>

/*
 * Uncompress the given buffer using zlib. Returns -1 error.
 *
 * On success, the output buffer must be freed by the caller.
 */
int inflate_data(const char *input, size_t input_size, char **output, size_t *output_size)
{
    struct z_stream_s stream = {0};
    Bytef stream_buffer[BUFSIZ];
    FILE *output_file;
    int result;

    assert(input);
    assert(output);
    assert(output_size);

    stream.next_in = (z_const Bytef *) input;
    stream.avail_in = input_size;

    /* 15 is the default window size, + 32 enables automatic header detection */
    if (inflateInit2(&stream, 15 + 32) != Z_OK) {
        return -1;
    }

    /* Use open_memstream to allocate the memory we need */
    if ((output_file = open_memstream(output, output_size)) == NULL) {
        return -1;
    }

    do {
        stream.next_out = stream_buffer;
        stream.avail_out = sizeof(stream_buffer);
        stream.total_out = 0;

        result = inflate(&stream, Z_NO_FLUSH);
        if ((result != Z_OK) && (result != Z_STREAM_END)) {
            inflateEnd(&stream);
            fclose(output_file);
            free(*output);
            *output = NULL;
            return -1;
        }

        if (stream.total_out > 0) {
            if (fwrite(stream_buffer, 1, stream.total_out, output_file) != stream.total_out) {
                inflateEnd(&stream);
                fclose(output_file);
                free(*output);
                *output = NULL;
                return -1;
            }
        }
    } while (stream.avail_in > 0);

    fclose(output_file);
    return 0;
}
