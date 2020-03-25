/**
 * ntfsvolume - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2002-2003 Richard Russon
 * Copyright (c) 2005 Anton Altaparmakov
 * Copyright (c) 2005-2006 Szabolcs Szakacsits
 *
 * This utility will locate the owner of any given sector or cluster.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "ntfscluster.h"
#include "types.h"
#include "attrib.h"
#include "utils.h"
#include "volume.h"
#include "debug.h"
#include "dir.h"
#include "volume.h"
/* #include "version.h" */
#include "logging.h"

static const char *EXEC_NAME = "ntfsvolume";
static struct options opts;

/**
 * version - Print version information about the program
 *
 * Print a copyright statement and a brief description of the program.
 *
 * Return:  none
 */
static void version(void)
{
	ntfs_log_info("\n%s v%s (libntfs-3g) - Print volume information.\n\n",
			 EXEC_NAME, VERSION);
	ntfs_log_info("Copyright (c) 2002-2003 Richard Russon\n");
	ntfs_log_info("Copyright (c) 2005 Anton Altaparmakov\n");
	ntfs_log_info("Copyright (c) 2005-2006 Szabolcs Szakacsits\n");
	ntfs_log_info("\n%s\n%s%s\n", ntfs_gpl, ntfs_bugs, ntfs_home);
}

/**
 * usage - Print a list of the parameters to the program
 *
 * Print a list of the parameters and options for the program.
 *
 * Return:  none
 */
static void usage(void)
{
	ntfs_log_info("\nUsage: %s [options] device\n"
		"    -f, --force          Use less caution\n"
		"    -q, --quiet          Less output\n"
		"    -v, --verbose        More output\n"
		"    -V, --version        Version information\n"
		"    -h, --help           Print this help\n\n",
		EXEC_NAME);
	ntfs_log_info("%s%s\n", ntfs_bugs, ntfs_home);
}

/**
 * parse_options - Read and validate the programs command line
 *
 * Read the command line, verify the syntax and parse the options.
 * This function is very long, but quite simple.
 *
 * Return:  1 Success
 *	    0 Error, one or more problems
 */
static int parse_options(int argc, char **argv)
{
	static const char *sopt = "-c:F:fh?I:ilqs:vV";
	static const struct option lopt[] = {
		{ "force",	no_argument,		NULL, 'f' },
		{ "help",	no_argument,		NULL, 'h' },
		{ "quiet",	no_argument,		NULL, 'q' },
		{ "verbose",	no_argument,		NULL, 'v' },
		{ "version",	no_argument,		NULL, 'V' },
		{ NULL,		0,			NULL, 0   }
	};

	int c = -1;
	int err  = 0;
	int ver  = 0;
	int help = 0;
	int levels = 0;

	opterr = 0; /* We'll handle the errors, thank you. */

	opts.range_begin = -1;
	opts.range_end   = -1;

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			if (!opts.device) {
				opts.device = argv[optind-1];
			} else {
				opts.device = NULL;
				err++;
			}
			break;

		case 'f':
			opts.force++;
			break;
		case 'h':
			help++;
			break;
		case 'q':
			opts.quiet++;
			ntfs_log_clear_levels(NTFS_LOG_LEVEL_QUIET);
			break;
		case 'v':
			opts.verbose++;
			ntfs_log_set_levels(NTFS_LOG_LEVEL_VERBOSE);
			break;
		case 'V':
			ver++;
			break;
		case '?':
			if (strncmp (argv[optind-1], "--log-", 6) == 0) {
				if (!ntfs_log_parse_option (argv[optind-1]))
					err++;
				break;
			}
			/* fall through */
		default:
            ntfs_log_error("Unknown option '%s'.\n", argv[optind-1]);
			err++;
			break;
		}
	}

	/* Make sure we're in sync with the log levels */
	levels = ntfs_log_get_levels();
	if (levels & NTFS_LOG_LEVEL_VERBOSE)
		opts.verbose++;
	if (!(levels & NTFS_LOG_LEVEL_QUIET))
		opts.quiet++;

	if (help || ver) {
		opts.quiet = 0;
	} else {
		if (opts.device == NULL) {
			if (argc > 1)
				ntfs_log_error("You must specify exactly one device.\n");
			err++;
		}

		if (opts.quiet && opts.verbose) {
			ntfs_log_error("You may not use --quiet and --verbose at the same time.\n");
			err++;
		}
	}

	if (ver)
		version();
	if (help || err)
		usage();

		/* tri-state 0 : done, 1 : error, -1 : proceed */
	return (err ? 1 : (help || ver ? 0 : -1));
}


/**
 * info
 */
static int info(ntfs_volume *vol)
{
	u64 a, b, c, d, e, f, g;
	int cb, sb, cps;

	cb  = vol->cluster_size_bits;
	sb  = vol->sector_size_bits;
	cps = cb - sb;

	a = vol->sector_size;
	b = vol->cluster_size;
	c = 1 << cps;
	d = vol->nr_clusters << cb;
	e = vol->nr_clusters;
	f = vol->nr_clusters >> cps;
	g = vol->mft_na->initialized_size >> vol->mft_record_size_bits;

	ntfs_log_info("bytes per sector        : %llu\n", (unsigned long long)a);
	ntfs_log_info("bytes per cluster       : %llu\n", (unsigned long long)b);
	ntfs_log_info("sectors per cluster     : %llu\n", (unsigned long long)c);
	ntfs_log_info("bytes per volume        : %llu\n", (unsigned long long)d);
	ntfs_log_info("sectors per volume      : %llu\n", (unsigned long long)e);
	ntfs_log_info("clusters per volume     : %llu\n", (unsigned long long)f);
	ntfs_log_info("initialized mft records : %llu\n", (unsigned long long)g);

	return 0;
}

/**
 * main - Begin here
 *
 * Start from here.
 *
 * Return:  0  Success, the program worked
 *	    1  Error, something went wrong
 */
int main(int argc, char *argv[])
{
	ntfs_volume *vol;
	int res;
	int result = 1;

	ntfs_log_set_handler(ntfs_log_handler_outerr);

	res = parse_options(argc, argv);
	if (res >= 0)
		return (res);

	utils_set_locale();

	vol = utils_mount_volume(opts.device, NTFS_MNT_RDONLY |
			(opts.force ? NTFS_MNT_RECOVER : 0));
	if (!vol)
		return 1;

    result = info(vol);

	ntfs_umount(vol, FALSE);
	return result;
}


