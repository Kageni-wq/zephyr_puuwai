/* SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>
#include <soc_memory_map.h>
#include <ethosu_driver.h>

uint64_t ethosu_address_remap(uint64_t address, int index)
{
	ARG_UNUSED(index);
	return local_to_global((const void *)(uintptr_t)address);
}
