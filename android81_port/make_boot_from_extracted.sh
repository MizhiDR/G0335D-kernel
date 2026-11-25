#!/bin/bash
# Build a boot.img from the extracted working kernel + DTB + ramdisk.
# Does not require a full kernel rebuild.
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

EXT="$ROOT/extracted"
OUT="$ROOT/out/boot_from_extracted"
mkdir -p "$OUT"

KERNEL_GZ="$EXT/kernel_from_boot.img.gz"
DTB="$EXT/g0335d_candidate.dtb"
RAMDISK="$OUT/ramdisk.cpio"

# Prefer extracted ramdisk if we saved one; otherwise note it
if [ ! -f "$KERNEL_GZ" ]; then
  echo "Missing $KERNEL_GZ"
  exit 1
fi
if [ ! -f "$DTB" ]; then
  echo "Missing $DTB"
  exit 1
fi

gunzip -c "$KERNEL_GZ" > "$OUT/kernel"
cat "$OUT/kernel" "$DTB" > "$OUT/Image-dtb"

echo "Created:"
ls -lh "$OUT/kernel" "$OUT/Image-dtb" "$DTB"

# If mkbootimg is available and we have a ramdisk, pack it
if command -v mkbootimg >/dev/null 2>&1 && [ -f "$RAMDISK" ]; then
  mkbootimg --kernel "$OUT/Image-dtb" --ramdisk "$RAMDISK" \
    --cmdline "console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x237 ehci-hcd.park=3 lpm_levels.sleep_disabled=1 cma=32M@0-0xffffffff" \
    --base 0x80000000 --pagesize 4096 --output "$OUT/boot.img"
  echo "boot.img -> $OUT/boot.img"
else
  echo "mkbootimg or ramdisk not present."
  echo "Copy the original ramdisk.cpio to $RAMDISK and re-run, or use an existing boot.img tool."
  echo "Image-dtb is ready at $OUT/Image-dtb"
fi
