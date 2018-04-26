/* See LICENSE for license details */
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define pagetok(size) ((size) << pageshift)
#define LOG1024			10
#define NUM_STRINGS 	 	11

struct meminfo_s {
	int total;
	int used;
	int free;
	int64_t buffers;
	int64_t cached;
	int swap_total;
	int swap_used;
	int swap_free;
} meminfo;

/* This was taken from the top(1) source code (slightly modified) */
static char *
format_k(int64_t amt)
{
        static char retarray[NUM_STRINGS][16];
        static int  idx = 0;
        char *ret, tag = 'K';

        ret = retarray[idx];
        idx = (idx + 1) % NUM_STRINGS;

        if (amt >= 10000) {
                amt = (amt + 512) / 1024;
                tag = 'M';
                if (amt >= 10000) {
                        amt = (amt + 512) / 1024;
                        tag = 'G';
                }
        }
        snprintf(ret, sizeof(retarray[0]), "%lld%c", amt, tag);
        return (ret);
}

static void
read_meminfo()
{
	static int uvmexp_mib[] = {CTL_VM, VM_UVMEXP};
	static int bcstats_mib[] = {CTL_VFS, VFS_GENERIC, VFS_BCACHESTAT};
	static int pageshift;
	struct uvmexp uvmexp;
	struct bcachestats bcstats;
	size_t size;

	size = sizeof(uvmexp);
	if (sysctl(uvmexp_mib, 2, &uvmexp, &size, NULL, 0) < 0)
		err(1, "sysctl");

	size = sizeof(bcstats);
	if (sysctl(bcstats_mib, 3, &bcstats, &size, NULL, 0) < 0)
		err(1, "sysctl");

	pageshift = uvmexp.pageshift;
	pageshift -= LOG1024;

	meminfo.total = pagetok(uvmexp.npages);
	meminfo.used = pagetok(uvmexp.npages - uvmexp.free);
	meminfo.free = pagetok(uvmexp.free);

	/* not sure yet about the following two buffers... */
	meminfo.buffers = pagetok(bcstats.numbufs);
	meminfo.cached = pagetok(bcstats.numbufpages);

	meminfo.swap_total = pagetok(uvmexp.swpages);
	meminfo.swap_used = pagetok(uvmexp.swpgonly);
	meminfo.swap_free = pagetok(uvmexp.swpages - uvmexp.swpgonly);
}

static void
print_usage()
{
	fprintf(stderr, "Usage output here...\n");
}

int
main(int argc, char *argv[])
{
	int ch;
	while ((ch = getopt(argc, argv, "vh")) != -1) {
		switch (ch) {
			case 'v':
				fprintf(stderr, "memfree-"VERSION"\n");
				return 0;
			case 'h':
				print_usage();
				return 0;
			default:
				break;
		}
	}

	if (pledge("stdio vminfo", NULL) == -1)
		err(EXIT_FAILURE, "pledge");

	memset(&meminfo, 0, sizeof(struct meminfo_s));
	read_meminfo();

	if (pledge("stdio", NULL) == -1)
		err(EXIT_FAILURE, "pledge");


	printf("\ttotal\t\tused\t\tfree\t\tshared\t\tbuffers\t\tcached\n");
	printf("%-7s\t%s\t\t%s\t\t%s\t\t0K\t\t%s\t\t%s\n", "Mem:",
		format_k(meminfo.total),
		format_k(meminfo.used),
		format_k(meminfo.free),
		format_k(meminfo.buffers),
		format_k(meminfo.cached)
	);

	int64_t bufcache = meminfo.buffers + meminfo.cached;

	printf("-/+ buffers/cache:\t%s\t\t%s\n",
		format_k(meminfo.used - bufcache),
		format_k(meminfo.free + bufcache)
	);
	printf("%-7s\t%s\t\t%s\t\t%s\n", "Swap:",
		format_k(meminfo.swap_total),
		format_k(meminfo.swap_used),
		format_k(meminfo.swap_free)
	);
	return 0;
}
