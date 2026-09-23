# Alif Ensemble and Balletto port

The vendor board and driver integration is available in this checkout. All 25
vendor board/core targets compile and link. The Puuwai application build has
been attempted but does **not** pass; the application blockers are recorded below.

## Sources

- Alif Zephyr: `E:/SDK/sdk-alif/zephyr`, revision
  `97fddffd316f63f9325545ec5c3dfc9a5034d831` (Zephyr 4.1 based).
- Earlier checkout: `E:/SDK/zephyr_alif/main/zephyr`, revision
  `f002a4d8499`; its history is included in the selected newer vendor revision.
- Additional Alif SDK drivers: `E:/SDK/sdk-alif/alif`.
- HAL: `hal_alif` at `41d6109e6b4fc512cbffdba7d82f7b38d4589bfd`, pinned in
  `west.yml`. The HAL checkout is unchanged; compatibility changes live here.
- Destination: this Zephyr 4.4.99 tree. Build toolchain: Zephyr SDK 1.0.1,
  GNU Arm GCC 14.3.0, Windows.

## Integration

The nine imported board families are `alif_b1_dk`, `alif_b1_eb`, `alif_b1_sk`,
`alif_e1c_dk`, `alif_e1c_sk`, `alif_e7_ak`, `alif_e7_dk`, `alif_e8_ak`, and
`alif_e8_dk`. Their 25 DTS targets include RTSS-HP, RTSS-HE and available APSS
configurations. SoC and DTS definitions are isolated under `alif/vendor` and
selected by `CONFIG_ALIF_VENDOR`.

Driver integration covers analog peripherals, GPIO/pinctrl/clocks, UART, DMA,
I2C and low-power I2C, SPI/OSPI, timers/RTC/PWM/quadrature decoding, CRC, entropy,
hardware information, watchdog, MRAM/flash/PSRAM, CAN, Ethernet/MDIO/PHY,
SDHC, USB DWC3 device mode, I2S/PDM/audio codecs, Bluetooth HCI, IEEE 802.15.4,
displays/DSI/D-PHY, camera sensors/CSI/CPI/ISP/JPEG and Ethos-U address translation.
Additional SDK audio support includes I2S sync, WM8904 and SI570.

Shared DesignWare, PL330, NS16550 and SDHC variants live in Alif-specific driver
directories. Current native drivers remain in place. Current video, CRC, UART,
CAN, PHY, USB and Bluetooth APIs are adapted rather than reverting public APIs
to the vendor's older Zephyr version. The HAL's global Kconfig entry is scoped
to vendor targets in `modules/alif/compat.cmake`.

See [E7 board documentation](boards/alif/alif_e7_dk/doc/index.rst) for build
commands, memory reservations, runner setup and compatibility boundaries.

## Compile results

| Check | Result |
| --- | --- |
| Hello-world across all vendor boards and cores | 25/25 linked |
| E7 HP and HE peripheral profiles | Linked |
| E7 Ethernet, USB device, SDHC, OSPI profile | Linked |
| E7 camera, display, DSI/D-PHY and PDM profile | Linked |
| E7 power management including suspend-to-RAM | Linked |
| E8 ISP, JPEG, Ethos-U, PSRAM, I3C and additional cameras | Linked |
| Balletto B1 Bluetooth HCI profile | Linked |
| Balletto B1 native IEEE 802.15.4 profile | Linked |
| Balletto B1 WM8904, SI570, I2S sync and display | Linked |
| E7 CRC known-vector/chunk-boundary test | Linked; not executed |
| Existing native Ensemble E8 HP hello-world | Linked |
| Twister profile discovery | Eight scenarios discovered |
| Twister build-only suite, including HP/HE CRC images | 11/11 linked; none executed |
| Alif flash runner context/help | Passed; no programming performed |

Build logs and images are in the workspace's `build` directory, one directory
above this repository. The board matrix is `build/alif_board_matrix.json`.
The E7 HP hello-world image is `build/alif_e7_hp_sdk1/zephyr/zephyr.elf`.
The final E7 connectivity, B1 Bluetooth and B1 audio images are in
`alif_e7_connectivity_final`, `alif_b1_radio_final`, and `alif_b1_audio_final`.
The E8 advanced profile is in `alif_e8_advanced`.
The consolidated test report is `build/alif_twister_short/twister.json`.
On Windows, use Twister's `--short-build-path` option; the standard nested
output paths exceed the GNU archiver's path-length limit in this workspace.

These are compiler/linker checks. No target has been flashed or run. The build
profiles exercise selected configurations, not every Kconfig combination.
In particular, OpenThread/encrypted 802.15.4 variants and the vendor ROM BLE
host stack have not been validated. The pinned radio HAL still selects the
deprecated `RING_BUFFER` symbol. Hardware testing remains necessary for DMA
coherency, USB enumeration, radio operation, camera pipelines and power states.
Vendor licenses are retained, including the Alif-specific license for SDK
audio components. The private video adapter and supervisor extensions are
described in the E7 documentation.

## Puuwai application build attempts

No directory with the exact name `puuwai_v0.03` was found. Two likely checkouts
were tested with `alif_e7_dk/ae722f80f55d5xx/rtss_hp`:

1. `E:/code/puuwai/puuwai_v0.3/main`: configuration stops because
   `main/CMakeLists.txt` appends the STM32 `dk_h747_hp.overlay` for every board
   except `puuwai_h745`. That overlay references `&mailbox`, which is absent
   on the Alif target. Log: `build/puuwai_v03_e7.log`.
2. `E:/code/nrf/puuwai_0.03`: board discovery stops at the external hardware
   module's `boards/vendor/custom_plank/board.yml`, whose schema is incompatible
   with this Zephyr version. Log: `build/puuwai_003_e7.log`.

Neither application checkout was changed. The main-MCU application needs an
explicit E7 board selection path, E7-specific overlays/configuration and an
Alif mapping for its board-level devices and inter-core transport before an
application-level build can succeed. A successful board sample build is not
a successful Puuwai firmware build.
