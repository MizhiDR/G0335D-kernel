#!/bin/bash
# Create boot.img from extracted kernel + DTB + ramdisk using original cmdline.
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

EXT="$ROOT/extracted"
OUT="$ROOT/out/boot_from_extracted"
mkdir -p "$OUT"

KERNEL_GZ="$EXT/kernel_from_boot.img.gz"
DTB="$EXT/g0335d_candidate.dtb"
RAMDISK="${RAMDISK:-$OUT/ramdisk.cpio}"

if [ ! -f "$KERNEL_GZ" ] || [ ! -f "$DTB" ]; then
  echo "Missing extracted kernel or DTB"
  exit 1
fi

gunzip -c "$KERNEL_GZ" > "$OUT/kernel"
cat "$OUT/kernel" "$DTB" > "$OUT/Image-dtb"

CMDLINE='console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x237 ehci-hcd.park=3 lpm_levels.sleep_disabled=1 cma=32M@0-0xffffffff androidboot.selinux=permissive'

echo "Image-dtb ready: $OUT/Image-dtb"
ls -lh "$OUT/Image-dtb"

if ! command -v mkbootimg >/dev/null 2>&1; then
  echo "mkbootimg not in PATH. Install android-tools or use the Image-dtb with your packer."
  exit 0
fi

if [ ! -f "$RAMDISK" ]; then
  echo "No ramdisk at $RAMDISK"
  echo "Place the original ramdisk.cpio there and re-run."
  exit 0
fi

mkbootimg \
  --kernel "$OUT/Image-dtb" \
  --ramdisk "$RAMDISK" \
  --cmdline "$CMDLINE" \
  --base 0x80000000 \
  --pagesize 4096 \
  --output "$OUT/boot.img"

echo "boot.img -> $OUT/boot.img"
ls -lh "$OUT/boot.img"
