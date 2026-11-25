# Known issues & fixes status (G0335D Android 8.1 kernel)

## Done
- [x] `android81_port/build_kernel.sh` — removed hardcoded `/root` and WSL paths. Now portable, auto-detects tree root, generates defconfig if missing.

## Verified OK
- [x] `android81_port/patches/10_capture_topo_fix.patch` — content is valid (GREE capture topology fallback to NULL_COPP). Not corrupt in current tree.

## Still open (need source or device testing)
- [ ] **GREE drivers missing** — defconfig enables touch (NT36xxx / Synaptics DSX), vibrator, TFA audio, TUSB320 but driver source not present in tree. Touch/vibrator/audio/USB-C will not work until drivers are added or configs disabled.
- [ ] **No G0335D device tree** — only Qualcomm reference DTBs are built. Stock DTB blob / board file missing.
- [ ] **WiFi** — `wlan` is modular but module is never built/packaged. Stock `wlan.ko` will not load (vermagic mismatch on 3.18.71). Need to either build the module or set `=y`.
- [ ] **Defconfig is full .config dump** — `arch/arm64/configs/g0335d_defconfig` and the android81 copy are ~120k lines. Should be regenerated with `make savedefconfig` after a clean build.
- [ ] MANIFEST.md5 is stale.
- [ ] `-Werror` may cause build failures on newer toolchains.
- [ ] Build artifacts should not be committed.

## Next recommended steps
1. Try a clean build with the fixed `build_kernel.sh`.
2. If build fails on missing drivers, either add the driver sources or comment out the corresponding CONFIG_* in defconfig.
3. For WiFi: either integrate a matching wlan driver or force built-in.
4. Obtain and add the real G0335D DTB.
