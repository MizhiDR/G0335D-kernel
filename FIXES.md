# Known issues & fixes status (G0335D Android 8.1 kernel)

## Done
- [x] `android81_port/build_kernel.sh` — removed hardcoded `/root` and WSL paths. Now portable, auto-detects tree root, generates defconfig if missing.
- [x] Extracted from working `Android8.1-vendor+boot.zip` / `boot.img`:
  - `extracted/g0335d_candidate.dtb` (one of the G0335D DTBs)
  - `extracted/wlan.ko` (vermagic matches 3.18.71-perf-...-dirty)
  - `extracted/kernel_from_boot.img.gz`
  - See `extracted/README.md`

## Verified OK
- [x] `android81_port/patches/10_capture_topo_fix.patch` — content is valid (GREE capture topology fallback to NULL_COPP).
- [x] `wlan.ko` vermagic = `3.18.71-perf-g4a0c249ca2c8-01402-g2933054d2488-dirty SMP preempt mod_unload modversions aarch64` — compatible with this tree.

## Still open (need source or device testing)
- [ ] **GREE drivers missing** — defconfig enables touch (NT36xxx / Synaptics DSX), vibrator, TFA audio, TUSB320 but driver source not present in tree.
- [ ] **Device tree** — stock DTB extracted (`extracted/g0335d_candidate.dtb`); still need to integrate into build or confirm which of the 31 DTBs is the exact match.
- [ ] **WiFi** — matching `wlan.ko` is now in `extracted/`; still need to package it into the boot/vendor image or build from source.
- [ ] **Defconfig is full .config dump** — should be regenerated with `make savedefconfig` after a clean build.
- [ ] MANIFEST.md5 is stale.
- [ ] `-Werror` may cause build failures on newer toolchains.

## Next recommended steps
1. Try a clean build with the fixed `build_kernel.sh`.
2. Package `extracted/wlan.ko` into the vendor image or load it.
3. Append / use `extracted/g0335d_candidate.dtb` when building boot.img.
4. If touch/audio fail, add missing driver sources or disable the corresponding CONFIG_*.
