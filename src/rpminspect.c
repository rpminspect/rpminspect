/*
 * Copyright (C) 2019-2020  Red Hat, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <glob.h>
#include <regex.h>

#include "rpminspect.h"

/* Global librpminspect state */
struct rpminspect ri;

void sigabrt_handler(__attribute__ ((unused)) int i)
{
    rpmFreeRpmrc();
    return;
}

static void usage(const char *progname)
{
    assert(progname != NULL);

    printf(_("Compare package builds for policy compliance and consistency.\n\n"));
    printf(_("Usage: %s [OPTIONS] [before build] [after build]\n"), progname);
    printf(_("Options:\n"));
    printf(_("  -c FILE, --config=FILE   Configuration file to use\n"));
    printf(_("                             (default: %s)\n"), CFGFILE);
    printf(_("  -p NAME, --profile=NAME  Configuration profile to use\n"));
    printf(_("  -T LIST, --tests=LIST    List of tests to run\n"));
    printf(_("                             (default: ALL)\n"));
    printf(_("  -E LIST, --exclude=LIST  List of tests to exclude\n"));
    printf(_("                             (default: none)\n"));
    printf(_("  -a LIST, --arches=LIST   List of architectures to check\n"));
    printf(_("  -r STR, --release=STR    Product release string\n"));
    printf(_("  -o FILE, --output=FILE   Write results to FILE\n"));
    printf(_("                             (default: stdout)\n"));
    printf(_("  -F TYPE, --format=TYPE   Format output results as TYPE\n"));
    printf(_("                             (default: text)\n"));
    printf(_("  -t TAG, --threshold=TAG  Result threshold triggering exit\n"));
    printf(_("                           failure (default: VERIFY)\n"));
    printf(_("  -l, --list               List available tests and formats\n"));
    printf(_("  -w PATH, --workdir=PATH  Temporary directory to use\n"));
    printf(_("                             (default: %s)\n"), DEFAULT_WORKDIR);
    printf(_("  -f, --fetch-only         Fetch builds only, do not perform inspections\n"));
    printf(_("                             (implies -k)\n"));
    printf(_("  -k, --keep               Do not remove the comparison working files\n"));
    printf(_("  -d, --debug              Debugging mode output\n"));
    printf(_("  -v, --verbose            Verbose inspection output\n"));
    printf(_("                           when finished, display full path\n"));
    printf(_("  -?, --help               Display usage information\n"));
    printf(_("  -V, --version            Display program version\n"));
    printf(_("\nSee the rpminspect(1) man page for more information.\n"));

    return;
}

/*
 * Get the product release string by grabbing a possible dist tag from
 * the Release value.  Dist tags begin with '.' and go to the end of the
 * Release value.  Trim any trailing '/' characters in case the user is
 * specifying a build from a local path.
 */
