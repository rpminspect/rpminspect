/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <err.h>
#include "rpminspect.h"

/*
 * Returns true if the specified file is LLVM IR Bitcode, false
 * otherwise.
 */
bool is_llvm_ir_bitcode(const char *file)
{
    int fd;
    int flags = O_RDONLY | O_CLOEXEC;
    char magic[4];

    assert(file != NULL);

    /* read the first 6 bytes of the file in question */
#ifdef O_LARGEFILE
    flags |= O_LARGEFILE;
#endif
    fd = open(file, flags);

    if (fd == -1) {
        warn("open");
        return -1;
    }

    if (read(fd, magic, sizeof(magic)) != sizeof(magic)) {
        warn("read");

        if (close(fd) == -1) {
            warn("close");
        }

        return -1;
    }

    if (close(fd) == -1) {
        warn("close");
        return -1;
    }

    /* LLVM IR bitcode files begin with 0x4243C0DE */
    if (magic[0] == '\x42' && magic[1] == '\x43' && magic[2] == '\xC0' && magic[3] == '\xDE') {
        return true;
    }

    return false;
}
