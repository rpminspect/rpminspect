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

#include "config.h"

#include <assert.h>
#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <rpm/rpmlib.h>
#include <curl/curl.h>
#include "builds.h"
#include "rpminspect.h"

/* Local global variables */
static struct rpminspect *workri = NULL;
static int whichbuild = BEFORE_BUILD;
static int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

/* This array holds strings that map to the whichbuild index value. */
static char *build_desc[] = { "before", "after" };

/* Local prototypes */
static void _set_worksubdir(struct rpminspect *, bool, struct koji_build *);
static int _get_rpm_info(const char *);
static int _copytree(const char *, const struct stat *, int, struct FTW *);
static int _download_artifacts(const struct rpminspect *, struct koji_build *);
static void _curl_helper(const bool, const char *, const char *);

/*
 * Set the working subdirectory for this particular run based on whether
 * this is a remote build or a local build.
 */
static void _set_worksubdir(struct rpminspect *ri, bool is_local, struct koji_build *kb) {
    assert(ri != NULL);

    if (ri->worksubdir != NULL) {
        return;
    }

    if (is_local) {
        xasprintf(&ri->worksubdir, "%s/local.XXXXXX", ri->workdir);
    } else {
        assert(kb != NULL);
        xasprintf(&ri->worksubdir, "%s/%s-%s.XXXXXX", ri->workdir, kb->name, kb->version);
    }

    if (mkdtemp(ri->worksubdir) == NULL) {
        fprintf(stderr, "*** Unable to create local work subdirectory: %s\n", strerror(errno));
        fflush(stderr);
        abort();
    }

    return;
}

/*
 * Collect package peer information.
 */
static int _get_rpm_info(const char *pkg) {
    int ret = 0;
    Header h;

    if ((ret = get_rpm_header(pkg, &h)) != 0) {
        return ret;
    }

    add_peer(&workri->peers, whichbuild, pkg, &h);
    headerFree(h);
    return ret;
}

/*
 * Used to recursively copy a build tree over to the working directory.
 */
static int _copytree(const char *fpath, const struct stat *sb,
                     int tflag, struct FTW *ftwbuf) {
    static int toptrim = 0;
    char *workfpath = NULL;
    char *bufpath = NULL;
    int ret = 0;

    /*
     * On our first call, take the length of the main directory that we will work
     * relative to for this rescursive copy.
     */
    if (ftwbuf->level == 0) {
        toptrim = strlen(fpath) + 1;
        return 0;
    }

    workfpath = ((char *) fpath) + toptrim;
    xasprintf(&bufpath, "%s/%s/%s", workri->worksubdir, build_desc[whichbuild], workfpath);

    if (S_ISDIR(sb->st_mode)) {
        if (mkdirp(bufpath, mode)) {
            fprintf(stderr, "*** Error creating directory %s: %s\n", bufpath, strerror(errno));
            ret = -1;
        }
    } else if (S_ISREG(sb->st_mode)) {
        if (copyfile(fpath, bufpath, true, false)) {
            fprintf(stderr, "*** Error copying file %s: %s\n", bufpath, strerror(errno));
            ret = -1;
        }
    } else {
        fprintf(stderr, "*** Unknown directory member encountered: %s\n", fpath);
        ret = -1;
    }

    /* Gather the RPM header for packages */
    if (tflag == FTW_F && strsuffix(bufpath, ".rpm") && _get_rpm_info(bufpath)) {
        ret = -1;
    }

    fflush(stderr);
    free(bufpath);

    return ret;
}

/*
 * Download helper for libcurl
 */
static void _curl_helper(const bool verbose, const char *src, const char *dst) {
    FILE *fp = NULL;
    CURL *c = NULL;
    int r;
    CURLcode cc;

    /* ignore unusued variable warnings if assert is disabled */
    (void) r;
    (void) cc;

    assert(src != NULL);
    assert(dst != NULL);

    /* initialize curl */
    if (!(c = curl_easy_init())) {
        fprintf(stderr, "*** curl_easy_init() failed\n");
        fflush(stderr);
        return;
    }

    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);

    if (verbose) {
        printf("Downloading %s...\n", src);
    }

    /* perform the download */
    fp = fopen(dst, "wb");
    assert(fp != NULL);
    curl_easy_setopt(c, CURLOPT_URL, src);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(c, CURLOPT_FAILONERROR, true);
    cc = curl_easy_perform(c);
    r = fclose(fp);
    assert(r == 0);

    /* remove output file if there was a download error (e.g., 404) */
    if (cc != CURLE_OK) {
        if (unlink(dst)) {
            fprintf(stderr, "*** unable to unlink %s: %s\n", dst, strerror(errno));
            fflush(stderr);
        }
    }

    curl_easy_cleanup(c);

    return;
}

/*
 * Given a remote artifact specification in a Koji build, download it
 * to our working directory.
 */
