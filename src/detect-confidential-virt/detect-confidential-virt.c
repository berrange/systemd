/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>

#include "alloc-util.h"
#include "build.h"
#include "main-func.h"
#include "pretty-print.h"
#include "string-table.h"
#include "conf-virt.h"

static bool arg_quiet = false;

static int help(void) {
        _cleanup_free_ char *link = NULL;
        int r;

        r = terminal_urlify_man("systemd-detect-confidential-virt", "1", &link);
        if (r < 0)
                return log_oom();

        printf("%s [OPTIONS...]\n\n"
               "Detect execution in a confidential virtualized environment.\n\n"
               "  -h --help             Show this help\n"
               "     --version          Show package version\n"
               "  -q --quiet            Don't output anything, just set return value\n"
               "     --list             List all known and detectable types of virtualization\n"
               "\nSee the %s for details.\n",
               program_invocation_short_name,
               link);

        return 0;
}

static int parse_argv(int argc, char *argv[]) {

        enum {
                ARG_VERSION = 0x100,
                ARG_LIST,
        };

        static const struct option options[] = {
                { "help",          no_argument, NULL, 'h'               },
                { "version",       no_argument, NULL, ARG_VERSION       },
                { "quiet",         no_argument, NULL, 'q'               },
                { "list",          no_argument, NULL, ARG_LIST          },
                {}
        };

        int c;

        assert(argc >= 0);
        assert(argv);

        while ((c = getopt_long(argc, argv, "hqcvr", options, NULL)) >= 0)

                switch (c) {

                case 'h':
                        return help();

                case ARG_VERSION:
                        return version();

                case 'q':
                        arg_quiet = true;
                        break;

                case ARG_LIST:
                        DUMP_STRING_TABLE(confidential_virtualization, ConfidentialVirtualization, _CONFIDENTIAL_VIRTUALIZATION_MAX);
                        return 0;

                case '?':
                        return -EINVAL;

                default:
                        assert_not_reached();
                }

        if (optind < argc)
                return log_error_errno(SYNTHETIC_ERRNO(EINVAL),
                                       "%s takes no arguments.",
                                       program_invocation_short_name);

        return 1;
}

static int run(int argc, char *argv[]) {
        ConfidentialVirtualization v;
        int r;

        /* This is mostly intended to be used for scripts which want
         * to detect whether we are being run in a virtualized
         * environment or not */

        log_setup();

        r = parse_argv(argc, argv);
        if (r <= 0)
                return r;

        v = detect_confidential_virtualization();
        if (v < 0)
                return log_error_errno(v, "Failed to check for confidential virtualization: %m");

        if (!arg_quiet)
                puts(confidential_virtualization_to_string(v));

        return v == CONFIDENTIAL_VIRTUALIZATION_NONE;
}

DEFINE_MAIN_FUNCTION_WITH_POSITIVE_FAILURE(run);
