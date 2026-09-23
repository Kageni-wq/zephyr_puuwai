/* SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/drivers/crc.h>
#include <zephyr/ztest.h>

static const struct device *const crc = DEVICE_DT_GET(DT_NODELABEL(crc0));

/* Standard check values for the ASCII message "123456789". */
static const struct {
	struct crc_ctx ctx;
	uint32_t expected;
} vectors[] = {
	{{.type = CRC8, .polynomial = 0x07, .seed = 0}, 0xf4},
	{{.type = CRC16_ITU_T, .polynomial = 0x1021, .seed = 0}, 0x31c3},
	{{.type = CRC16_ANSI, .polynomial = 0x8005, .seed = 0,
	  .reversed = CRC_FLAG_REVERSE_INPUT | CRC_FLAG_REVERSE_OUTPUT}, 0xbb3d},
	{{.type = CRC32_IEEE, .polynomial = 0x04c11db7, .seed = UINT32_MAX,
	  .reversed = CRC_FLAG_REVERSE_INPUT | CRC_FLAG_REVERSE_OUTPUT}, 0xcbf43926},
	{{.type = CRC32_C, .polynomial = 0x1edc6f41, .seed = UINT32_MAX,
	  .reversed = CRC_FLAG_REVERSE_INPUT | CRC_FLAG_REVERSE_OUTPUT}, 0xe3069283},
	{{.type = CRC32_MPEG2, .polynomial = 0x04c11db7, .seed = UINT32_MAX}, 0x0376e6e7},
};

ZTEST(alif_crc, test_vectors_and_chunk_boundaries)
{
	static const uint8_t message[] = "123456789";

	zassert_true(device_is_ready(crc));
	for (size_t i = 0; i < ARRAY_SIZE(vectors); i++) {
		for (size_t chunk = 1; chunk <= sizeof(message) - 1; chunk++) {
			struct crc_ctx ctx = vectors[i].ctx;

			zassert_ok(crc_begin(crc, &ctx));
			zassert_ok(crc_update(crc, &ctx, NULL, 0));
			for (size_t offset = 0; offset < sizeof(message) - 1; offset += chunk) {
				zassert_ok(crc_update(crc, &ctx, message + offset,
					MIN(chunk, sizeof(message) - 1 - offset)));
			}
			zassert_ok(crc_finish(crc, &ctx));
			zassert_equal(ctx.result, vectors[i].expected,
				      "vector %u, chunk %u", (unsigned int)i, (unsigned int)chunk);
		}
	}
}

ZTEST_SUITE(alif_crc, NULL, NULL, NULL, NULL, NULL);
