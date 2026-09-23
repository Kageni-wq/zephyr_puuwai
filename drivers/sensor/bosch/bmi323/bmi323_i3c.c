/*
 * Copyright (c) 2025 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bmi323_i3c.h"
#include <zephyr/device.h>
#include <zephyr/drivers/i3c.h>

#define IMU_BOSCH_BMI323_REG_I3C_DUMMY_OFFSET 0x2

static int bosch_bmi323_i3c_read_words(const void *context, uint8_t offset, uint16_t *words,
				       uint16_t words_count)
{
	const struct bmi323_i3c_context *cfg = context;
	struct i3c_device_desc *i3c = i3c_device_find(cfg->bus, &cfg->id);
	uint8_t dbuf[(words_count * 2) + IMU_BOSCH_BMI323_REG_I3C_DUMMY_OFFSET];
	int ret;

	if (i3c == NULL) {
		return -ENODEV;
	}
	ret = i3c_burst_read(i3c, offset, dbuf, sizeof(dbuf));
	if (!ret) {
		/* Copy actual data except first 2 bytes of dummy data */
		memcpy(words, &dbuf[IMU_BOSCH_BMI323_REG_I3C_DUMMY_OFFSET], (words_count * 2));
	}
	k_usleep(2);

	return ret;
}

static int bosch_bmi323_i3c_write_words(const void *context, uint8_t offset, uint16_t *words,
					uint16_t words_count)
{
	const struct bmi323_i3c_context *cfg = context;
	struct i3c_device_desc *i3c = i3c_device_find(cfg->bus, &cfg->id);
	uint8_t dbuf[(words_count * 2) + sizeof(offset)];
	int ret;

	/* Prepare buffer with offset and data */
	dbuf[0] = offset;
	memcpy(&dbuf[sizeof(offset)], words, (words_count * 2));

	if (i3c == NULL) {
		return -ENODEV;
	}
	ret = i3c_write(i3c, dbuf, sizeof(dbuf));
	k_usleep(2);

	return ret;
}

static int bosch_bmi323_i3c_init(const void *context)
{
	const struct bmi323_i3c_context *cfg = context;
	struct i3c_device_desc *i3c = i3c_device_find(cfg->bus, &cfg->id);
	uint16_t sensor_id[2];
	int ret;

	if (!device_is_ready(cfg->bus) || i3c == NULL) {
		return -ENODEV;
	}

	if (i3c == NULL) {
		return -ENODEV;
	}
	ret = i3c_burst_read(i3c, IMU_BOSCH_BMI323_REG_CHIP_ID, (uint8_t *)sensor_id,
			(IMU_BOSCH_BMI323_REG_I3C_DUMMY_OFFSET * 2));
	if (ret < 0) {
		return ret;
	}

	return 0;
}

const struct bosch_bmi323_bus_api bosch_bmi323_i3c_bus_api = {
	.read_words = bosch_bmi323_i3c_read_words,
	.write_words = bosch_bmi323_i3c_write_words,
	.init = bosch_bmi323_i3c_init
};
