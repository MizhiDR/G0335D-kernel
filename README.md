# GREE G0335D Kernel (Android 8.1 / LOS 15.1)

Linux 3.18.71 kernel source for GREE G0335D / G3S (Snapdragon 820/821 msm8996),
the Android 8.1 (LineageOS 15.1) porting mainline.

- `android81_port/`  — Android 8.1 port patch set (01~09) + all-in-one + verify scripts
- `g0335d_port/`     — GREE G0335D specific patches
- `arch/arm64/configs/g0335d_defconfig` — device defconfig
- `ANDROID81_ONLY.txt` — tree scope note

Build: `ARCH=arm64 CROSS_COMPILE=<aarch64-linux-android-> make O=out g0335d_defconfig && make O=out -jN Image.gz`
