/* SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_INCLUDE_DRIVERS_VIDEO_ALIF_CONTROLS_H_
#define ZEPHYR_INCLUDE_DRIVERS_VIDEO_ALIF_CONTROLS_H_
#include <zephyr/drivers/video/video_alif.h>
/** Set a vendor control, including aggregate ISP parameters and JPEG input buffers.
 * @param dev Alif video device.
 * @param id Control identifier from video_alif.h or video/controls.h.
 * @param value Pointer to the control's scalar or aggregate value.
 * @retval 0 Success.
 * @retval -EINVAL Invalid value.
 * @retval -ENOTSUP Unsupported control.
 */
int alif_video_set_control(const struct device *dev, uint32_t id, void *value);
/** Read a vendor control.
 * @param dev Alif video device.
 * @param id Control identifier.
 * @param value Storage for the control's scalar or aggregate value.
 * @retval 0 Success.
 * @retval -EINVAL Invalid value.
 * @retval -ENOTSUP Unsupported control.
 */
int alif_video_get_control(const struct device *dev, uint32_t id, void *value);
#endif
