/* SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_DRIVERS_SENSOR_BMI323_I3C_H_
#define ZEPHYR_DRIVERS_SENSOR_BMI323_I3C_H_

#include <zephyr/drivers/i3c.h>
#include "bmi323.h"

struct bmi323_i3c_context {
	const struct device *bus;
	struct i3c_device_id id;
};

extern const struct bosch_bmi323_bus_api bosch_bmi323_i3c_bus_api;

#define BMI323_DEVICE_I3C_BUS(inst)                                                          \
	static const struct bmi323_i3c_context i3c_spec##inst = {                            \
		.bus = DEVICE_DT_GET(DT_INST_BUS(inst)),                                    \
		.id = I3C_DEVICE_ID_DT_INST(inst),                                          \
	};                                                                                 \
	static const struct bosch_bmi323_bus bosch_bmi323_bus_api##inst = {                   \
		.context = &i3c_spec##inst, .api = &bosch_bmi323_i3c_bus_api,                 \
	}
#endif
