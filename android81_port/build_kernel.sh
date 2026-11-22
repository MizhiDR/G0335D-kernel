#!/bin/bash
# Rebuild Image.gz for Android 8.1 / LOS15.1 from the DEDICATED 8.1 tree.
# LOCAL BUILD ONLY: no device access, no flashing, no reboot.
# Run: wsl -d Ub2004 -- bash -lc "bash .../android81_port/build_kernel.sh"
set -o pipefail

T=/root/kernel_g0335d_android81_los151
cd "$T" || exit 1

export ARCH=arm64
export CROSS_COMPILE=/root/tc49/bin/aarch64-linux-android-
export PATH=/root/tc49/bin:$PATH
JOBS=$(nproc)
LOG=/root/android81_build.log

echo "=== tree identity ==="
cat "$T/ANDROID81_ONLY.txt" 2>/dev/null || echo "WARNING: not the 8.1 tree marker!"

echo "=== config precheck ==="
grep -nE '^CONFIG_(QFMT_V2|QUOTA_TREE|QUOTA_NETLINK_INTERFACE|EXT4_ENCRYPTION|EXT4_FS_ENCRYPTION|ENCRYPTED_KEYS|QPNP_HAPTIC|ANDROID_BINDER_DEVICES)=' out/.config
printf "FS_ENCRYPTION (must be off): %s\n" "$(grep -m1 '^CONFIG_FS_ENCRYPTION=' out/.config || echo OK/off)"

echo "=== make oldconfig (non-interactive) ==="
yes "" | make O=out oldconfig > /root/android81_oldconfig.log 2>&1
grep -iE 'error' /root/android81_oldconfig.log | head -n 10 || true

echo "=== make Image.gz ==="
make O=out -j"$JOBS" Image.gz > "$LOG" 2>&1
RC=$?
echo "make rc=$RC"
tail -n 25 "$LOG"
grep -iE 'error:|Error [0-9]|undefined reference|No such file' "$LOG" | head -n 30 || true

echo "=== required objects ==="
for o in fs/quota/quota_v2.o fs/quota/quota_tree.o \
         fs/ext4/crypto_policy.o fs/ext4/crypto.o fs/ext4/crypto_key.o \
         fs/ext4/crypto_fname.o drivers/platform/msm/qpnp-haptic.o \
         drivers/staging/android/binder_alloc.o; do
  ls -l "out/$o" 2>&1
done

echo "=== product ==="
ls -l out/arch/arm64/boot/Image.gz 2>&1
md5sum out/arch/arm64/boot/Image.gz 2>&1

if [ "$RC" -ne 0 ]; then
  echo "BUILD FAILED"
  exit "$RC"
fi
echo "BUILD OK -- next: g0335d_debug/build_boot_dbg51_quota.py (use the 124-DTB blob!)"