static int _download_artifacts(const struct rpminspect *ri, struct koji_build *build) {
    koji_buildlist_entry_t *buildentry = NULL;
    koji_rpmlist_entry_t *rpm = NULL;
    char *src = NULL;
    char *dst = NULL;
    char *pkg = NULL;

    assert(build != NULL);
    assert(build->builds != NULL);

    /* Iterate over list of builds, each with a list of packages */
    TAILQ_FOREACH(buildentry, build->builds, builditems) {
        if (TAILQ_EMPTY(buildentry->rpms)) {
            /*
             * not sure how you could have an empty package list,
             * but let's not underestimate what jobs people can
             * submit to koji
             */
            continue;
        }

        /* Download module metadata at the top level */
        if (ri->buildtype == KOJI_BUILD_MODULE) {
            /* Create destination directory */
            xasprintf(&dst, "%s/%s", workri->worksubdir, build_desc[whichbuild]);

            if (mkdirp(dst, mode)) {
                fprintf(stderr, "*** Error creating directory %s: %s\n", dst, strerror(errno));
                fflush(stderr);
                return -1;
            }

            free(dst);

            /* Get the main metadata file */
            xasprintf(&dst, "%s/%s/modulemd.txt", workri->worksubdir, build_desc[whichbuild]);
            xasprintf(&src, "%s/packages/%s/%s/%s/files/module/modulemd.txt", workri->kojimbs, build->package_name, build->version, build->release);
            _curl_helper(workri->verbose, src, dst);
            free(src);
            free(dst);
        }

        /* Iterate over the list of packages for this build */
        TAILQ_FOREACH(rpm, buildentry->rpms, items) {
            /* create the destination directory */
            xasprintf(&dst, "%s/%s/%s", workri->worksubdir, build_desc[whichbuild], rpm->arch);

            if (mkdirp(dst, mode)) {
                fprintf(stderr, "*** Error creating directory %s: %s\n", dst, strerror(errno));
                fflush(stderr);
                return -1;
            }

            free(dst);

            /* for modules, get the per-arch module metadata */
            if (workri->buildtype == KOJI_BUILD_MODULE) {
                xasprintf(&dst, "%s/%s/%s/modulemd.%s.txt", workri->worksubdir, build_desc[whichbuild], rpm->arch, rpm->arch);

                /* only download this file if we have not already gotten it */
                if (access(dst, F_OK|R_OK)) {
                    xasprintf(&src, "%s/packages/%s/%s/%s/files/module/modulemd.%s.txt", workri->kojimbs, build->package_name, build->version, build->release, rpm->arch);
                    _curl_helper(workri->verbose, src, dst);
                    free(src);
                }

                free(dst);
            }

            /* build path strings */
            xasprintf(&pkg, "%s-%s-%s.%s.rpm", rpm->name, rpm->version, rpm->release, rpm->arch);
            xasprintf(&dst, "%s/%s/%s/%s", workri->worksubdir, build_desc[whichbuild], rpm->arch, pkg);

            if (!strcmp(build->volume_name, "DEFAULT")) {
                xasprintf(&src, "%s/packages/%s/%s/%s/%s/%s", (workri->buildtype == KOJI_BUILD_MODULE) ? workri->kojimbs : workri->kojiursine, buildentry->package_name, rpm->version, rpm->release, rpm->arch, pkg);
            } else {
                xasprintf(&src, "%s/%s/packages/%s/%s/%s/%s/%s", (workri->buildtype == KOJI_BUILD_MODULE) ? workri->kojimbs : workri->kojiursine, build->volume_name, buildentry->package_name, rpm->version, rpm->release, rpm->arch, pkg);
            }

            _curl_helper(workri->verbose, src, dst);

            /* gather the RPM header */
            if (_get_rpm_info(dst)) {
                fprintf(stderr, "*** Error reading RPM: %s\n", dst);
                fflush(stderr);
                return -1;
            }

            /* start over */
            free(src);
            free(dst);
            free(pkg);
        }
    }

    return 0;
}

/*
 * Determines if specified builds are local or remote and fetches
 * them to the working directory.  Either build can be local or
 * remote.
 */
int gather_builds(struct rpminspect *ri) {
    struct koji_build *build = NULL;

    assert(ri != NULL);
    assert(ri->after != NULL);

    workri = ri;

    /* process after first so the temp directory gets the NV of that pkg */
    if (ri->after != NULL) {
        if (is_local_build(ri->after)) {
            whichbuild = AFTER_BUILD;
            _set_worksubdir(ri, true, NULL);

            /* copy after tree */
            if (nftw(ri->after, _copytree, 15, FTW_PHYS) == -1) {
                fprintf(stderr, "*** Error gathering build %s: %s\n", ri->after, strerror(errno));
                fflush(stderr);
                return -1;
            }
        } else if ((build = get_koji_build(ri, ri->after)) != NULL) {
            whichbuild = AFTER_BUILD;
            _set_worksubdir(ri, false, build);

            if (_download_artifacts(ri, build)) {
                fprintf(stderr, "*** Error downloading build %s\n", ri->after);
                fflush(stderr);
                return -1;
            }
        } else {
            fprintf(stderr, "*** Unable to find after build: %s\n", ri->after);
            fflush(stderr);
            return -2;
        }
    }

    /* did we get a before build specified? */
    if (ri->before == NULL) {
        return 0;
    }

    /* before build specified, find it */
    if (is_local_build(ri->before)) {
        whichbuild = BEFORE_BUILD;
        _set_worksubdir(ri, true, NULL);

        /* copy before tree */
        if (nftw(ri->before, _copytree, 15, FTW_PHYS) == -1) {
            fprintf(stderr, "*** Error gathering build %s: %s\n", ri->before, strerror(errno));
            fflush(stderr);
            return -1;
        }
    } else if ((build = get_koji_build(ri, ri->before)) != NULL) {
        whichbuild = BEFORE_BUILD;
        _set_worksubdir(ri, false, build);

        if (_download_artifacts(ri, build)) {
            fprintf(stderr, "*** Error downloading build %s\n", ri->before);
            fflush(stderr);
            return -1;
        }
    } else {
        fprintf(stderr, "*** Unable to find before build: %s\n", ri->before);
        fflush(stderr);
        return -1;
    }

    return 0;
}
