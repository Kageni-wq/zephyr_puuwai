/* SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "alif_video_compat.h"

static const struct alif_video_ops *alif_video_ops(const struct device *dev)
{
	STRUCT_SECTION_FOREACH(alif_video_adapter, adapter) {
		if (adapter->api == dev->api) {
			return adapter->ops;
		}
	}
	return NULL;
}

static enum alif_video_endpoint alif_video_endpoint(enum video_buf_type type)
{
	return type == VIDEO_BUF_TYPE_INPUT ? ALIF_VIDEO_EP_IN : ALIF_VIDEO_EP_OUT;
}

static int alif_video_complete_format(struct video_format *fmt)
{
	struct video_format estimate = *fmt;
	uint64_t size;
	int ret;

	ret = video_estimate_fmt_size(&estimate);
	if (ret != 0 && fmt->pitch == 0U) {
		return ret;
	}
	if (fmt->pitch == 0U) {
		fmt->pitch = estimate.pitch;
	}
	/* Preserve the padded stride returned by the camera pipeline. */
	size = (uint64_t)fmt->pitch * fmt->height;
	if (ret == 0) {
		if (estimate.pitch != 0U) {
			size = MAX(size, (uint64_t)estimate.size * fmt->pitch / estimate.pitch);
		} else {
			size = estimate.size;
		}
	}
	if (size > UINT32_MAX) {
		return -EOVERFLOW;
	}
	fmt->size = size;
	return 0;
}

int alif_video_set_format(const struct device *dev, enum alif_video_endpoint ep,
			  struct video_format *fmt)
{
	struct video_format remote = *fmt;
	int ret;

	remote.type = ep == ALIF_VIDEO_EP_IN ? VIDEO_BUF_TYPE_INPUT : VIDEO_BUF_TYPE_OUTPUT;
	ret = video_driver_set_format(dev, &remote);
	remote.type = fmt->type;
	*fmt = remote;
	return ret;
}

int alif_video_get_format(const struct device *dev, enum alif_video_endpoint ep,
			  struct video_format *fmt)
{
	struct video_format remote = *fmt;
	int ret;

	remote.type = ep == ALIF_VIDEO_EP_IN ? VIDEO_BUF_TYPE_INPUT : VIDEO_BUF_TYPE_OUTPUT;
	ret = video_driver_get_format(dev, &remote);
	remote.type = fmt->type;
	*fmt = remote;
	return ret;
}

int alif_video_get_caps(const struct device *dev, enum alif_video_endpoint ep,
			struct video_caps *caps)
{
	struct video_caps remote = *caps;
	int ret;

	remote.type = ep == ALIF_VIDEO_EP_IN ? VIDEO_BUF_TYPE_INPUT : VIDEO_BUF_TYPE_OUTPUT;
	ret = video_driver_get_caps(dev, &remote);
	remote.type = caps->type;
	*caps = remote;
	return ret;
}

int alif_video_flush(const struct device *dev, enum alif_video_endpoint ep, bool cancel)
{
	ARG_UNUSED(ep);
	return video_driver_flush(dev, cancel);
}

int alif_video_stream_start(const struct device *dev)
{
	return video_driver_set_stream(dev, true, VIDEO_BUF_TYPE_OUTPUT);
}

int alif_video_stream_stop(const struct device *dev)
{
	return video_driver_set_stream(dev, false, VIDEO_BUF_TYPE_OUTPUT);
}

int alif_video_set_control(const struct device *dev, uint32_t id, void *value)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);

	if (value == NULL) {
		return -EINVAL;
	}
	if (ops != NULL) {
		return ops->set_ctrl != NULL ? ops->set_ctrl(dev, id, value) : -ENOTSUP;
	}
	struct video_control ctrl = {.id = id, .val = *(int32_t *)value};

	return video_set_ctrl(dev, &ctrl);
}

int alif_video_get_control(const struct device *dev, uint32_t id, void *value)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);
	struct video_control ctrl = {.id = id};
	int ret;

	if (value == NULL) {
		return -EINVAL;
	}
	if (ops != NULL) {
		return ops->get_ctrl != NULL ? ops->get_ctrl(dev, id, value) : -ENOTSUP;
	}
	ret = video_get_ctrl(dev, &ctrl);
	if (ret == 0) {
		*(int32_t *)value = ctrl.val;
	}
	return ret;
}

