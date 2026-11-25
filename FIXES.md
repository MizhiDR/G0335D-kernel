# Known issues & fixes status (G0335D Android 8.1 kernel)

## Done
- [x] `android81_port/build_kernel.sh` — portable, no hardcoded paths.
- [x] Extracted from working boot.img into `extracted/`:
  - `g0335d_candidate.dtb`
  - `wlan.ko` (vermagic matches 3.18.71)
  - `kernel_from_boot.img.gz`
- [x] `android81_port/package_boot.sh` — Image.gz + DTB packaging after a source build.
- [x] `android81_port/make_boot_from_extracted.sh` — builds Image-dtb (and boot.img if mkbootimg + ramdisk present) from the extracted working kernel. No rebuild required.
- [x] **NT36xxx disabled** in `g0335d_defconfig` and `g0335d_defconfig_android81` (`# CONFIG_TOUCHSCREEN_NT36xxx is not set`). No source existed in the tree.

## Verified in tree
- Synaptics DSX / v21, Atmel maxtouch, TFA audio, TUSB320, qcacld-2.0 WLAN source — all present.

## Still open
- [ ] Confirm exact G0335D DTB among the 31 extracted candidates (using `g0335d_candidate.dtb` for now).
- [ ] Defconfig is still a full dump — regenerate with `make savedefconfig` after a clean build if desired.
- [ ] Package `wlan.ko` into vendor or load at runtime.
- [ ] MANIFEST.md5 stale.

## How to get a bootable image quickly
```bash
bash android81_port/make_boot_from_extracted.sh
```
Needs `mkbootimg` and a ramdisk.cpio in `out/boot_from_extracted/` for a full boot.img. Image-dtb is always produced.
