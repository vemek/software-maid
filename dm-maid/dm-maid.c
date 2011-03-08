/*
 * Copyright (C) 2011 Mark King - mk429@netsoc.tcd.ie
 *
 * This file is released under the GPL.
 */

#include "dm.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/slab.h>
#include <linux/device-mapper.h>

/* what params might look like:
 * 0 [size of storage summed] maid [cache dev] {[storage dev]}
 *
 * might not need [size], could get from blkdev.h
 * FIXME: assuming we're using the whole device as the userspace tools
 * will be powering down everything anyway, so we should have
 * exclusive use of the device.
 *
 */

struct maid_cache_c {
	struct dm_dev *cache_dev;
	sector_t cache_sectors_len;
	sector_t **cached_sectors_table;
	struct dm_dev **storage_devs;
	int num_storage_devs;
}

static int maid_cache_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	struct maid_cache_c *mc;
	sector_t dev_size;
	int r;

	if (argc < 2 ) {
		ti->error = "Invalid argument count";
		return -EINVAL;
	}

	mc = kmalloc(sizeof(*mc), GFP_KERNEL);
	mc->cached_sectors_table = NULL;
	mc->storage_devs = NULL;
	mc->num_storage_devs = argc - 1;

	if (mc == NULL) {
		ti->error = "dm-maid-cache: Cannot allocate maid cache context";
		return -ENOMEM;
	}

	// get cache dev
	dev_size = 100; // FIXME: get actual sector size of device!!
	if (!dm_get_device(ti, argv[0], 0, dev_size,
			dm_table_get_mode(ti->table), &mc->cache_dev)) {
		ti-error = "dm-maid-cache: Device lookup failed for cache device";
		r = -EINVAL;
		goto bad;
	}
	mc->cache_sectors_len = dev_size;

	// set up cache lookup table
	mc->cached_sectors_table = kmalloc(sizeof(sector_t *) * mc->cache_sectors_len, GFP_KERNEL);
	if(mc->cached_sectors_table == NULL) {
		ti->error = "dm-maid-cache: Cannot allocate cache lookup table";
		r = -ENOMEM;
		goto bad;
	}

	// NULL cache table
	for (int i = 0; i < (int)mc->cache_sectors_len; i++) {
		mc->cached_sectors_table[i] = NULL;
	}

	// set up device lookup table
	mc->storage_devs = kmalloc(sizeof(void*) * mc->num_storage_devs, GFP_KERNEL);
	if (mc->storage_devs == NULL) {
		ti->error = "dm-maid-cache: Cannot allocate storage device lookup table";
		r = -ENOMEM;
		goto bad;
	}

	// get storage devices
	for (int i = 1; i < argc; i++) {
		dev_size = 100; // FIXME: get actual sector size of the device!!
		if(!dm_get_device(ti, argv[i], 0, dev_size,
				dm_table_get_mode(ti->table), &mc->storage_devs[i-1])) {
			ti->error = "dm-maid-cache: Device lookup failed for storage device";
			r = -EINVAL;
			goto bad;
		}
	}

	ti->private = mc;
	return 0;
	
bad:
	// RUH ROH - free everything
	if (mc->cached_sectors_table) {
		kfree(mc->cached_sectors_table);
	}
	if (mc->storage_devs) {
		kfree(mc->storage_devs);
	}
	kfree(mc);
	return r;
}

static void maid_cache_dtr(struct dm_target *ti)
{
	struct maid_cache_c *mc = (struct maid_cache_c *)ti->private;
	dm_put_device(ti, mc->cache_dev);
	for (int i = 0; i < mc->num_storage_devs; i++) {
		dm_put_device(ti, mc->storage_devs[i]);
	}
	kfree(mc->storage_devs);
	kfree(mc->cached_sectors_table);
	kfree(mc);
}

static int maid_cache_get_device_index(struct dm_target *ti, sector_t bi_sector)
{
	// TODO
	return 0;
}

static sector_t maid_cache_map_sector(struct dm_target *ti, int dev_index,
				sector_t bi_sector)
{
	// TODO
	return 0;
}

static void maid_cache_map_bio(struct dm_target *ti, struct bio *bio)
{
	// TODO
	return;
}

static int maid_cache_map(struct dm_target *ti, struct bio *bio,
				union map_info *map_context)
{
	// TODO: add some code to update the cache, switch on read or write
	maid_cache_map_bio(ti, bio);
	return DM_MAPIO_REMAPPED;
}

static struct target_type maid_cache_target = {
	.name   = "maid-cache",
	.version = {0, 0, 1},
	.module = THIS_MODULE,
	.ctr    = maid_cache_ctr,
	.dtr    = maid_cache_dtr,
	.map    = maid_cache_map, // TODO
	.status = maid_cache_status, // TODO
	.ioctl  = maid_cache_ioctl, // TODO
// need this be implemented?
//	.merge  = maid_cache_merge,
//	.iterate_devices = linear_iterate_devices,
};

int __init dm_maid_cache_init(void)
{
	int r = dm_register_target(&maid_cache_target);

	if (r < 0)
		DMERR("register failed %d", r);

	return r;
}

void dm_maid_cache_exit(void)
{
	dm_unregister_target(&maid_cache_target);
}