static char *get_product_release(const favor_release_t favor_release, const char *before, const char *after)
{
    int c;
    char *pos = NULL;
    char *before_product = NULL;
    char *after_product = NULL;
    char *needle = NULL;
    string_entry_t *entry = NULL;
    ENTRY e;
    ENTRY *eptr;
    regex_t product_regex;
    int result;
    char reg_error[BUFSIZ];
    regmatch_t before_matches[1];
    regmatch_t after_matches[1];
    bool matched = false;

    assert(after != NULL);

    pos = rindex(after, '.');
    if (!pos) {
        fprintf(stderr, _("*** Product release for after build (%s) is empty\n"), after);
        return NULL;
    }

    /*
     * Get the character after the last occurrence of a period. This should
     * tell us what release flag the product is.
     */
    pos += 1;
    after_product = strdup(pos);
    if (!after_product) {
        fprintf(stderr, _("*** Product release for after build (%s) is empty\n"), after);
        free(after_product);
        return NULL;
    }

    /*
     * Trim any trailing slashes in case the user is specifying builds from
     * local paths
     */
    after_product[strcspn(after_product, "/")] = 0;

    if (before) {
        pos = rindex(before, '.');

        if (!pos) {
            fprintf(stderr, _("*** Product release for before build (%s) is empty\n"), before);
            free(after_product);
            return NULL;
        }

        pos += 1;
        before_product = strdup(pos);

        if (!before_product) {
            fprintf(stderr, _("*** Product release for before build (%s) is empty\n"), before);
            free(after_product);
            return NULL;
        }

        /*
         * Trim any trailing slashes in case the user is specifying builds from
         * local paths
         */
        before_product[strcspn(before_product, "/")] = 0;

        /*
         * If builds are different and we have no products hash table, fail
         */
        c = strcmp(before_product, after_product);

        if (c && (ri.product_keys != NULL)) {
            /* after_product and before_product are refreshed in the loop */
            free(after_product);
            free(before_product);

            /* Try to see if a product mapping matches our strings */
            TAILQ_FOREACH(entry, ri.product_keys, items) {
                /* refresh after_product */
                xasprintf(&needle, ".%s", entry->data);
                after_product = strstr(after, needle);
                before_product = strstr(before, needle);
                free(needle);

                if (after_product == NULL || before_product == NULL) {
                    continue;
                }

                /* find this product in the hash table */
                e.key = entry->data;
                hsearch_r(e, FIND, &eptr, ri.products);

                /* if the config file entry is empty, just ignore it */
                if (eptr == NULL) {
                    continue;
                }

                /* build a regex for this product release string */
                result = regcomp(&product_regex, eptr->data, 0);

                if (result != 0) {
                    regerror(result, &product_regex, reg_error, sizeof(reg_error));
                    fprintf(stderr, _("*** unable to compile product release regular expression: %s\n"), reg_error);
                    free(before_product);
                    free(after_product);
                    return NULL;
                }

                /* now try to match the before and after builds */
                if (regexec(&product_regex, before_product, 1, before_matches, 0) != 0) {
                    regfree(&product_regex);
                    continue;
                }

                if (regexec(&product_regex, after_product, 1, after_matches, 0) != 0) {
                    regfree(&product_regex);
                    continue;
                }

                if (before_matches[0].rm_so > -1 && after_matches[0].rm_so > -1) {
                    matched = true;
                    after_product = strdup(entry->data);
                }

                regfree(&product_regex);

                if (matched) {
                    break;
                }
            }
        } else if (!c) {
            matched = true;
            free(before_product);
        }
    } else {
        matched = true;
    }

    /* Handle release favoring */
    if (!matched && favor_release != FAVOR_NONE) {
        c = strverscmp(before_product, after_product);
        matched = true;

        if (favor_release == FAVOR_OLDEST) {
            if (c <= 0) {
                free(after_product);
                after_product = before_product;
            } else {
                free(before_product);
            }
        } else if (favor_release == FAVOR_NEWEST) {
            if (c >= 0) {
                free(after_product);
                after_product = before_product;
            } else {
                free(before_product);
            }
        }
    }

    if (!matched) {
        fprintf(stderr, _("*** Unable to determine product release for %s and %s\n"), before, after);
        free(before_product);
        free(after_product);
        after_product = NULL;
    }

    return after_product;
}

/*
 * Used to ensure the user only specifies the -T or -E option.
 */
static void check_inspection_options(const bool inspection_opt, const char *progname)
{
    assert(progname != NULL);

    if (inspection_opt) {
        fprintf(stderr, _("*** The -T and -E options are mutually exclusive\n"));
        fprintf(stderr, _("*** See `%s --help` for more information.\n"), progname);
        fflush(stderr);
        exit(RI_PROGRAM_ERROR);
    }

    return;
}

/*
 * Used in the -T and -E option processing to report any unknown
 * test names provided.  Exit if true.
 */
static void check_found(const bool found, const char *inspection, const char *progname)
{
    assert(inspection != NULL);
    assert(progname != NULL);

    if (!found) {
        fprintf(stderr, _("*** Unknown inspection: `%s`\n"), inspection);
        fprintf(stderr, _("*** See `%s --help` for more information.\n"), progname);
        fflush(stderr);
        exit(RI_PROGRAM_ERROR);
    }

    return;
}

