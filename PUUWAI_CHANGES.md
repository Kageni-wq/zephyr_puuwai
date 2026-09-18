# Puuwai changes to this Zephyr fork

Every change below used to be a build-local patch in the Puuwai hardware module:
a Python script that checked a pinned source hash, rewrote the file into the
build tree, and swapped it into the Zephyr target. They are now ordinary changes
to this fork, so nothing verifies the Zephyr revision at configure time.

Changes that need code from the hardware module are compiled only when their
option in `Kconfig.puuwai` is set; the module sets those options itself. Every
other change is unconditional, because it is a correction rather than a hook.

| File | Change | Gate |
| --- | --- | --- |
| `drivers/timer/stm32_lptim_timer.c` | Wider asynchronous-write guard, kernel-tick remainder carried across announcements, and `puuwai_lptim_prepare_stop()` for PM entry | none |
| `drivers/clock_control/clock_stm32_ll_h7.c` | M4 uses its own RCC allocation view instead of CPU1's (RM0399 9.5.10) | none |
| `subsys/fs/fat_fs.c` | A failed `f_close`/`f_closedir` keeps its FIL/DIR so `fs_close()` can retry | none |
| `drivers/mipi_dsi/dsi_stm32.c` | DSI command mode for panels without `MIPI_DSI_MODE_VIDEO`; LP or HS per message from `MIPI_DSI_MSG_USE_LPM` | none |
| `drivers/mipi_dsi/Kconfig.stm32` | The host no longer requires LTDC, since command mode needs no framebuffer | none |
| `drivers/input/input_cst8xx.c` | IRQ and poll work are drained before suspend; reset failures propagate | none |
| `drivers/audio/dmic_stm32_dfsdm.c` | `filter0-sync` honored, DMA callback selects by hardware event and reports faults, parent runtime-PM references released | none |
| `drivers/i2s/i2s_stm32_sai.c` | DMA transfer width follows the runtime PCM word size instead of the devicetree default; DROP stops hardware before releasing the slab; parent/child PM | none |
| `subsys/sd/sd_ops.c` | `DISK_IOCTL_CTRL_DEINIT` stops on a busy card and only adopts bus settings that were applied | none |
| `drivers/disk/sdmmc_subsys.c` | DEINIT marks the card uninitialized only on success; reference-counted deinit for the power owner | `PUUWAI_SD_HOST_HOOKS` (added functions only) |
| `drivers/sdhc/sdhc_stm32.c` | Requests are admitted only while the SD host owns power and clocks | `PUUWAI_SD_HOST_HOOKS` |
| `drivers/display/display_gc9x01x.c` | Reset and blanking failures propagate; device PM delegates to the panel owner | `PUUWAI_PANEL_HOOKS` (delegation only) |
| `drivers/wifi/nxp/src/nxp_wifi_drv.c` | WLAN initialization deferred to the platform bridge that owns the shared IW612 rails | `PUUWAI_IW612_COLD_LIFECYCLE` |
| `drivers/bluetooth/hci/hci_nxp_setup.c` | Shared PWDN pin left to its board owner; checked H4 shutdown and cold-start hooks | none (board conditioned) |

With every option off this tree behaves as upstream, except for the corrections
listed as ungated.

Two more build-local patches remain in the hardware module, because they modify
the NXP HAL module rather than Zephyr: `wlcmgr/wlan.c` and
`sdio_nxp_abs/mlan_sdio.c`. They still check pinned source hashes.

## Rebasing onto upstream

`git log --oneline upstream/main..` on this branch shows these changes as
commits. When a conflict appears, the question to answer is whether upstream
fixed the same problem; if it did, drop the change instead of merging it.
