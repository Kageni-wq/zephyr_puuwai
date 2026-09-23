# SPDX-License-Identifier: Apache-2.0

# The pinned vendor HAL predates the native Alif port. Its Kconfig extends
# shared Bluetooth and linker symbols without restricting them to Alif SoCs.
# Scope that module's generated source entry without modifying the HAL checkout.
if("alif" IN_LIST ZEPHYR_MODULE_NAMES)
  file(READ "${kconfig_modules_file}" alif_kconfig_modules)
  string(REGEX REPLACE
    "(menu \"alif [^\n]*\n)(osource [^\n]*\n)"
    "\\1if ALIF_VENDOR\n\\2endif # ALIF_VENDOR\n"
    alif_kconfig_modules "${alif_kconfig_modules}"
  )
  file(WRITE "${kconfig_modules_file}" "${alif_kconfig_modules}")
  unset(alif_kconfig_modules)
endif()
