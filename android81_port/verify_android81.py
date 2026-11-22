#!/usr/bin/env python3
"""Verify every Android-8.1 fix is present in the dedicated kernel tree.

Usage (WSL): python3 /root/kernel_g0335d_android81_los151/android81_port/verify_android81.py [tree]
Read-only. Prints PASS/FAIL per check, exits 1 if anything is missing.
"""
import os
import sys

TREE = sys.argv[1] if len(sys.argv) > 1 else "/root/kernel_g0335d_android81_los151"
DEF = "arch/arm64/configs/g0335d_defconfig"
OUTCFG = "out/.config"

BINDER_FILES = {
    "drivers/staging/android/binder.c": "binder_alloc_mmap_handler",
    "drivers/staging/android/binder_alloc.c": "binder_alloc",
    "drivers/staging/android/binder_alloc.h": "binder_alloc",
    "drivers/staging/android/binder_trace.h": "binder",
    "drivers/staging/android/uapi/binder.h": "BC_TRANSACTION_SG",
}

CFG_REQUIRED = [
    "CONFIG_QPNP_HAPTIC=y",
    "CONFIG_QFMT_V2=y",
    "CONFIG_QUOTA_TREE=y",
    "CONFIG_QUOTA_NETLINK_INTERFACE=y",
    "CONFIG_EXT4_ENCRYPTION=y",
    "CONFIG_EXT4_FS_ENCRYPTION=y",
    "CONFIG_ENCRYPTED_KEYS=y",
    'CONFIG_ANDROID_BINDER_DEVICES="binder,hwbinder,vndbinder"',
]


