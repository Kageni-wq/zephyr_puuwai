/* SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_DRIVERS_VIDEO_ALIF_VIDEO_COMPAT_H_
#define ZEPHYR_DRIVERS_VIDEO_ALIF_VIDEO_COMPAT_H_

#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video/alif-controls.h>
#include "video_common.h"
#include <zephyr/devicetree/port-endpoint.h>

enum alif_video_endpoint {
	ALIF_VIDEO_EP_ALL = -2,
	ALIF_VIDEO_EP_IN = -3,
	ALIF_VIDEO_EP_OUT = -4,
};

/* Keep the vendor pipeline's endpoint model private to these drivers. */
struct alif_video_ops {
	int (*set_format)(const struct device *, enum alif_video_endpoint, struct video_format *);
	int (*get_format)(const struct device *, enum alif_video_endpoint, struct video_format *);
	int (*set_stream)(const struct device *, bool);
	int (*get_caps)(const struct device *, enum alif_video_endpoint, struct video_caps *);
	int (*enqueue)(const struct device *, enum alif_video_endpoint, struct video_buffer *);
	int (*dequeue)(const struct device *, enum alif_video_endpoint, struct video_buffer **,
		       k_timeout_t);
	int (*flush)(const struct device *, enum alif_video_endpoint, bool);
	int (*set_ctrl)(const struct device *, unsigned int, void *);
	int (*get_ctrl)(const struct device *, unsigned int, void *);
	int (*set_signal)(const struct device *, enum alif_video_endpoint, struct k_poll_signal *);
	int (*set_frmival)(const struct device *, enum alif_video_endpoint, struct video_frmival *);
	int (*get_frmival)(const struct device *, enum alif_video_endpoint, struct video_frmival *);
	int (*enum_frmival)(const struct device *, enum alif_video_endpoint,
			    struct video_frmival_enum *);
};

struct alif_video_adapter {
	const struct video_driver_api *api;
	const struct alif_video_ops *ops;
};

int alif_video_set_format(const struct device *dev, enum alif_video_endpoint ep,
			  struct video_format *fmt);
int alif_video_get_format(const struct device *dev, enum alif_video_endpoint ep,
			  struct video_format *fmt);
int alif_video_get_caps(const struct device *dev, enum alif_video_endpoint ep,
			struct video_caps *caps);
int alif_video_flush(const struct device *dev, enum alif_video_endpoint ep, bool cancel);
int alif_video_stream_start(const struct device *dev);
int alif_video_stream_stop(const struct device *dev);

int alif_video_native_set_format(const struct device *dev, struct video_format *fmt);
int alif_video_native_get_format(const struct device *dev, struct video_format *fmt);
int alif_video_native_get_caps(const struct device *dev, struct video_caps *caps);
int alif_video_native_set_stream(const struct device *dev, bool enable, enum video_buf_type type);
int alif_video_native_enqueue(const struct device *dev, struct video_buffer *buf);
int alif_video_native_dequeue(const struct device *dev, struct video_buffer **buf, k_timeout_t timeout);
int alif_video_native_flush(const struct device *dev, bool cancel);
int alif_video_native_set_ctrl(const struct device *dev, uint32_t cid);
int alif_video_native_get_ctrl(const struct device *dev, uint32_t cid);
int alif_video_native_set_signal(const struct device *dev, struct k_poll_signal *signal);

#define ALIF_VIDEO_API(name, ...)                                                                   \
	static const struct alif_video_ops name##_ops = { __VA_ARGS__ };                            \
	static DEVICE_API(video, name) = {                                                         \
		.set_format = alif_video_native_set_format,                                        \
		.get_format = alif_video_native_get_format,                                        \
		.get_caps = alif_video_native_get_caps,                                            \
		.set_stream = alif_video_native_set_stream,                                        \
		.enqueue = alif_video_native_enqueue,                                              \
		.dequeue = alif_video_native_dequeue,                                              \
		.flush = alif_video_native_flush,                                                  \
		.set_ctrl = alif_video_native_set_ctrl,                                            \
		.get_volatile_ctrl = alif_video_native_get_ctrl,                                    \
		.set_signal = alif_video_native_set_signal,                                        \
	};                                                                                        \
	static const STRUCT_SECTION_ITERABLE(alif_video_adapter, name##_adapter) = {               \
		.api = &name, .ops = &name##_ops,                                                  \
	}

#endif
