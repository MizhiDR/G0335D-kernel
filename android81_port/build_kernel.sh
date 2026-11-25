#!/bin/bash
# Rebuild Image.gz for Android 8.1 / LOS15.1 from the DEDICATED 8.1 tree.
# Portable version: no hardcoded /root or WSL paths.
# Usage: from kernel root, run: bash android81_port/build_kernel.sh
# Optional env: CROSS_COMPILE, OUT_DIR, JOBS
set -o pipefail

# Resolve kernel tree root (parent of android81_port/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
T="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$T" || exit 1

export ARCH=arm64
# Default CROSS_COMPILE; override if needed
: "${CROSS_COMPILE:=aarch64-linux-android-}"
export CROSS_COMPILE

OUT_DIR="${OUT_DIR:-out}"
JOBS="${JOBS:-$(nproc)}"
LOG="${LOG:-android81_build.log}"

echo "=== tree identity ==="
echo "Kernel tree: $T"
cat "$T/ANDROID81_ONLY.txt" 2>/dev/null || echo "WARNING: not the 8.1 tree marker!"

# Ensure out dir and config exist before grepping
if [ ! -f "$OUT_DIR/.config" ]; then
  echo "=== no .config found, generating from g0335d_defconfig ==="
  make O="$OUT_DIR" g0335d_defconfig || exit 1
fi

echo "=== config precheck ==="
grep -nE '^CONFIG_(QFMT_V2|QUOTA_TREE|QUOTA_NETLINK_INTERFACE|EXT4_ENCRYPTION|EXT4_FS_ENCRYPTION|ENCRYPTED_KEYS|QPNP_HAPTIC|ANDROID_BINDER_DEVICES)=' "$OUT_DIR/.config" || true
printf "FS_ENCRYPTION (must be off): %s\n" "$(grep -m1 '^CONFIG_FS_ENCRYPTION=' "$OUT_DIR/.config" || echo OK/off)"

echo "=== make oldconfig (non-interactive) ==="
yes "" | make O="$OUT_DIR" oldconfig > android81_oldconfig.log 2>&1
grep -iE 'error' android81_oldconfig.log | head -n 10 || true

echo "=== make Image.gz ==="
make O="$OUT_DIR" -j"$JOBS" Image.gz > "$LOG" 2>&1
RC=$?
echo "make rc=$RC"
tail -n 25 "$LOG"
grep -iE 'error:|Error [0-9]|undefined reference|No such file' "$LOG" | head -n 30 || true

echo "=== required objects ==="
for o in fs/quota/quota_v2.o fs/quota/quota_tree.o \
         fs/ext4/crypto_policy.o fs/ext4/crypto.o fs/ext4/crypto_key.o \
         fs/ext4/crypto_fname.o drivers/platform/msm/qpnp-haptic.o \
         drivers/staging/android/binder_alloc.o; do
  ls -l "$OUT_DIR/$o" 2>&1
done

echo "=== product ==="
ls -l "$OUT_DIR/arch/arm64/boot/Image.gz" 2>&1
md5sum "$OUT_DIR/arch/arm64/boot/Image.gz" 2>&1

if [ "$RC" -ne 0 ]; then
  echo "BUILD FAILED"
  exit "$RC"
fi
echo "BUILD OK"
