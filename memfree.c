#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#define pagetok(size) ((size) << pageshift)
#define MAX_STRING_LENGTH	16 
#define LOG1024 		10

static int pageshift;

struct meminfo_s {
	int total;
	int used;
	int free;
	int shared;
	int buffers;
	int cached;
	int swap_total;
	int swap_used;
	int swap_free;
} meminfo;

char *
format_k(int amt)
{
	char *ret = NULL; 
	char tag = 'K';
        if (amt >= 10000) {
                amt = (amt + 512) / 1024;
                tag = 'M';
                if (amt >= 10000) {
                        amt = (amt + 512) / 1024;
                        tag = 'G';
                }
        }
	size_t size = snprintf(NULL, 0, "%d%c", amt, tag);
	ret = malloc(size + 1);
        snprintf(ret, size + 1, "%d%c", amt, tag);
        return ret;
}

void
read_meminfo()
{
	static int uvmexp_mib[] = {CTL_VM, VM_UVMEXP};
	struct uvmexp uvmexp;
	size_t size;

	size = sizeof(uvmexp);
	if (sysctl(uvmexp_mib, 2, &uvmexp, &size, NULL, 0) < 0)
		err(1, "sysctl");

	pageshift = uvmexp.pageshift;
	pageshift -= LOG1024;

	meminfo.total = pagetok(uvmexp.npages);
	meminfo.used = pagetok(uvmexp.active);
	meminfo.free = pagetok(uvmexp.free);
	meminfo.swap_total = pagetok(uvmexp.swpages);
	meminfo.swap_used = pagetok(uvmexp.swpginuse);
	meminfo.swap_free = pagetok(uvmexp.swpages - uvmexp.swpginuse);
}

int
main(int argc, char *argv[])
{
	memset(&meminfo, 0, sizeof(struct meminfo_s));

	if (pledge("stdio vminfo", NULL) == -1)
		err(EXIT_FAILURE, "pledge");

	read_meminfo();

	if (pledge("stdio", NULL) == -1)
		err(EXIT_FAILURE, "pledge");


	printf("\ttotal\t\tused\t\tfree\t\tshared\t\tbuffers\t\tcached\n");	
        printf("%-7s\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s\n", "Mem:",
            format_k(meminfo.total),
            format_k(meminfo.used),
            format_k(meminfo.free),
            format_k(meminfo.shared),
            format_k(meminfo.buffers),
            format_k(meminfo.cached)
	);

	int bufcache = meminfo.buffers + meminfo.cached;
        printf(
                "-/+ buffers/cache:\t%s\t\t%s\n", 
                format_k(meminfo.used + bufcache),
                format_k(meminfo.free - bufcache) 
	);
	printf(
            "%-7s\t%s\t\t%s\t\t%s\n", "Swap:",
            format_k(meminfo.swap_total),
            format_k(meminfo.swap_used),
            format_k(meminfo.swap_free)
	);
	return 0;
}
