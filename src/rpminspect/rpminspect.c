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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <glob.h>

#include "rpminspect.h"
#include "builds.h"

/* Global librpminspect state */
struct rpminspect ri;

static void usage(const char *progname) {
    assert(progname != NULL);

    printf("Compare package builds for policy compliance and consistency.\n\n");
    printf("Usage: %s [OPTIONS] [before build] [after build]\n", progname);
    printf("Options:\n");
    printf("  -c FILE, --config=FILE   Configuration file to use\n");
    printf("                             (default: %s)\n", CFGFILE);
    printf("  -T LIST, --tests=LIST    Comma-separated list of tests to run or skip\n");
    printf("                             (default: ALL)\n");
    printf("  -r STR, --release=STR    Product release string\n");
    printf("  -o FILE, --output=FILE   Write results to FILE\n");
    printf("                             (default: stdout)\n");
    printf("  -F TYPE, --format=TYPE   Format output results as TYPE\n");
    printf("                             (default: text)\n");
    printf("  -l, --list               List available tests and formats\n");
    printf("  -w PATH, --workdir=PATH  Temporary directory to use\n");
    printf("                             (default: %s)\n", DEFAULT_WORKDIR);
    printf("  -f, --fetch-only         Fetch builds only, do not perform inspections\n");
    printf("                             (implies -k)\n");
    printf("  -k, --keep               Do not remove the comparison working files\n");
    printf("  -v, --verbose            Verbose inspection output\n");
    printf("                           when finished, display full path\n");
    printf("  -?, --help               Display usage information\n");
    printf("  -V, --version            Display program version\n");
    printf("\nSee the rpminspect(1) man page for more information.\n");

    return;
}

/*
 * Get the product release string by grabbing a possible dist tag from
 * the Release value.  Dist tags begin with '.' and go to the end of the
 * Release value.  Trim any trailing '/' characters in case the user is
 * specifying a build from a local path.
 */
static char *get_product_release(const char *before, const char *after) {
    char *pos = NULL;
    char *before_product = NULL;
    char *after_product = NULL;

    assert(after != NULL);

    pos = rindex(after, '.') + 1;
    after_product = strdup(pos);

    if (!after_product) {
        fprintf(stderr, "*** Product release for after build (%s) is empty\n", after);
        return NULL;
    }

    /*
     * Trim any trailing slashes in case the user is specifying builds from
     * local paths
     */
    after_product[strcspn(after_product, "/")] = 0;

    if (before) {
        pos = rindex(before, '.') + 1;
        before_product = strdup(pos);

        if (!before_product) {
            fprintf(stderr, "*** Product release for before build (%s) is empty\n", before);
            free(after_product);
            return NULL;
        }

        /*
         * Trim any trailing slashes in case the user is specifying builds from
         * local paths
         */
        before_product[strcspn(before_product, "/")] = 0;

        if (strcmp(before_product, after_product)) {
            fprintf(stderr, "*** Builds have different product releases (%s != %s)\n", before_product, after_product);
            free(before_product);
            free(after_product);
            return NULL;
        }

        free(before_product);
    }

    return after_product;
}

