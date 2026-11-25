# Extracted from Android8.1-vendor+boot.zip / boot.img

- `g0335d_candidate.dtb` — one of the DTBs containing G0335D / gree_g0335d strings (offset 12425436 in boot.img)
- `wlan.ko` — Qualcomm Atheros WLAN module
  - vermagic: `3.18.71-perf-g4a0c249ca2c8-01402-g2933054d2488-dirty SMP preempt mod_unload modversions aarch64`
  - Matches the 3.18.71 kernel in this tree
- `kernel_from_boot.img.gz` — kernel binary extracted from boot.img (gzipped)

Other G0335D-matching DTBs were also present in the boot.img (dtb_07, 14, 15, 20-23, 25, 26).