int alif_video_native_set_format(const struct device *dev, struct video_format *fmt)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);
	int ret;

	if (fmt->type != VIDEO_BUF_TYPE_INPUT && fmt->type != VIDEO_BUF_TYPE_OUTPUT) {
		return -EINVAL;
	}
	ret = ops->set_format != NULL ?
		ops->set_format(dev, alif_video_endpoint(fmt->type), fmt) : -ENOSYS;
	return ret == 0 ? alif_video_complete_format(fmt) : ret;
}

int alif_video_native_get_format(const struct device *dev, struct video_format *fmt)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);
	enum video_buf_type type = fmt->type;
	int ret;

	if (type != VIDEO_BUF_TYPE_INPUT && type != VIDEO_BUF_TYPE_OUTPUT) {
		return -EINVAL;
	}
	ret = ops->get_format != NULL ?
		ops->get_format(dev, alif_video_endpoint(type), fmt) : -ENOSYS;
	fmt->type = type;
	return ret == 0 ? alif_video_complete_format(fmt) : ret;
}

int alif_video_native_get_caps(const struct device *dev, struct video_caps *caps)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);

	if (caps->type != VIDEO_BUF_TYPE_INPUT && caps->type != VIDEO_BUF_TYPE_OUTPUT) {
		return -EINVAL;
	}
	return ops->get_caps != NULL ?
		ops->get_caps(dev, alif_video_endpoint(caps->type), caps) : -ENOSYS;
}

int alif_video_native_set_stream(const struct device *dev, bool enable, enum video_buf_type type)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);

	if (type != VIDEO_BUF_TYPE_INPUT && type != VIDEO_BUF_TYPE_OUTPUT) {
		return -EINVAL;
	}
	return ops->set_stream != NULL ? ops->set_stream(dev, enable) : -ENOSYS;
}

int alif_video_native_enqueue(const struct device *dev, struct video_buffer *buf)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);

	if (buf->type != VIDEO_BUF_TYPE_INPUT && buf->type != VIDEO_BUF_TYPE_OUTPUT) {
		return -EINVAL;
	}
	return ops->enqueue != NULL ?
		ops->enqueue(dev, alif_video_endpoint(buf->type), buf) : -ENOSYS;
}

int alif_video_native_dequeue(const struct device *dev, struct video_buffer **buf, k_timeout_t timeout)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);

	return ops->dequeue != NULL ? ops->dequeue(dev, ALIF_VIDEO_EP_OUT, buf, timeout) : -ENOSYS;
}

int alif_video_native_flush(const struct device *dev, bool cancel)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);

	return ops->flush != NULL ? ops->flush(dev, ALIF_VIDEO_EP_ALL, cancel) : -ENOSYS;
}

static struct video_ctrl *alif_video_find_ctrl(const struct device *dev, uint32_t cid)
{
	struct video_device *vdev = video_find_vdev(dev);
	struct video_ctrl *ctrl;

	if (vdev == NULL) {
		return NULL;
	}
	SYS_DLIST_FOR_EACH_CONTAINER(&vdev->ctrls, ctrl, node) {
		if (ctrl->id == cid) {
			return ctrl;
		}
	}
	return NULL;
}

int alif_video_native_set_ctrl(const struct device *dev, uint32_t cid)
{
	struct video_ctrl *ctrl = alif_video_find_ctrl(dev, cid);

	return ctrl != NULL ? alif_video_set_control(dev, cid, &ctrl->val) : -ENOTSUP;
}

int alif_video_native_get_ctrl(const struct device *dev, uint32_t cid)
{
	struct video_ctrl *ctrl = alif_video_find_ctrl(dev, cid);

	return ctrl != NULL ? alif_video_get_control(dev, cid, &ctrl->val) : -ENOTSUP;
}

int alif_video_native_set_signal(const struct device *dev, struct k_poll_signal *signal)
{
	const struct alif_video_ops *ops = alif_video_ops(dev);

	return ops->set_signal != NULL ? ops->set_signal(dev, ALIF_VIDEO_EP_OUT, signal) : -ENOSYS;
}