int main(int argc, char **argv) {
    char *progname = basename(argv[0]);
    int c, i, j;
    int idx = 0;
    int ret = EXIT_SUCCESS;
    glob_t expand;
    char *short_options = "c:T:r:o:F:lw:fkv\?V";
    struct option long_options[] = {
        { "config", required_argument, 0, 'c' },
        { "tests", required_argument, 0, 'T' },
        { "release", required_argument, 0, 'r' },
        { "list", no_argument, 0, 'l' },
        { "output", required_argument, 0, 'o' },
        { "format", required_argument, 0, 'F' },
        { "workdir", required_argument, 0, 'w' },
        { "fetch-only", no_argument, 0, 'f' },
        { "keep", no_argument, 0, 'k' },
        { "verbose", no_argument, 0, 'v' },
        { "help", no_argument, 0, '?' },
        { "version", no_argument, 0, 'V' },
        { 0, 0, 0, 0 }
    };
    char *cfgfile = NULL;
    char *workdir = NULL;
    char *output = NULL;
    char *release = NULL;
    int formatidx = -1;
    bool fetch_only = false;
    bool keep = false;
    bool verbose = false;
    int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    bool found = false;
    char *test = NULL;
    uint64_t tests = 0;
    uint64_t selected = 0;
    bool negated_tests = false;
    size_t width = tty_width();

    /* parse command line options */
    while (1) {
        c = getopt_long(argc, argv, short_options, long_options, &idx);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'c':
                /* Capture user specified config file */
                cfgfile = strdup(optarg);
                break;
            case 'T':
                if (!strcasecmp(optarg, "all")) {
                    selected = ~tests;
                    break;
                }

                while ((test = strsep(&optarg, ",")) != NULL) {
                    found = false;

                    for (i = 0; inspections[i].flag != 0; i++) {
                        if (*test == '!' && !strcasecmp(test+1, inspections[i].name)) {
                            if (!negated_tests) {
                                negated_tests = true;
                                selected = ~tests;
                            }

                            /* user wants to skip this specific test */
                            selected &= ~(inspections[i].flag);
                            found = true;
                            break;
                        } else if (!strcasecmp(test, inspections[i].name)) {
                            /* user wants to perform this specific test */
                            selected |= inspections[i].flag;
                            found = true;
                            break;
                        } else if (!strcasecmp(test, "ALL")) {
                            selected = ~tests;
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        fprintf(stderr, "*** Unknown test specified: `%s`\n", test);
                        fprintf(stderr, "*** See `%s --help` for more information.\n", progname);
                        fflush(stderr);
                        return EXIT_FAILURE;
                    }
                }

                break;
            case 'r':
                release = strdup(optarg);
                break;
            case 'o':
                output = strdup(optarg);
                break;
            case 'F':
                /* validate the specified output format */
                for (i = 0; formats[i].type != -1; i++) {
                    if (!strcasecmp(formats[i].name, optarg)) {
                        formatidx = formats[i].type;
                        break;
                    }
                }

                if (formatidx < 0) {
                    fprintf(stderr, "*** Invalid output format: `%s`.\n", optarg);
                    fflush(stderr);
                    return EXIT_FAILURE;
                }

                break;
            case 'l':
                /* list the formats available */
                printf("Available output formats:\n");

                for (i = 0; formats[i].type != -1; i++) {
                    if (i > 0) {
                        printf("\n");
                    }

                    printf("    %s\n", formats[i].name);

                    if (formats[i].desc != NULL) {
                        printwrap(formats[i].desc, width, 8, stdout);
                        printf("\n");
                    }
                }

                /* list the inspections available */
                printf("\nAvailable inspections:\n");

                for (i = 0; inspections[i].flag != 0; i++) {
                    if (i > 0) {
                        printf("\n");
                    }

                    printf("    %s\n", inspections[i].name);

                    if (inspections[i].desc != NULL) {
                        printwrap(inspections[i].desc, width, 8, stdout);
                        printf("\n");
                    }
                }

                exit(EXIT_SUCCESS);
                break;
            case 'w':
                if (index(optarg, '~')) {
                    /* allow for ~ expansion */
                    j = glob(optarg, GLOB_TILDE_CHECK, NULL, &expand);

                    if (j == 0 && expand.gl_pathc == 1) {
                        workdir = strdup(expand.gl_pathv[0]);
                    } else {
                        fprintf(stderr, "*** Unable to expand workdir: `%s`: %s", optarg, strerror(errno));
                        fflush(stderr);
                        return EXIT_FAILURE;
                    }

                    globfree(&expand);
                } else {
                    workdir = strdup(optarg);
                }

                break;
            case 'f':
                fetch_only = true;        /* fall-thru: -f implies -k */
                __attribute__ ((fallthrough));
            case 'k':
                keep = true;
                break;
            case 'v':
                verbose = true;
                break;
            case '?':
                usage(progname);
                exit(0);
            case 'V':
                printf("%s version %s\n", progname, PACKAGE_VERSION);
                exit(0);
            default:
                fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
                fflush(stderr);
                exit(EXIT_FAILURE);
        }
    }

    /* Find an appropriate configuration file */
    if ((cfgfile == NULL) || (access(CFGFILE, F_OK|R_OK) == -1)) {
        cfgfile = strdup(CFGFILE);
    }

    /* Initialize librpminspect */
    if (init_rpminspect(&ri, cfgfile) != 0) {
        fprintf(stderr, "Failed to read configuration file\n");
        exit(EXIT_FAILURE);
    }

    free(cfgfile);

    /* various options from the command line */
    ri.verbose = verbose;
    ri.product_release = release;

    /* Copy in user-selected tests if they specified something */
    if (selected != 0) {
        ri.tests = selected;
    }

    /* the user did not specify a working directory */
    if (workdir != NULL) {
        free(ri.workdir);
        ri.workdir = workdir;
    }

    /*
     * we should exactly one more argument (single build) or two arguments
     * (a before and after build)
     */
    if (optind == (argc - 1)) {
        /* only a single build specified */
        ri.after = strdup(argv[optind]);
    } else if ((optind + 1) == (argc - 1)) {
        /* we got a before and after build */
        ri.before = strdup(argv[optind]);
        ri.after = strdup(argv[optind + 1]);
    } else {
        /* user gave us too many arguments */
        fprintf(stderr, "*** Invalid before and after build specification.\n");
        fprintf(stderr, "*** See `%s --help` for more information.\n", progname);
        fflush(stderr);
        free_rpminspect(&ri);
        return EXIT_FAILURE;
    }

    /*
     * Determine product release unless the user specified one.
     */
    if (ri.product_release == NULL) {
        ri.product_release = get_product_release(ri.before, ri.after);

        if (ri.product_release == NULL) {
            fflush(stderr);
            free_rpminspect(&ri);
            return EXIT_FAILURE;
        }
    }

    /* initialize librpm, we'll be using it */
    if (init_librpm() != RPMRC_OK) {
        fprintf(stderr, "*** unable to read RPM configuration\n");
        fflush(stderr);
        return EXIT_FAILURE;
    }

    /* create the working directory */
    if (mkdirp(ri.workdir, mode)) {
        fprintf(stderr, "*** Unable to create directory %s: %s\n", ri.workdir, strerror(errno));
        fflush(stderr);
        free_rpminspect(&ri);
        return EXIT_FAILURE;
    }

    /* validate and gather the builds specified */
    if ((ret = gather_builds(&ri, fetch_only))) {
        fprintf(stderr, "*** Failed to gather specified builds.\n");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    /* perform the selected inspections */
    if (!fetch_only) {
        for (i = 0; inspections[i].flag != 0; i++) {
            /* test not selected by user */
            if (!(ri.tests & inspections[i].flag)) {
                continue;
            }

           /* inspection requires before/after builds and we have one */
           if (ri.before == NULL && !inspections[i].single_build) {
               continue;
           }

            if (!inspections[i].driver(&ri)) {
                ret = EXIT_FAILURE;
            }
        }

        /* output the results */
        if (formatidx == -1) {
            formatidx = 0;                 /* default to 'text' output */
        }

        if (ri.results != NULL) {
            formats[formatidx].driver(ri.results, output);
        }
    }

    /* Clean up */
    if (keep) {
        printf("\nKeeping working directory: %s\n", ri.worksubdir);
    } else {
        if (rmtree(ri.workdir, true, true)) {
           fprintf(stderr, "*** Error removing directory %s: %s\n", ri.workdir, strerror(errno));
           fflush(stderr);
        }
    }

    free_rpminspect(&ri);

    return EXIT_SUCCESS;
}





/*
 * CAREFUL!  This string comes to us as the --release=XXX option,
 * which nominally comes from the 'variant' row in the Errata
 * rpmdiff_runs table.  (In short: we get that from ET).
 * But erratadiff.py:get_release_for_variant() can "massage"
 * that!  For instance, "6Server" gets converted to "6any".
 */
//struct {
//    const char *release;
//    enum Platform platform;
//} release_platform_map[] = {
//    {"2.1AS",   PLATFORM_RHEL_2_1      },
//    {"3AS",     PLATFORM_RHEL_3        },
//    {"4AS",     PLATFORM_RHEL_4        },
//    {"5Server", PLATFORM_RHEL_5_Server },
//    {"5Client", PLATFORM_RHEL_5_Client },
//    {"6any",    PLATFORM_RHEL_6        },
//    {"7any",    PLATFORM_RHEL_7        },
//    {"8any",    PLATFORM_RHEL_8        },
//};
