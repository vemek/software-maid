#include <libdevmapper.h>

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	/* Let's try something basic.
	 * List the active dm devices:
	 */

	struct dm_task *dmt;
	struct dm_names *names;
	unsigned next = 0;

	if (!(dmt = dm_task_create(DM_DEVICE_LIST))) {
		printf("could not create dm task\n");
		return 1;
	}
	if (!dm_task_run(dmt) || !(names = dm_task_get_names(dmt))) {
		printf("error running task\n");
		dm_task_destroy(dmt);
		return 1;
	}

	if (!names->dev) {
		printf("no devices set up!\n");
		dm_task_destroy(dmt);
		return 0;
	}

	do {
		names = (struct dm_names *)((char *) names + next);
		printf("%s\t(%d, %d)\n", names->name, (int) major(names->dev), (int) minor(names->dev));
		next = names->next;
	} while (next);

	dm_task_destroy(dmt);

	return 0;
}
