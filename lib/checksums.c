/* {{{ Apache License version 2.0
 */
/*
 * Copyright 2004-2019 David Shea <david@reallylongword.org>
 *                     Chris Lumens <chris@bangmoney.org>
 *                     David Cantrell <david.l.cantrell@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* }}} */

/**
 * @file checksums.c
 * @author David Cantrell &lt;david.l.cantrell@gmail.com&gt;
 * @author Chris Lumens &lt;chris@bangmoney.org&gt;
 * @author David Shea &lt;david@reallylongword.org&gt;
 * @date 2004-2019
 * @brief Calculate an MD5, SHA-1, SHA-224, SHA-256, SHA-384, or SHA-512 checksum for a file.
 * @copyright Apache-2.0
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include "rpminspect.h"

/**
 * @brief Take in a file, return a checksum.
 *
 * Given a file, its **mode_t**, and a valid checksum type, compute
 * the checksum and return the human-readable digest string for that
 * checksum.  This function allocates memory for the string and the
 * caller must free it when done.
 *
 * @param filename Filename the function should use.
 * @param st_mode The **mode_t** for the specified file, gathered from **stat(2)**.
 * @param type Which checksum type to calculate.
 * @note Caller must free returned string when done.
 * @return String containing the human-readable checksum digest, or NULL on failure.
 */
char *compute_checksum(const char *filename, mode_t *st_mode, enum checksum type)
{
    struct stat sb;
    mode_t *mode = NULL;
    unsigned char buf[BUFSIZ];
    int input, len, i;
    char *ret = NULL;
    unsigned char digest[BUFSIZ];
    MD5_CTX md5c;
    SHA_CTX sha1c;
    SHA256_CTX sha256c;
    SHA512_CTX sha512c;

    /* if the user did not provide a mode_t, get it */
    if (st_mode == NULL) {
        if (lstat(filename, &sb) != 0) {
            return NULL;
        }

        mode = &sb.st_mode;
    } else {
        mode = st_mode;
    }

    /* don't calculate the checksum of a device node */
    if (S_ISCHR(*mode) || S_ISBLK(*mode) ||
        S_ISFIFO(*mode) || S_ISSOCK(*mode)) {
        fprintf(stderr, _("*** Cannot calculate checksum on devices or fifos: %s\n"), filename);
        fflush(stderr);
        return NULL;
    }

    /*
     * This holds all possible digest type computations.  if any future digest
     * is larger than BUFSIZ, hopefully we update this and get faster systems.
     */
    memset(&buf, '\0', BUFSIZ);
    memset(&digest, '\0', BUFSIZ);

    /* Initialize the correct context based on the checksum type. */
    if (type == MD5SUM) {
        MD5_Init(&md5c);
    } else if (type == SHA1SUM) {
        SHA1_Init(&sha1c);
    } else if (type == SHA224SUM || type == SHA256SUM) {
        SHA256_Init(&sha256c);
    } else if (type == SHA384SUM || type == SHA512SUM) {
        SHA512_Init(&sha512c);
    }

    /* read in the file to generate the requested checksum */
    if ((input = open(filename, O_RDONLY)) == -1) {
        fprintf(stderr, _("*** Unable to open %s: %s\n"), filename, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    if ((len = read(input, buf, sizeof(buf))) == -1) {
        fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    while (len > 0) {
        /* update the correct context based on the checksum type */
        if (type == MD5SUM) {
            MD5_Update(&md5c, buf, len);
        } else if (type == SHA1SUM) {
            SHA1_Update(&sha1c, buf, len);
        } else if (type == SHA224SUM || type == SHA256SUM) {
            SHA256_Update(&sha256c, buf, len);
        } else if (type == SHA384SUM || type == SHA512SUM) {
            SHA512_Update(&sha512c, buf, len);
        }

        if ((len = read(input, buf, sizeof(buf))) == -1) {
            fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
            fflush(stderr);
            return NULL;
        }
    }

    if (close(input) == -1) {
        fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    /* finalize the correct context type and determine the loop length */
    if (type == MD5SUM) {
        MD5_Final(digest, &md5c);
    } else if (type == SHA1SUM) {
        SHA1_Final(digest, &sha1c);
    } else if (type == SHA224SUM || type == SHA256SUM) {
        SHA256_Final(digest, &sha256c);
    } else if (type == SHA384SUM || type == SHA512SUM) {
        SHA512_Final(digest, &sha512c);
    }

    if (type == MD5SUM) {
        len = MD5_DIGEST_LENGTH;
    } else if (type == SHA1SUM) {
        len = SHA_DIGEST_LENGTH;
    } else if (type == SHA224SUM) {
        len = SHA224_DIGEST_LENGTH;
    } else if (type == SHA256SUM) {
        len = SHA256_DIGEST_LENGTH;
    } else if (type == SHA384SUM) {
        len = SHA384_DIGEST_LENGTH;
    } else if (type == SHA512SUM) {
        len = SHA512_DIGEST_LENGTH;
    }

    /* this is our human readable digest, caller must free */
    if ((ret = calloc(len + 1, sizeof(char *))) == NULL) {
        fprintf(stderr, "%s (%d): %s\n", __func__, __LINE__, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    for (i = 0; i < len; ++i) {
        sprintf(&ret[i*2], "%02x", (unsigned int) digest[i]);
    }

    return ret;
}

/**
 * @brief Return checksum string of the given **rpmfile_entry_t**.
 *
 * The **rpmfile_entry_t** will contain a cached checksum string or
 * not.  If it does, this function returns the cached string.  If the
 * string is NULL, this function calculates the checksum, caches it,
 * and returns the string.
 *
 * @param file The **rpmfile_entry_t** specifying the file to use.
 * @note Do not free the result returned, that is handled by
 *       **free_files()**.
 * @return String containing the human-readable checksum digest, or
 *         NULL on failure.
 */
char *checksum(rpmfile_entry_t *file)
{
    assert(file != NULL);

    if (file->checksum) {
        return file->checksum;
    }

    file->checksum = compute_checksum(file->fullpath, &file->st.st_mode, SHA256SUM);
    return file->checksum;
}
