/*
 * Copyright (C) 2011 Mark King - mk429@netsoc.tcd.ie
 *
 * This file is released under the GPL.
 *
 * This daemon monitors MAID arrays and puts them to sleep,
 * if it is appropriate to do so. It is also responsible for
 * flushing the cache at regular intervals.
 */

#include <linux/device-mapper.h>


