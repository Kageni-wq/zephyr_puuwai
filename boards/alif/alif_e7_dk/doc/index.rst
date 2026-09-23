.. zephyr:board:: alif_e7_dk

Overview
********

This target provides the Alif SDK board description for the Ensemble E7
Development Kit. The port includes the Cortex-M55 high-performance and
high-efficiency cores and the Cortex-A32 application processor configuration.
It uses the vendor SoC integration under ``soc/alif/vendor``.

The E7 board qualifiers are:

* ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
* ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
* ``alif_e7_dk/ae722f80f55d5xx/apss``
* ``alif_e7_dk/ae302f80f55d5xx/rtss_hp``
* ``alif_e7_dk/ae302f80f55d5xx/rtss_he``

The default M55 images execute from MRAM and use local DTCM for RAM. The HP
console is UART4. Select the qualifier matching the chip and core, and consult
the board DTS before assigning peripheral pins.

Dependencies and builds
***********************

The west manifest pins ``hal_alif``. Update it from the west workspace before
building::

   west update hal_alif
   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/hello_world

The commands in this page use paths relative to the Zephyr repository. Select
this checkout through ``ZEPHYR_BASE`` when multiple Zephyr trees are installed.
Zephyr SDK 1.0.1 with GNU Arm GCC 14.3.0 was used for the compile checks.

To check peripheral integration::

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp tests/drivers/alif/compile

Additional build profiles live alongside that test application's ``prj.conf``.
For example::

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp tests/drivers/alif/compile -- -DEXTRA_CONF_FILE=connectivity.conf -DEXTRA_DTC_OVERLAY_FILE=connectivity.overlay

Twister metadata covers the E7 peripheral, connectivity, media and power
management profiles, E8 accelerators, and Balletto B1 audio and radio profiles::

   west twister --build-only -T tests/drivers/alif/compile -p alif_e7_dk/ae722f80f55d5xx/rtss_hp
   west twister --build-only -T tests/drivers/alif/compile -p alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   west twister --build-only -T tests/drivers/alif/compile -p alif_b1_dk/ab1c1f4m51820hh0/rtss_he

On Windows, add ``--short-build-path`` to avoid compiler-tool path-length
limits in Twister's nested build directories.

These overlays deliberately enable devices for compilation. Their simultaneous
pin assignments and attached camera or audio devices are not a verified wiring
configuration for a physical board.

Memory and power management
***************************

DMA peripherals require suitable globally addressable memory. The connectivity
profile reserves the final 96 KiB of HP DTCM for non-secure DMA buffers and
reduces the ordinary RAM region accordingly. The Balletto audio profile uses a
separate 512 KiB reservation for its framebuffer. Preserve non-overlapping
regions when changing these overlays or enabling additional peripherals.

Deep power management requires the companion counter selected in ``pm.overlay``
and the corresponding Secure Enclave services. The suspend-to-RAM path retains
Zephyr's native CPU context handling and adds private Alif state for the MPU,
SAU, NVIC and associated system registers. Successful linking does not establish
that retention, wake sources or device resume sequencing work on hardware.

Programming and debugging
*************************

``alif_flash`` uses Alif's Secure Enclave tools for programming and delegates
debugging to Zephyr's J-Link runner. Install the SE tools separately, configure
them for the target device, and set ``ALIF_SE_TOOLS_DIR`` to their directory.
The runner supports native executables and Python versions of the SE tools.

Inspect runner settings without programming the target::

   west flash --context -r alif_flash

Programming with ``west flash`` copies ``zephyr.bin`` into the SE tools'
``build/images`` directory and generates their temporary application table of
contents. It requires a configured, connected target. The runner's command-line
registration has been checked; programming and debug sessions have not been
tested on hardware.

An optional CMSIS DFP SVD file can provide debugger metadata. Set
``ALIF_DFP_SVD_DIR`` at CMake configuration time to its ``Debug/SVD`` directory.
The SVD is not required to compile or link an image.

Compatibility boundary
**********************

Existing native Alif board targets remain available. The SDK-derived targets
select ``CONFIG_ALIF_VENDOR`` and use separate clock and pinctrl compatibles
to avoid replacing the native integration. The vendor B1 SoC identifier
``ab1c1f4m51820ph0_vendor`` distinguishes it from the existing native identifier.

The CRC, UART, CAN, USB device, Ethernet PHY and Bluetooth HCI integrations use
the current Zephyr interfaces. The camera pipeline has an internal adapter for
the older Alif endpoint model. Aggregate ISP settings and JPEG input-buffer
controls use ``alif_video_set_control()`` from
``<zephyr/drivers/video/alif-controls.h>``. The JPEG encoder retains the vendor
control-driven input-buffer convention; it is not a native two-queue video
memory-to-memory encoder. The callback-based I2S sync and polling IPM extensions
are supervisor APIs.

Validation status
*****************

All 25 SDK-derived board/core configurations compiled and linked the hello-world
sample. Separate peripheral profiles compiled the devices normally disabled by
the default board configuration. The existing native
``ensemble_e8_dk/ae822fa0e5597ls0/rtss_hp`` hello-world build also passed.
The combined Twister suite built eleven configurations, including the HP and
HE CRC test images, with zero build failures.

The CRC test in ``tests/drivers/alif/crc`` contains standard check vectors and
incremental update boundary cases. Its E7 image has been built but has not been
executed. No hardware functional, electrical, timing, radio, camera, storage or
power measurements have been performed. Application configuration and peripheral
wiring still require validation on the intended board.
