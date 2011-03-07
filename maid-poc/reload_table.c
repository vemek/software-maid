#include <libdevmapper.h>

#include <stdio.h>
#include <stdlib.h>

static int _set_task_device(struct dm_task *dmt, const char *name, int optional)
{
    if (name) {
        if (!dm_task_set_name(dmt, name))
            return 0;
    } else if (!optional) {
        fprintf(stderr, "No device specified.\n");
        return 0;
    }

    return 1;
}


int main(int argc, char** argv) {
	/*
	 * Dump a devices table. First arg should be the device name.
	 *
	 */
	char* name;
	if (argc < 2) {
		printf("usage: reload_table <device name>\n");
		return 1;
	} else {
		name = argv[1];
	}
	struct dm_task *dmt;
	struct dm_info info;
	struct dm_names *names;
	uint64_t start, length;
	char* params, * target_type;
	void* next = NULL;

	if (!(dmt = dm_task_create(DM_DEVICE_TABLE))) {
		printf("could not create dm task\n");
		return 1;
	}

	if (!_set_task_device(dmt, name, 0)) {
		printf("device %s may not exist -?\n", name);
	}

	if (!dm_task_run(dmt)) {
		printf("error running task\n");
		dm_task_destroy(dmt);
		return 1;
	}

	if (!dm_task_get_info(dmt, &info) || !info.exists) {
		printf("could not get info for %s\n", name);
		return 1;
	}

	/* not sure about this bit */
	do {
		next = dm_get_next_target(dmt, next, &start, &length, &target_type, &params);
		printf("%" PRIu64 " %" PRIu64 " %s %s\n", start, length, target_type, params);
	} while(next);


	dm_task_destroy(dmt);

	return 0;
}
