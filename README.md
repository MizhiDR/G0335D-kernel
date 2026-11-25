# GREE G0335D Kernel (Android 8.1 / LOS 15.1)

Linux 3.18.71 kernel source for GREE G0335D / G3S (Snapdragon 820/821 msm8996),
the Android 8.1 (LineageOS 15.1) porting mainline.

Fork with fixes: `kukirin-ui/android_kernel_G0335D_msm8996Pro`

---

## Status (Nov 2025)

### Done
- `android81_port/build_kernel.sh` — portable (no hardcoded `/root` or WSL paths). Auto-detects tree, generates defconfig if missing.
- Extracted from a working `Android8.1-vendor+boot.zip` / `boot.img` into `extracted/`:
  - `g0335d_candidate.dtb` — one of the DTBs containing G0335D strings
  - `wlan.ko` — vermagic `3.18.71-perf-g4a0c249ca2c8-01402-g2933054d2488-dirty SMP preempt mod_unload modversions aarch64`
  - `kernel_from_boot.img.gz` — kernel binary that previously booted
  - `LOAD_WLAN.txt` — notes if the module fails to load
- `android81_port/package_boot.sh` — packages Image.gz + DTB after a source build
- `android81_port/make_boot_from_extracted.sh` — builds Image-dtb from extracted kernel (no rebuild)
- `android81_port/mkboot_extracted.sh` — full `mkbootimg` with the original cmdline from the working boot.img
- Basic **NT36xxx** touchscreen driver added:
  - `drivers/input/touchscreen/nt36xxx/nt36xxx.c`
  - Kconfig + Makefile wired
  - `CONFIG_TOUCHSCREEN_NT36xxx=y` re-enabled
- `FIXES.md` — short checklist

### Drivers present in tree
- Synaptics DSX / DSX v21
- Atmel maxtouch
- TFA audio codecs
- TUSB320 (`drivers/misc/type-c/ti_i2c_tusb320.c`)
- qcacld-2.0 WLAN source (`drivers/staging/qcacld-2.0`)

### Still open / caveats
- NT36xxx driver is a **basic** implementation. Protocol offsets, GPIOs and panel specifics may need tuning against the real G0335D DTB. Touch is not guaranteed on first boot.
- DTB is a **candidate** (`g0335d_candidate.dtb`). The original boot.img contained 31 DTBs; this one has G0335D-related strings but may not be the exact match.
- `wlan.ko` will only load if vermagic matches the running kernel. Otherwise rebuild qcacld-2.0 or use the matching prebuilt module.
- Defconfig is still a large full `.config` dump. Can be cleaned later with `make savedefconfig`.
- MANIFEST.md5 is stale.

---

## Quick test (recommended first — no rebuild)

Uses the kernel that already booted on the device.

```bash
# optional: place original ramdisk.cpio at out/boot_from_extracted/ramdisk.cpio
bash android81_port/mkboot_extracted.sh
```

Produces:
- `out/boot_from_extracted/Image-dtb`
- `out/boot_from_extracted/boot.img` (if `mkbootimg` and ramdisk are available)

Original cmdline used:
```
console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x237 ehci-hcd.park=3 lpm_levels.sleep_disabled=1 cma=32M@0-0xffffffff androidboot.selinux=permissive
```

---

## Source build

```bash
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-android-

bash android81_port/build_kernel.sh
# or manually:
make O=out g0335d_defconfig
make O=out -j$(nproc) Image.gz

bash android81_port/package_boot.sh
```

---

## Layout

- `android81_port/` — Android 8.1 patches, build and packaging scripts
- `extracted/` — DTB, wlan.ko, kernel taken from a working boot.img
- `arch/arm64/configs/g0335d_defconfig` — device defconfig
- `drivers/input/touchscreen/nt36xxx/` — added basic NT36xxx driver
- `FIXES.md` — concise status list

---

## Credits

Fixes and NT36xxx driver skeleton by Grok (xAI).  
Base tree from the G0335D Android 8.1 porting effort.