int main(int argc, char **argv) {
    char *progname = basename(argv[0]);
    int c, i, j;
    int idx = 0;
    int ret = RI_INSPECTION_SUCCESS;
    glob_t expand;
    char *short_options = "c:p:T:E:a:r:o:F:lw:t:fkdv\?V";
    struct option long_options[] = {
        { "config", required_argument, 0, 'c' },
        { "profile", required_argument, 0, 'p' },
        { "tests", required_argument, 0, 'T' },
        { "exclude", required_argument, 0, 'E' },
        { "arches", required_argument, 0, 'a' },
        { "release", required_argument, 0, 'r' },
        { "list", no_argument, 0, 'l' },
        { "output", required_argument, 0, 'o' },
        { "format", required_argument, 0, 'F' },
        { "workdir", required_argument, 0, 'w' },
        { "threshold", required_argument, 0, 't' },
        { "fetch-only", no_argument, 0, 'f' },
        { "keep", no_argument, 0, 'k' },
        { "debug", no_argument, 0, 'd' },
        { "verbose", no_argument, 0, 'v' },
        { "help", no_argument, 0, '?' },
        { "version", no_argument, 0, 'V' },
        { 0, 0, 0, 0 }
    };
    char *cfgfile = NULL;
    char *profile = NULL;
    char *archopt = NULL;
    char *walk = NULL;
    char *token = NULL;
    char *workdir = NULL;
    char cwd[PATH_MAX + 1];
    char *r = NULL;
    char *output = NULL;
    char *release = NULL;
    char *threshold = NULL;
    int formatidx = -1;
    bool fetch_only = false;
    bool keep = false;
    bool list = false;
    bool debug = false;
    bool verbose = false;
    int mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    bool found = false;
    char *inspection = NULL;
    const char *desc = NULL;
    char *insoptarg = NULL;
    char *tmp = NULL;
    bool inspection_opt = false;
    bool exclude = false;
    size_t width = tty_width();
    string_list_t *valid_arches = NULL;
    string_entry_t *arch = NULL;
    string_entry_t *entry = NULL;
    rpmpeer_entry_t *peer = NULL;
    const char *after_rel = NULL;
    const char *before_rel = NULL;
    struct result_params params;
    size_t cmdlen = 0;
    char *tail = NULL;

    /* Be friendly to "rpminspect ... 2>&1 | tee" use case */
    setlinebuf(stdout);

    /* SIGABRT handler since we use abort() in some failure cases */
    signal(SIGABRT, sigabrt_handler);

    /* Set up the i18n environment */
    setlocale(LC_ALL, "");
    bindtextdomain("rpminspect", "/usr/share/locale/");
    textdomain("rpminspect");

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
            case 'p':
                /* Configuration profile to use */
                profile = strdup(optarg);
                break;
            case 'T':
                /* Inspections to enable */
                check_inspection_options(inspection_opt, progname);
                insoptarg = strdup(optarg);
                exclude = false;
                inspection_opt = true;
                break;
            case 'E':
                /* Inspections to disable */
                check_inspection_options(inspection_opt, progname);
                insoptarg = strdup(optarg);
                exclude = true;
                inspection_opt = true;
                break;
            case 'a':
                archopt = strdup(optarg);
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
                    fprintf(stderr, _("*** Invalid output format: `%s`.\n"), optarg);
                    fflush(stderr);
                    return RI_PROGRAM_ERROR;
                }

                break;
            case 'l':
                list = true;
                break;
            case 'w':
                if (index(optarg, '~')) {
                    /* allow for ~ expansion */
                    j = glob(optarg, GLOB_TILDE_CHECK, NULL, &expand);

                    if (j == 0 && expand.gl_pathc == 1) {
                        workdir = strdup(expand.gl_pathv[0]);
                    } else {
                        fprintf(stderr, _("*** Unable to expand workdir: `%s`: %s"), optarg, strerror(errno));
                        fflush(stderr);
                        return RI_PROGRAM_ERROR;
                    }

                    globfree(&expand);
                } else {
                    workdir = strdup(optarg);
                }

                break;
            case 't':
                threshold = strdup(optarg);
                break;
            case 'f':
                fetch_only = true;        /* -f implies -k */
                /* fall through */
            case 'k':
                keep = true;
                break;
            case 'd':
                debug = true;
                break;
            case 'v':
                verbose = true;
                break;
            case '?':
                usage(progname);
                exit(0);
            case 'V':
                printf(_("%s version %s\n"), progname, PACKAGE_VERSION);
                exit(0);
            default:
                fprintf(stderr, _("?? getopt returned character code 0%o ??\n"), c);
                fflush(stderr);
                exit(RI_PROGRAM_ERROR);
        }
    }

    /* list inspections and formats and exit if asked to */
    if (list) {
        /* list the formats available */
        printf(_("Available output formats:\n"));

        for (i = 0; formats[i].type != -1; i++) {
            if (i > 0 && verbose) {
                printf("\n");
            }

            printf("    %s\n", formats[i].name);
            desc = format_desc(formats[i].type);

            if (desc != NULL && verbose) {
                printwrap(desc, width, 8, stdout);
                printf("\n");
            }
        }

        /* list the inspections available */
        printf(_("\nAvailable inspections:\n"));

        for (i = 0; inspections[i].flag != 0; i++) {
            if (i > 0 && verbose) {
                printf("\n");
            }

            printf("    %s\n", inspections[i].name);
            desc = inspection_desc(inspections[i].flag);

            if (desc != NULL && verbose) {
                printwrap(desc, width, 8, stdout);
                printf("\n");
            }
        }

        exit(RI_INSPECTION_SUCCESS);
    }

    /*
     * Find an appropriate configuration file. This involves:
     *
     *  - Using the user-passed value and sanity-checking it,
     *  - Using the global default if it exists, or
     *  - Telling the user they need to install a required dependency.
     */
    if (cfgfile != NULL && access(cfgfile, F_OK|R_OK) == -1) {
        fprintf(stderr, _("Specified config file (%s) is unreadable.\n"), cfgfile);
        exit(RI_PROGRAM_ERROR);
    } else if (cfgfile == NULL && access(CFGFILE, F_OK|R_OK) == 0) {
        cfgfile = strdup(CFGFILE);
    } else if (cfgfile == NULL) {
        fprintf(stderr, _("Unable to read the default config file (%s).\n"), CFGFILE);
        fprintf(stderr, _("Have you installed an rpminspect-data package for your distro?\n"));
        exit(RI_PROGRAM_ERROR);
    }

    /* Initialize librpminspect */
    if (init_rpminspect(&ri, cfgfile, profile) != 0) {
        fprintf(stderr, _("Failed to read configuration file\n"));
        exit(RI_PROGRAM_ERROR);
    }

    free(cfgfile);
    free(profile);

    /* various options from the command line */
    set_debug_mode(debug);
    ri.verbose = verbose;
    ri.product_release = release;
    ri.threshold = getseverity(threshold);

    /*
     * any inspection selections on the command line can override
     * selections made via the config files
     */
    if (inspection_opt) {
        if (exclude) {
            ri.tests = ~0;
        } else {
            ri.tests = 0;
        }

        tmp = insoptarg;

        while ((inspection = strsep(&insoptarg, ",")) != NULL) {
            found = process_inspection_flag(inspection, exclude, &ri.tests);
            check_found(found, inspection, progname);
        }

        free(tmp);
    }

    /* the user specified a working directory */
    if (workdir != NULL) {
        free(ri.workdir);
        ri.workdir = workdir;
    }

    /* no workdir specified, but fetch only requested, default to cwd */
    if (workdir == NULL && fetch_only) {
        memset(cwd, '\0', sizeof(cwd));
        r = getcwd(cwd, PATH_MAX);
        assert(r != NULL);

        free(ri.workdir);
        ri.workdir = strdup(r);
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
        fprintf(stderr, _("*** Invalid before and after build specification.\n"));
        fprintf(stderr, _("*** See `%s --help` for more information.\n"), progname);
        fflush(stderr);
        free_rpminspect(&ri);
        return RI_PROGRAM_ERROR;
    }

    /*
     * Fetch-only mode can only work with a single build
     */
    if (fetch_only && ri.before) {
        fprintf(stderr, _("*** Fetch only mode takes a single build specification.\n"));
        fprintf(stderr, _("*** See `%s --help` for more information.\n"), progname);
        fflush(stderr);
        free_rpminspect(&ri);
        return RI_PROGRAM_ERROR;
    }

    /* initialize librpm, we'll be using it */
    if (init_librpm() != RPMRC_OK) {
        fprintf(stderr, _("*** unable to read RPM configuration\n"));
        fflush(stderr);
        return RI_PROGRAM_ERROR;
    }

    /* if an architecture list is specified, validate it */
    if (archopt) {
        /* initialize the list of allowed architectures */
        ri.arches = calloc(1, sizeof(*ri.arches));
        assert(ri.arches != NULL);
        TAILQ_INIT(ri.arches);

        /* get a list of valid architectures */
        valid_arches = get_all_arches(&ri);

        /* collect the specified architectures */
        walk = archopt;

        while ((token = strsep(&walk, ",")) != NULL) {
            /* if the architecture is invalid, report it and exit */
            found = false;

            TAILQ_FOREACH(arch, valid_arches, items) {
                if (!strcmp(token, arch->data)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                fprintf(stderr, _("*** Unsupported architecture specified: `%s`\n"), token);
                fprintf(stderr, _("*** See `%s --help` for more information.\n"), progname);
                fflush(stderr);
                rpmFreeRpmrc();
                return RI_PROGRAM_ERROR;
            }

            /* architecture is valid, save it */
            entry = calloc(1, sizeof(*entry));
            assert(entry != NULL);

            entry->data = strdup(token);
            assert(entry->data != NULL);

            TAILQ_INSERT_TAIL(ri.arches, entry, items);
        }

        /* clean up */
        free(archopt);
        list_free(valid_arches, free);
    }

    /* create the working directory */
    if (mkdirp(ri.workdir, mode)) {
        fprintf(stderr, _("*** Unable to create directory %s: %s\n"), ri.workdir, strerror(errno));
        fflush(stderr);
        free_rpminspect(&ri);
        rpmFreeRpmrc();
        return RI_PROGRAM_ERROR;
    }

    /* validate and gather the builds specified */
    if (gather_builds(&ri, fetch_only)) {
        fprintf(stderr, _("*** Failed to gather specified builds.\n"));
        fflush(stderr);
        rpmFreeRpmrc();
        exit(RI_PROGRAM_ERROR);
    }

    /* add command information to the results output */
    init_result_params(&params);
    params.severity = RESULT_INFO;
    params.header = HEADER_RPMINSPECT;
    params.msg = _("command line");

    for (i = 0; i < argc; i++) {
        cmdlen += strlen(argv[i]) + 1;
    }

    params.details = malloc(cmdlen + 1);
    assert(params.details != NULL);
    tail = params.details;

    for (i = 0; i < argc; i++) {
        if (i != 0) {
            tail = stpcpy(tail, " ");
        }

        tail = stpcpy(tail, argv[i]);
    }

    add_result_entry(&ri.results, &params);
    ri.worst_result = params.severity;
    free(params.details);

    /* perform the selected inspections */
    if (!fetch_only) {
        /* Determine product release unless the user specified one. */
        if (ri.product_release == NULL) {
            if (ri.peers == NULL || TAILQ_EMPTY(ri.peers)) {
                fprintf(stderr, _("*** No peers, ensure packages exist for specified architecture(s).\n"));
                fflush(stderr);
                free_rpminspect(&ri);
                return RI_PROGRAM_ERROR;
            }

            peer = TAILQ_FIRST(ri.peers);
            after_rel = headerGetString(peer->after_hdr, RPMTAG_RELEASE);

            if (ri.before) {
                before_rel = headerGetString(peer->before_hdr, RPMTAG_RELEASE);
            }

            ri.product_release = get_product_release(ri.favor_release, before_rel, after_rel);
            DEBUG_PRINT("product_release=%s\n", ri.product_release);

            if (ri.product_release == NULL) {
                fflush(stderr);
                free_rpminspect(&ri);
                return RI_PROGRAM_ERROR;
            }
        }

        for (i = 0; inspections[i].flag != 0; i++) {
            /* test not selected by user */
            if (!(ri.tests & inspections[i].flag)) {
                continue;
            }

            /* inspection requires before/after builds and we have one */
            if (ri.before == NULL && !inspections[i].single_build) {
                continue;
            }

            inspections[i].driver(&ri);
        }

        /* output the results */
        if (formatidx == -1) {
            formatidx = 0;                 /* default to 'text' output */
        }

        if (ri.results != NULL) {
            formats[formatidx].driver(ri.results, output);
        }
    }

    /* Set exit code based on result threshold */
    if (ri.worst_result >= ri.threshold) {
        ret = RI_INSPECTION_FAILURE;
    }

    /* Clean up */
    if (keep) {
        printf(_("\nKeeping working directory: %s\n"), ri.worksubdir);
    } else {
        if (rmtree(ri.workdir, true, true)) {
           fprintf(stderr, _("*** Error removing directory %s: %s\n"), ri.workdir, strerror(errno));
           fflush(stderr);
        }
    }

    free_rpminspect(&ri);
    rpmFreeRpmrc();

    return ret;
}
