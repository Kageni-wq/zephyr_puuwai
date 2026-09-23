# Copyright 2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources(ethos_u_common.c)

if(CONFIG_ALIF_VENDOR AND CONFIG_CPU_CORTEX_M)
  zephyr_library_sources(ethos_u_alif.c)
endif()

# zephyr-keep-sorted-start
zephyr_library_sources_ifdef(CONFIG_ETHOS_U_ARM ethos_u_arm.c)
zephyr_library_sources_ifdef(CONFIG_ETHOS_U_INFINEON ethos_u_infineon.c)
zephyr_library_sources_ifdef(CONFIG_ETHOS_U_NUMAKER ethos_u_numaker.c)
zephyr_library_sources_ifdef(CONFIG_ETHOS_U_RENESAS ethos_u_renesas.c)
# zephyr-keep-sorted-stop
