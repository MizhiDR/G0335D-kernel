# Known issues & fixes status (G0335D Android 8.1 kernel)

## Done
- [x] Portable `android81_port/build_kernel.sh`
- [x] Extracted working pieces in `extracted/` (DTB, wlan.ko, kernel)
- [x] `android81_port/package_boot.sh`
- [x] `android81_port/make_boot_from_extracted.sh`
- [x] **`android81_port/mkboot_extracted.sh`** — ready mkbootimg command with original cmdline from the working boot.img
- [x] Basic NT36xxx driver added and config enabled
- [x] `extracted/LOAD_WLAN.txt` — vermagic and load notes

## Quick test path (no rebuild)
```bash
# put original ramdisk.cpio in out/boot_from_extracted/ if you have it
bash android81_port/mkboot_extracted.sh
```
Produces `out/boot_from_extracted/boot.img` (or at least Image-dtb).

## Still open
- NT36xxx driver is basic — touch may need protocol/GPIO tweaks for this panel.
- Confirm exact DTB among the 31 candidates.
- wlan.ko vermagic must match the running kernel (or rebuild qcacld-2.0).