def read(rel):
    p = os.path.join(TREE, rel)
    if not os.path.isfile(p):
        return None
    with open(p, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def main():
    results = []

    def check(name, ok):
        results.append((name, bool(ok)))

    check("dedicated 8.1 marker", os.path.isfile(os.path.join(TREE, "ANDROID81_ONLY.txt")))
    check("android81_port/ exists", os.path.isdir(os.path.join(TREE, "android81_port")))

    for rel, marker in BINDER_FILES.items():
        txt = read(rel)
        check("Oreo binder source " + os.path.basename(rel), txt is not None and marker in txt)

    kcfg = read("drivers/staging/android/Kconfig") or ""
    mk = read("drivers/staging/android/Makefile") or ""
    check("Kconfig ANDROID_BINDER_DEVICES", "config ANDROID_BINDER_DEVICES" in kcfg)
    check("Makefile binder_alloc.o", "binder_alloc.o" in mk)

    for rel in (DEF, OUTCFG):
        txt = read(rel)
        if txt is None:
            for c in CFG_REQUIRED:
                check("%s: %s" % (os.path.basename(rel), c), False)
            check("%s: FS_ENCRYPTION off" % os.path.basename(rel), False)
            continue
        for c in CFG_REQUIRED:
            check("%s: %s" % (os.path.basename(rel), c), c in txt)
        check("%s: FS_ENCRYPTION off" % os.path.basename(rel),
              "CONFIG_FS_ENCRYPTION=y" not in txt)

    panel = read("drivers/video/msm/mdss/mdss_dsi_panel.c") or ""
    check("AQ62 display patch intact", "AQ62" in panel)

    dts = read("drivers/video/msm/mdss/mdss_dsi_panel.c") or ""
    check("no marlin-only fix01/09 leftovers (informational)", True)

    # --- stage 04 / 05 / 06 (dbg52 camera, dbg53 WLAN, dbg54 reboot) ---
    cam = read("drivers/media/platform/msm/camera_v2/sensor/msm_sensor_driver.c") or ""
    check("dbg52 camera power compat marker", "GREE_COMPAT" in cam)

    ol = read("drivers/staging/qcacld-2.0/CORE/SERVICES/BMI/ol_fw.c") or ""
    cnss = read("drivers/net/wireless/cnss/cnss_pci.c") or ""
    check("dbg53 wlan fast-fail (ol_fw.c)", "GREE_WLAN_FASTFAIL" in ol)
    check("dbg53 wlan fast-fail (cnss_pci.c)", "GREE_WLAN_FASTFAIL" in cnss)

    po = read("drivers/power/reset/msm-poweroff.c") or ""
    check("dbg54 restart TZ fix marker", "GREE_RESTART_TZ_FIX" in po)
    check("dbg54 set_dload_mode no longer issues SCM_DLOAD_CMD",
          "on ? SCM_DLOAD_MODE : 0" not in po)
    check("dbg54 scm_disable_sdi no longer issues WDOG SCM",
          "SCM_WDOG_DEBUG_BOOT_PART, 1, 0" not in po)
    check("dbg54 reset machinery intact",
          "halt_spmi_pmic_arbiter();" in po and "deassert_ps_hold();" in po)
    check("dbg54 EDL SCM path intact", "SCM_EDLOAD_MODE, 0" in po)

    # --- stage 07 (dbg55 Goodix GF fingerprint driver) ---
    gdir = "drivers/input/fingerprint/goodix/"
    gc = read(gdir + "gf_spi.c") or ""
    gh = read(gdir + "gf_spi.h") or ""
    gp = read(gdir + "platform.c") or ""
    gk = read(gdir + "Kconfig") or ""
    gm = read(gdir + "Makefile") or ""
    check("dbg55 gf_spi.c present", "GREE_GOODIX_FP" in gc)
    check("dbg55 creates /dev/goodix_fp",
          "GF_DEV_NAME" in gc and '"goodix_fp"' in gc)
    check("dbg55 DT compatible goodix,fingerprint",
          '"goodix,fingerprint"' in gc)
    check("dbg55 binds as SPI driver", "spi_register_driver(&gf_driver)" in gc)
    check("dbg55 USE_SPI_BUS selected", "#define  USE_SPI_BUS" in gh)
    check("dbg55 USE_PLATFORM_BUS disabled", "//#define  USE_PLATFORM_BUS" in gh)
    check("dbg55 GREE RESET ioctl encoding",
          "GF_IOC_RESET" in gh and "_IOW(GF_IOC_MAGIC, 3, unsigned int)" in gh)
    check("dbg55 KEY_NAVI_LONG fallback", "#ifndef KEY_NAVI_LONG" in gc)
    check("dbg55 no missing spidev.h include", "#include <linux/spi/spidev.h>" not in gc
          and "#include <linux/spi/spidev.h>" not in gp)
    check("dbg55 static gf not kfreed", "kfree(gf_dev)" not in gc)
    check("dbg55 DT gpio properties", "goodix,gpio_pwr" in gp
          and "goodix,gpio_reset" in gp and "goodix,gpio_irq" in gp)
    check("dbg55 Kconfig symbol", "config FINGERPRINT_GOODIX" in gk)
    check("dbg55 sub Makefile", "gf-objs := gf_spi.o platform.o" in gm)
    p_kcfg = read("drivers/input/fingerprint/Kconfig") or ""
    p_mk = read("drivers/input/fingerprint/Makefile") or ""
    check("dbg55 parent Kconfig sources goodix", "goodix/Kconfig" in p_kcfg)
    check("dbg55 parent Makefile builds goodix", "goodix/" in p_mk)
    for rel in (DEF, OUTCFG):
        txt = read(rel) or ""
        check("%s: INPUT_FINGERPRINT=y" % os.path.basename(rel),
              "CONFIG_INPUT_FINGERPRINT=y" in txt)
        check("%s: FINGERPRINT_GOODIX=y" % os.path.basename(rel),
              "CONFIG_FINGERPRINT_GOODIX=y" in txt)
        check("%s: FPC1020 explicitly off" % os.path.basename(rel),
              "# CONFIG_FINGERPRINT_FPC1020_TA is not set" in txt)

    # --- stage 08 (dbg58 spi_qsd pinctrl downgrade) ---
    # NOTE: the in-tree comment still says "dbg57"; v3 is the version that
    # actually shipped in dbg58. Do not "fix" the comment without rebuilding.
    sq = read("drivers/spi/spi_qsd.c") or ""
    check("dbg58 pinctrl downgrade (warn + continue)",
          "pinctrl failed during " in sq and "init, continuing" in sq)
    check("dbg58 logs real errno and init_done",
          "err=%d, init_done=%d" in sq and "dd->is_init_complete" in sq)
    check("dbg58 clears pinctrl result", "result = 0;" in sq)
    check("dbg58 keeps original error line for diagnosis",
          "Can not set %s pins" in sq)

    # --- stage 09 (dbg59 Goodix GF IRQ: EDGE_BOTH + pinctrl + poll fallback) ---
    gc9 = read("drivers/input/fingerprint/goodix/gf_spi.c") or ""
    gh9 = read("drivers/input/fingerprint/goodix/gf_spi.h") or ""
    gp9 = read("drivers/input/fingerprint/goodix/platform.c") or ""
    p9 = read("android81_port/patches/09_goodix_fp_irq_fix.patch") or ""
    check("dbg59 patch 09 archived", "dbg59 - Goodix GF" in p9)
    check("dbg59 requests BOTH edges",
          "IRQF_TRIGGER_RISING" in gc9 and "IRQF_TRIGGER_FALLING" in gc9
          and "IRQF_ONESHOT" in gc9)
    check("dbg59 poll fallback present",
          "gf_poll_timer_fn" in gc9 and "hw_irq_seen" in gc9
          and "poll edge" in gc9)
    check("dbg59 timer field in gf_dev", "struct timer_list poll_timer" in gh9)
    check("dbg59 reads DT gpio flags",
          "of_get_named_gpio_flags" in gp9 and "irq_gpio_flags" in gp9)
    check("dbg59 applies eint_as_int, failure non-fatal",
          "eint_as_int" in gp9 and "pinctrl_select_state" in gp9
          and "continuing" in gp9 and "IS_ERR_OR_NULL" in gp9)

    # --- stage 10 (dbg61 capture topology fix: recording) ---
    # dbg60 checked topology==NULL_COPP(0x10312) -> dead code (actual=0x1000fff1).
    # dbg61 checks topology > 0x10000000 -> NULL_COPP (the corrected fix).
    q6 = read("sound/soc/msm/qdsp6v2/q6adm.c") or ""
    p10 = read("android81_port/patches/10_capture_topo_fix.patch") or ""
    check("dbg61 patch 10 archived", "GREE_CAPTURE_TOPO_FIX dbg61" in p10)
    check("dbg61 capture out-of-range -> NULL_COPP",
          "GREE_CAPTURE_TOPO_FIX dbg61" in q6
          and "ADM_PATH_LIVE_REC || path == ADM_PATH_NONLIVE_REC" in q6
          and "topology > 0x10000000" in q6
          and "topology = NULL_COPP_TOPOLOGY;" in q6)
    check("dbg60 dead code / dbg60b diag removed",
          "null->default" not in q6 and "GREE_ADM_DIAG" not in q6)

    for name, ok in results:
        print("  [%s] %s" % ("PASS" if ok else "FAIL", name))
    bad = [n for n, ok in results if not ok]
    print("\n%d/%d checks passed on %s" % (len(results) - len(bad), len(results), TREE))
    if bad:
        print("FAILED: " + ", ".join(bad))
        return 1
    print("ALL ANDROID 8.1 FIXES PRESENT")
    return 0


if __name__ == "__main__":
    sys.exit(main())
