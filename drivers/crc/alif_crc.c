/*
 * Copyright (C) 2024 Alif Semiconductor.
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#define DT_DRV_COMPAT alif_alif_crc

#include <zephyr/drivers/crc.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/device_mmio.h>
#include "alif_crc_reg.h"

struct alif_crc_data {
	DEVICE_MMIO_RAM;
	struct k_sem lock;
	struct crc_ctx *active;
	uint8_t pending[4];
	uint8_t pending_len;
	uint8_t width;
};

static uint32_t alif_crc_reverse(uint32_t value, uint8_t width)
{
	uint32_t result = 0U;

	for (uint8_t i = 0U; i < width; i++) {
		result = (result << 1U) | (value & 1U);
		value >>= 1U;
	}
	return result;
}

static int alif_crc_begin(const struct device *dev, struct crc_ctx *ctx)
{
	struct alif_crc_data *data = dev->data;
	uintptr_t base = DEVICE_MMIO_GET(dev);
	uint32_t control;
	uint32_t polynomial;
	uint8_t width;

	if (ctx == NULL || (ctx->reversed & ~(CRC_FLAG_REVERSE_INPUT |
					    CRC_FLAG_REVERSE_OUTPUT)) != 0U) {
		return -EINVAL;
	}
	switch (ctx->type) {
	case CRC8:
	case CRC8_CCITT:
		width = 8U;
		control = CRC_8_CCITT | CRC_ALGO_8_BIT_SIZE;
		polynomial = 0x07U;
		break;
	case CRC16:
	case CRC16_ANSI:
		width = 16U;
		control = CRC_16 | CRC_ALGO_16_BIT_SIZE;
		polynomial = 0x8005U;
		break;
	case CRC16_CCITT:
	case CRC16_ITU_T:
		width = 16U;
		control = CRC_16_CCITT | CRC_ALGO_16_BIT_SIZE;
		polynomial = 0x1021U;
		break;
	case CRC32_IEEE:
	case CRC32_MPEG2:
		width = 32U;
		control = CRC_32 | CRC_ALGO_32_BIT_SIZE;
		polynomial = CRC_32_STANDARD_POLY;
		break;
	case CRC32_C:
		width = 32U;
		control = CRC_32C | CRC_ALGO_32_BIT_SIZE;
		polynomial = CRC_32C_STANDARD_POLY;
		break;
	default:
		return -ENOTSUP;
	}
	if (width != 32U && ctx->polynomial != polynomial) {
		return -ENOTSUP;
	}
	if (ctx == data->active) {
		return -EBUSY;
	}
	k_sem_take(&data->lock, K_FOREVER);
	if (width == 32U && ctx->polynomial != polynomial) {
		control |= CRC_CUSTOM_POLY;
		sys_write32(ctx->polynomial, base + CRC_POLY_CUSTOM);
	}
	data->width = width;
	data->pending_len = 0U;
	data->active = ctx;
	sys_write32(ctx->seed, base + CRC_SEED);
	/* Keep the hardware accumulator unreflected until finish(). */
	sys_write32(control | CRC_INIT_BIT, base + CRC_CONTROL);
	ctx->result = ctx->seed;
	ctx->state = CRC_STATE_IN_PROGRESS;
	return 0;
}

static int alif_crc_update(const struct device *dev, struct crc_ctx *ctx,
			   const void *buffer, size_t length)
{
	struct alif_crc_data *data = dev->data;
	uintptr_t base = DEVICE_MMIO_GET(dev);
	const uint8_t *bytes = buffer;

	if (ctx == NULL || data->active != ctx || ctx->state != CRC_STATE_IN_PROGRESS ||
	    (buffer == NULL && length != 0U)) {
		return -EINVAL;
	}
	for (size_t i = 0U; i < length; i++) {
		uint8_t byte = bytes[i];

		if ((ctx->reversed & CRC_FLAG_REVERSE_INPUT) != 0U) {
			byte = alif_crc_reverse(byte, 8U);
		}
		if (data->width < 32U) {
			sys_write8(byte, base + CRC_DATA_IN_8_0);
		} else {
			/* Keep partial words across updates; the 32-bit engine
			 * accepts complete words only.
			 */
			data->pending[data->pending_len++] = byte;
			if (data->pending_len == sizeof(data->pending)) {
				sys_write32(sys_get_be32(data->pending), base + CRC_DATA_IN_32_0);
				data->pending_len = 0U;
			}
		}
	}
	return 0;
}

static int alif_crc_finish(const struct device *dev, struct crc_ctx *ctx)
{
	struct alif_crc_data *data = dev->data;
	uint32_t result;

	if (ctx == NULL || data->active != ctx || ctx->state != CRC_STATE_IN_PROGRESS) {
		return -EINVAL;
	}
	result = sys_read32(DEVICE_MMIO_GET(dev) + CRC_OUT);
	/* Complete at most three bytes without padding the message. */
	for (uint8_t i = 0U; i < data->pending_len; i++) {
		result ^= (uint32_t)data->pending[i] << 24U;
		for (uint8_t bit = 0U; bit < 8U; bit++) {
			result = (result << 1U) ^ ((result & BIT(31)) != 0U ?
						  ctx->polynomial : 0U);
		}
	}
	if ((ctx->reversed & CRC_FLAG_REVERSE_OUTPUT) != 0U) {
		result = alif_crc_reverse(result, data->width);
	}
	if (ctx->type == CRC32_IEEE || ctx->type == CRC32_C) {
		result ^= UINT32_MAX;
	}
	ctx->result = result & (UINT32_MAX >> (32U - data->width));
	ctx->state = CRC_STATE_IDLE;
	data->active = NULL;
	k_sem_give(&data->lock);
	return 0;
}

static DEVICE_API(crc, alif_crc_api) = {
	.begin = alif_crc_begin,
	.update = alif_crc_update,
	.finish = alif_crc_finish,
};

static int alif_crc_init(const struct device *dev)
{
	struct alif_crc_data *data = dev->data;

	DEVICE_MMIO_MAP(dev, K_MEM_CACHE_NONE);
	k_sem_init(&data->lock, 1, 1);
	return 0;
}

#define ALIF_CRC_DEFINE(n)                                                                          \
	static struct alif_crc_data alif_crc_data_##n;                                              \
	static const struct crc_config alif_crc_config_##n = {                                      \
		DEVICE_MMIO_ROM_INIT(DT_DRV_INST(n)),                                               \
	};                                                                                        \
	DEVICE_DT_INST_DEFINE(n, alif_crc_init, NULL, &alif_crc_data_##n, &alif_crc_config_##n,       \
			      POST_KERNEL, CONFIG_CRC_DRIVER_INIT_PRIORITY, &alif_crc_api);

DT_INST_FOREACH_STATUS_OKAY(ALIF_CRC_DEFINE)
