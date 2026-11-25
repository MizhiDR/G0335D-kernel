# Known issues & fixes status (G0335D Android 8.1 kernel)

## Done
- [x] `android81_port/build_kernel.sh` — portable, no hardcoded paths.
- [x] Extracted from working boot.img:
  - `extracted/g0335d_candidate.dtb`
  - `extracted/wlan.ko` (vermagic matches 3.18.71-perf-...-dirty)
  - `extracted/kernel_from_boot.img.gz`
- [x] `android81_port/package_boot.sh` — copies Image.gz + DTB (+ wlan.ko) into out/package and builds Image.gz-dtb.

## Verified in tree (not missing)
- Synaptics DSX / DSX v21 touch drivers exist (`drivers/input/touchscreen/synaptics_dsx*`)
- Atmel maxtouch exists
- TFA audio codecs exist (`sound/soc/codecs/tfa*`)
- TUSB320 exists (`drivers/misc/type-c/ti_i2c_tusb320.c`)
- QPNP haptic is referenced
- qcacld-2.0 WLAN driver source is in `drivers/staging/qcacld-2.0`

## Still open
- [ ] **NT36xxx** — `CONFIG_TOUCHSCREEN_NT36xxx=y` but no driver source in tree. Either add the driver or set `# CONFIG_TOUCHSCREEN_NT36xxx is not set`.
- [ ] Integrate the extracted DTB into the normal build (currently only via package_boot.sh).
- [ ] Confirm which of the 31 DTBs from the stock boot.img is the exact G0335D match (candidate is dtb_05).
- [ ] Defconfig is a full .config dump — regenerate with `make savedefconfig` after a clean build.
- [ ] MANIFEST.md5 is stale.
- [ ] Package wlan.ko into vendor image or load it at runtime.

## Next
1. Build with fixed `build_kernel.sh`.
2. Run `bash android81_port/package_boot.sh` to get Image.gz-dtb.
3. If NT36 touch is required, source the driver; otherwise disable the config.
