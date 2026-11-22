# Android 8.1 / LineageOS 15.1 专用内核源码（GREE G0335D）

本目录与 WSL `/root/kernel_g0335d_android81_los151/` 是 **Android 8.1 专用** 内核树及其固化补丁集。
与 LOS 14.1 / cm-14.1 世代的源码**严格隔离**：本树的补丁不得反向打到 14.1 树上，14.1 的历史 `out*`
也不进入本备份。

## 1. 源码树身份对照（务必分清）

| 路径 | 版本 / 世代 | 角色 |
|---|---|---|
| `/root/kernel_g0335d_android81_los151` | GREE 3.18.71 基线 + 8.1 修复 | **Android 8.1 专用备份树**，与主线补丁逐字节同步 |
| `/root/kernel_g0335d` | GREE 3.18.71 原工作树 | **实际编译树**（`O=out`，dbg51~58 均由此产出）；每次改源码两棵树同步打补丁 |
| `/root/kernel_msm8996_oreo` | marlin 3.18.52 `android-msm-marlin-3.18-oreo-r6` | 早期主线 fix01~14，已收官；只取已验证的结论，不整树照搬 |
| `/root/msm8996_cm14` / `g3s_kernel` / `g0245d` | 3.18.31 cm-14.1 | 仅参考 |
| `/root/flyme/sys` | 魅族 7.1.1 system | **禁止用于推 GREE 硬件结论**；GREE 官方素材只用 `D:\G0335D含线刷\system.img` 与 `D:\G0335D_MIUI_PORT\` |

**备份方式**：`rsync -a` 从 `/root/kernel_g0335d` 复制，显式排除 `out49` / `out_gold` / `out_pristine` /
`out_stock` / `out_tusb`（14.1 与早期实验构建产物），保留 `out/`。
备份 9.5G（源码 6.0G + out 3.6G），2026-11-15 17:29 完成。树根有 `ANDROID81_ONLY.txt` 标记。

## 2. 已固化的 8.1 修复（01~09）

| # | 修复 | 现象 / 根因 | 改动 |
|---|---|---|---|
| 1 | **Oreo binder backport** | 3.18.71 是 N 世代 binder，只有一个 `/dev/binder`；8.1 HIDL 需要 `binder,hwbinder,vndbinder` | 从 marlin 3.18.52 复制 `binder.c`、`binder_alloc.c/.h`、`binder_trace.h`、`uapi/binder.h`；Kconfig 增 `ANDROID_BINDER_DEVICES`，Makefile 增 `binder_alloc.o`；defconfig 与 `out/.config` 增 `CONFIG_ANDROID_BINDER_DEVICES="binder,hwbinder,vndbinder"` |
| 2 | **CONFIG_QPNP_HAPTIC=y** | 未开 → 无 `/sys/class/timed_output/vibrator` → vibrator HAL `-19` → `StartVibratorService` 无限等 → 卡 LOS 动画（dbg49/50） | defconfig + `out/.config` 单行 |
| 3 | **ext4 quota / FBE** | `/data` ext4 带 quota feature，但缺 `QFMT_V2`/`QUOTA_TREE` → `ext4_enable_quotas=-ESRCH` → `mount_all returned 2` → CryptKeeper「Encryption unsuccessful」（dbg50） | `QFMT_V2` / `QUOTA_TREE` / `QUOTA_NETLINK_INTERFACE` / `EXT4_ENCRYPTION` / `EXT4_FS_ENCRYPTION` / `ENCRYPTED_KEYS` 全 `=y`；**`CONFIG_FS_ENCRYPTION` 保持关闭**（与 ext4 自带 crypto 冲突） |
| 4 | **相机 power-setting 兜底 (dbg52)** | GREE 7.1 sensor 库 `power_setting_array.size==0`，8.1 `msm_sensor_driver.c` 直接跳过上电 → `id_reg=0x0` | `msm_sensor_driver.c` 加 `GREE_COMPAT` 诊断与最小上电序列；**注：官方 7.1 sensor 库与 8.1 框架 ABI 不匹配，vendor 侧相机适配已整体回滚，仅内核兜底保留** |
| 5 | **WLAN 固件 fast-fail (dbg53)** | `evicted30.bin` / `bdwlan30.b00` 缺失，`request_firmware()` 各等 60s ueventd 超时 → `insmod wlan.ko` 阻塞 init 121s | `ol_fw.c`（board-data 变体）与 `cnss_pci.c`（evicted）改用 `request_firmware_direct()`；默认 `bdwlan30.bin` 回退与 EDL 无关逻辑不动 |
| 6 | **重启 TZ SCM 抑制 (dbg54)** | 系统态 reboot（电源菜单 / `adb reboot`）经 `msm_restart_prepare()` + `do_msm_restart()` 发 `SCM_DLOAD_CMD` / `SCM_WDOG_DEBUG_BOOT_PART`，毒化 TZ → 下次开机约 9s 后 PS_HOLD 被拉低，只能长按 10s 硬断电或 TWRP→System | `msm-poweroff.c`：`set_dload_mode()` 只写 IMEM 魔数不再发 SCM；`scm_disable_sdi()` 变空操作 + 日志。**IMEM 标志、`dload_mode_enabled`、`halt_spmi_pmic_arbiter()`、`deassert_ps_hold()`、EDL 专用 SCM 全部保留**，重启动作本身不变 |
| 7 | **Goodix GF 指纹驱动 (dbg55)** | GREE 硬件是 Goodix GF + QSEE TA，官方 3.18.71 内核内建 `gf_spi.c` 创建 `/dev/goodix_fp`；本 CM-14.1 树把驱动裁掉了（defconfig 里 `CONFIG_GOODIX_FP=y` 是无效孤立符号），设备无节点，LOS15.1 的 QFP/HIDL 栈与硬件不匹配只能止血 | 从 `MoKee/android_kernel_zuk_msm8996 @ mkm`（同为 msm8996/QSEE）移植 `drivers/input/fingerprint/goodix/` 5 文件；改 `USE_PLATFORM_BUS`→`USE_SPI_BUS`、对齐 `GF_IOC_RESET` 编码为 `_IOW('G',3,u32)`、补 `KEY_NAVI_LONG` fallback、去掉 3.18 不存在的 `<linux/spi/spidev.h>`、修 3 处上游 bug；`CONFIG_INPUT_FINGERPRINT=y` + `CONFIG_FINGERPRINT_GOODIX=y`，FPC1020 显式关闭 |
| 8 | **spi_qsd pinctrl 失败降级 (dbg58)** | `pinctrl_select_state("spi_default")` 在 `get_local_resources()` 阶段失败（实测 `err=-22`、`is_init_complete=1`，伴随 `no irq domain found for /soc/pinctrl@01010000`，DTB 与 CM pinctrl 驱动代际不匹配）→ 原代码 `goto error` 让 `dd->suspended` 保持 1 → `msm_spi_setup()` 返回 `-EBUSY` → `of_register_spi_devices()` 静默丢弃**所有** DT 子节点，`goodix_touchfinger@0` 从不注册、`gf_probe` 从不执行 | `drivers/spi/spi_qsd.c` 的 `msm_spi_request_gpios()`：pinctrl 失败**无条件**改为 `dev_warn` 打印真实 errno + `result = 0` 后继续。bootloader/DT 已完成管脚复用，放行 `spi_setup()` 与子设备注册。dbg58 生效后 `/dev/goodix_fp`、`spi5.0`、`goodix_fp` input 设备全部出现 |
| 9 | **Goodix GF 指纹中断 (dbg59)** | `/proc/interrupts` 的 `479: 0 0 0 msmgpio 121 gf` 恒为 0，HAL 停在 `Waiting Finger Down`。DT `goodix,gpio_irq=<0x36 0x79 0x01>` 第三格是 active-low 标志，驱动用 `of_get_named_gpio()` 丢掉 flags 且硬编码 `IRQF_TRIGGER_RISING`；DT 另有 `pinctrl-names="eint_as_int"` + `pinctrl-0=<0x168>`（gpio121 / function=gpio / 8mA / input-enable / bias-disable）但 `platform.c` 从未应用，实际是 `2mA pull down` | 三处同时改：① `request_threaded_irq` 改 `RISING\|FALLING\|ONESHOT`（本树 `msm_gpio_irq_set_type()` 支持 `IRQ_TYPE_EDGE_BOTH`）；② `gf_parse_dts()` 用 `of_get_named_gpio_flags()` 读 flags，并 `devm_pinctrl_get()`+`pinctrl_lookup_state("eint_as_int")`+`pinctrl_select_state()`，**任何 pinctrl 错误只打日志不返回失败**（照 08 的教训）；③ HAL 使能中断期间 5ms 轮询 GPIO 电平作为兜底，一旦真实硬件中断到达（`hw_irq_seen=1`）自动停轮询。改 `gf_spi.c`/`gf_spi.h`/`platform.c`，patch 09 |

> `08` 的基线是 pristine `drivers/spi/spi_qsd.c`（md5 `e0b8ff7a…`），v3 结果 md5 `3a130510…`，
> patch md5 `032262838e1d16d93328d94a42a86827`。**patch 内注释仍写 "dbg57"**，那是 v2 沿用的文字；
> 真正生效、已上机验证的是 **dbg58 / v3（无条件降级）**。改注释会改变已验证产物的 md5，不要动。
> `07` 的 ioctl 与官方 `gxfingerprint.default.so` 的 `0x4700~0x470a` 逐项对齐（`'G'` magic / nr 0–10）。
> `09` 的基线是 dbg58 三文件快照 `g0335d_debug/fp_replay_baseline/drivers/input/fingerprint/goodix/`
> （gf_spi.c `6cf30e00…` / gf_spi.h `944b29cd…` / platform.c `1a5c371f…`），
> patch md5 `c713abc3ec81`，`verify_fp_irq_replay.sh` 已输出 `REPLAY_OK`。

3.18.71 静态核查：marlin 的 fix01/03/05/09 目标代码在本树**不存在**，无需移植；AQ59/AQ61/AQ62 显示补丁
（`drivers/video/msm/mdss/mdss_dsi_panel.c`，13 个 marker）保持原样；`mdss_mdp.panel=` 解析仍在。

## 3. 本目录内容

```
patches/01_oreo_binder_backport.patch   阶段 1 完整 diff（源文件+Kconfig+Makefile+两份 config）
patches/02_qpnp_haptic.patch            阶段 2 单行 diff
patches/03_ext4_quota_fbe.patch         阶段 3 diff
patches/04_camera_power_compat.patch    dbg52 相机上电兜底（基线 = 已打 01~03 的 3.18.71 树）
patches/05_wlan_fw_fastfail.patch       dbg53 WLAN 固件 fast-fail（同上基线）
patches/06_restart_tz_fix.patch         dbg54 重启 TZ SCM 抑制（同上基线）
patches/07_goodix_fp_driver.patch       dbg55 Goodix GF 指纹驱动（含 5 个新文件 + Kconfig/Makefile/config）
patches/08_spi_qsd_pinctrl_fix.patch    dbg58 spi_qsd pinctrl 无条件降级（仅改 spi_qsd.c）
patches/09_goodix_fp_irq_fix.patch      dbg59 GF 中断：双沿 + eint_as_int pinctrl + 轮询兜底（3 文件）
patches/00_all_in_one.patch             干净 3.18.71 基线 → 01~03 状态，一次性重放（不含 04~09）
configs/g0335d_defconfig_android81      最终 defconfig 快照（已含 07 的两个符号）
configs/out_dot_config_android81        最终 out/.config 快照（构建用）
verify_android81.py                     幂等校验：逐项确认 01~09 全部仍在源码里
build_kernel.sh                         一键重建 Image.gz（本树，O=out）
MANIFEST.md5                            补丁集与关键源码文件校验和
```

`04/05/06/07/08/09` 各自从对应基线生成，均为**真实 diff**；
`07` 的基线保存在 `g0335d_debug/fp_port_baseline/`，`08` 的基线是 pristine `spi_qsd.c`，
`09` 的基线是 dbg58 三文件快照 `g0335d_debug/fp_replay_baseline/drivers/input/fingerprint/goodix/`。
`g0335d_debug/verify_goodix_fp_replay.sh`（07，9 文件）、`verify_restart_tz_replay.sh`（06）、
`verify_spi_qsd_replay.sh`（08）、`verify_fp_irq_replay.sh`（09）均已输出 `REPLAY_OK`。
早期三补丁的基线取自三份 `*_fix_backup/*.orig`（`apply_binder_backport.py`、`apply_haptic_fix.py`、
`apply_crypto_quota_fix.py` 当年自动保存），是严格连续的增量（clean → binder → haptic → quota/FBE → final），
`g0335d_debug/verify_patch_replay.sh` 已在 `/tmp` 临时树重放 `01+02+03`，9 个文件与最终树逐字节一致。
`verify_android81.py` 在专用树上应全项通过（编译树唯一预期 FAIL 是 `ANDROID81_ONLY.txt` 专用树标记）。

## 4. 从干净基线重建（在 WSL 内）

```bash
cd /root/<clean_gree_3.18.71_tree>
P=/root/kernel_g0335d_android81_los151/android81_port/patches
patch -p1 < $P/00_all_in_one.patch            # 01~03（binder + haptic + quota/FBE + 两份 config）
patch -p1 < $P/04_camera_power_compat.patch   # dbg52
patch -p1 < $P/05_wlan_fw_fastfail.patch      # dbg53
patch -p1 < $P/06_restart_tz_fix.patch        # dbg54
patch -p1 < $P/07_goodix_fp_driver.patch      # dbg55
patch -p1 < $P/08_spi_qsd_pinctrl_fix.patch   # dbg58（v3，无条件降级）
patch -p1 < $P/09_goodix_fp_irq_fix.patch     # dbg59（GF IRQ，基线=已打 01~08 的树）
mkdir -p out
cp /root/kernel_g0335d_android81_los151/android81_port/configs/out_dot_config_android81 out/.config
python3 /root/kernel_g0335d_android81_los151/android81_port/verify_android81.py .
```

`00_all_in_one.patch` **只到 01~03**；04~08 基线同为「已打 01~03 的树」，**09 的基线是已打 01~08 的树**
（09 改的 `drivers/input/fingerprint/goodix/` 三文件依赖 07 新建的目录）。
其余各补丁改的文件互不重叠（`msm_sensor_driver.c` / `ol_fw.c`+`cnss_pci.c` / `msm-poweroff.c` /
两份 config / `spi_qsd.c`），按上面顺序逐个应用，顺序不可颠倒。
`07` 会新建 `drivers/input/fingerprint/goodix/` 目录（`patch` 自动创建）。

## 5. 构建 / 打包 / 校验

```bash
wsl -d Ub2004 -- bash -lc "bash /root/kernel_g0335d_android81_los151/android81_port/build_kernel.sh"
wsl -d Ub2004 -- python3 /root/kernel_g0335d_android81_los151/android81_port/verify_android81.py
```

- 工具链 `/root/tc49/bin/aarch64-linux-android-`（gcc 4.9），`ARCH=arm64`，`O=out`，需 `CONFIG_NO_ERROR_ON_MISMATCH=y`。
- boot 打包：Windows 侧 `g0335d_debug/build_boot_dbg51_quota.py` 一族。**必须用原厂 124 颗 DTB**
  （`g0335d_debug/gree371_Image_dbg51_124dtb.bin`），不要用 `out/arch/arm64/boot/Image.gz-dtb`
  （源码树 dts 全集 126 颗，会改变 LK CDT 匹配顺序）。
- **boot 分区当前内容**：`g0335d_debug/boot_g0335d_dbg59_fpirq.img`，
  md5 `b644376985e221352f69ff1b274b87a5`（58,875,904 B / 14,374 页），
  内核 `Image.gz` md5 `4e2feaf3d6c7b671b7b2646421f33a0b`。
  02:29 分块刷入 `/dev/block/sde17`、读回校验通过；02:30 普通重启后 `/proc/version = #88` 验证生效。
- 回滚链：dbg58 `b2fbefb6…` → dbg57 `ae860820…` → dbg56 `01d937b4…` → dbg55 `b09418d2…` → dbg54 `e7d4a352…`
  → dbg53 `e931f3f5…` → dbg52 `0056e734…` → dbg51 `d7b74b15…` → dbg42 `22ff69a9…` / dbg41 `8648777a…`。
  上一个稳定点 dbg54 `e7d4a3526b9a44d5968eabffd59698d4`（22:03 / 22:16 两次开机验证过）。
- dbg55~59 各打包脚本只替换 `Image.gz`；124 颗原厂 DTB（`fbec1d69…`）、两段 ramdisk、cmdline 与
  header 除 `kernel_size` 外逐字节不变。`wlan.ko` 自 dbg54 起字节一致（`2ab16d4d…`），vermagic 未变。
- dbg59 构建/打包/刷写：`build_kernel_dbg59.sh`（零 error/零 warning）、
  `build_boot_dbg59_irq.py`、`flash_dbg59.sh`。
- 开机后只读验收：`g0335d_debug/verify_dbg55_goodixfp.sh`（`/dev/goodix_fp`、`spi5.0`、
  `gf_probe` / pinctrl 降级日志、IRQ、开机耗时、WiFi）、`verify_fp_runtime.sh`（指纹链路）。

## 6. 设备侧（vendor）修复 —— 不属于内核源码

固化在 `g0335d_debug/android81_vendor/`：

| 文件 | 目标路径 | md5 |
|---|---|---|
| `wlan.ko` | `/vendor/lib/modules/wlan.ko` | `2ab16d4d…`（dbg53/54 产物；旧版 `fd44237a…` 留作 `wlan.ko.dbg51bak`） |
| `wlan_g0335d.rc` | `/vendor/etc/init/wlan_g0335d.rc` | `01b5a926…` |
| `tfa98xx.cnt` | `/vendor/firmware/tfa98xx.cnt` | `30c047a1…` |
| `audio.primary.msm8996.so` | `/vendor/lib/hw/audio.primary.msm8996.so` | `bb9c673b…` |

外加 `settings put system screen_brightness_mode 0`（关自动亮度）。这些是 `/system` 落盘文件 +
settings，`deploy_all.sh` 可一键重放，**运行前必须单独获得用户授权**（会写 `/system`）。

### 6.1 指纹 vendor（阶段 2，已部署）

素材目录 `g0335d_debug/fp_vendor_assets/`，部署脚本 `deploy_fp_vendor.sh`，
回滚脚本 `rollback_fp_vendor.sh`（备份在设备 `/cache/fp_vendor_backup/`）。

| 内容 | 目标路径 | 说明 |
|---|---|---|
| `fingerprint.qcom.so` | `/vendor/lib64/hw/fingerprint.qcom.so` | **当前 = Plan B 官方上层 HAL**（见 6.2）：官方 `/lib64/hw/fingerprint.default.so` 182352 B，md5 `e96b4fcc…`，**零 patch**。v1~v4 补丁版（末版 `72444dac…`）已备份为本地 `.v4bak` 与设备 `/cache/fp_vendor_backup/planb/fingerprint.qcom.so.v4`，可用 `rollback_fp_planb.sh` 回退 |
| `libfpnav.so` | `/vendor/lib64/libfpnav.so` | 官方依赖，`293a0f64…` |
| `goodixfp.*` ×9 | `/system/etc/firmware/` | QSEE TA（mdt/b00~b06/flist） |
| `fps_hal.rc.enabled` | `/vendor/etc/init/android.hardware.biometrics.fingerprint@2.1-service.rc` | 去掉 `disabled`，`class late_start` / `user system` / `group system input` |
| feature xml | `/vendor/etc/permissions/android.hardware.fingerprint.xml` | 由 `.disabled` 改回 `.xml` |
| ueventd 规则 | `/vendor/ueventd.rc:272` | `/dev/goodix_fp 0660 system system` |

同时移走 init 目录里的 `fps_hal.rc.bak`（Android 8.1 init 会解析 `.bak`，曾导致 121s 阻塞），
并恢复被止血脚本禁用的 QFP 特征声明。**这些写 `/system`，执行前需单独授权。**

### 6.2 指纹 Plan B：恢复官方两层架构（02:05 部署，用户授权）

**根因（推翻上面 6.1 的 v1~v4 路线）**：底层 `gxfingerprint.default.so` 的 device 是
728 字节 Goodix 私有结构，`open()` 从 offset 112 起填的全是 `fnCa_*` thunk
（112=free wrapper、120=OpenSession、136=Init、144=Reset、152=device_setMode、
176=fnCa_Register…）。8.1 HIDL 服务在 `112/136/144/152/160/168/176/184/192/200`
按 AOSP `fingerprint_device_t` 语义取函数指针 —— **偏移数字碰巧重合、语义完全错位**，
所以 `enroll()` 落到私有 thunk，`Bad input argument. NULL Buffer.` → `startEnroll failed, result=1`。
**此前「无整体 ABI 错位」的结论作废**；v1~v4 全是在错误的层上打补丁。

| 文件 | 目标路径 | md5 |
|---|---|---|
| 上层 HAL（**零 patch**） | `/vendor/lib64/hw/fingerprint.qcom.so` | `e96b4fcc…` |
| 原始底层 HAL | `/vendor/lib64/hw/gxfingerprint.default.so` | `5cd06e61…` |
| binder 客户端 / 服务端 | `/vendor/lib64/libfp_client.so` / `libfpservice.so` | `2ffa1d7d…` / `34603206…` |
| daemon / 调试工具 | `/vendor/bin/gx_fpd` / `gx_fpcmd` | `bea76fce…` / `7c17733f…` |
| init rc | `/vendor/etc/init/00_gx_fpd.rc` | `9d6de9e5…` |

`00_` 前缀保证 init 先于 `fps_hal` 解析；`class late_start` / `user system` / `group system input drmrpc`。
链路：`fps_hal → fingerprint.qcom.so(上层) → libfp_client.so → goodix.fp(gx_fpd) →
libfpservice.so → gxfingerprint.default.so → QSEE TA + /dev/goodix_fp`。
素材 `g0335d_debug/fp_official_upper/`（全部从官方 `system.img` 只读导出）；脚本
`deploy_fp_planb.sh` / `rollback_fp_planb.sh` / `probe_fp_planb.sh`；只读分析
`analyze_fp_vtable.sh` / `export_fp_upper.sh` / `analyze_fp_planb.sh` / `prepare_fp_planb.sh`。
**刻意不动** `/vendor/lib64/libQSEEComAPI.so`（设备 31352 B vs 官方 30704 B，现版可用）。

验证（02:08）：`goodix.fp: [android.hardware.IFpService]` 注册、
`goodix_sensor_init() ok!`、`HAL transmit cmd[81] to TA`、
`fingerprint_enroll(gid=0, timeout_sec=60)` → `Client::register 1, timeout 60 s`
→ `===> Waiting Finger Down.` / `gf_enable_irq`；
`startEnroll failed` / `Bad input argument` / `Handle is NULL` / `vendor -1` **全为 0**。
> `setsid gx_fpd &` 后必须让 adb shell 存活 ~3s，否则 SIGHUP 会杀掉子进程（脚本已修）。
> **02:30 整机重启后已验证 `00_gx_fpd.rc` 开机自启**：`init.svc.gx_fpd=running`（PID 691），
> `service list` 有 `goodix.fp: [android.hardware.IFpService]`，不再需要手动 `setsid`。
> 残留竞态：`fps_hal` 与 `gx_fpd` 同属 `class late_start`，02:30:44 首次 `Fp::connect failed!`，
> 约 50s 后（02:31:35）同一进程重试成功；开机后约 1 分钟内指纹可能不可用，待修（需再写 `/system` 改 rc）。

## 7. 状态（2026-11-16 02:42，dbg59 + Plan B 两层架构 + HATfix）

> 下面第 8 节是 02:12 的历史状态，**已被本节覆盖**；保留仅作纠错记录。

- **当前 boot = dbg59** `b644376985e221352f69ff1b274b87a5`（3.18.71 + 01~09 补丁），
  `/proc/version = #88`，02:30 普通重启后生效。**普通重启仍可用**（dbg54 的 TZ 修复未回归）。
- 固化 `g0335d_port/android81/` ≡ 两棵树 `android81_port/`：01 binder / 02 haptic /
  03 quota-FBE / 04 相机兜底 / 05 wlan fast-fail / 06 重启 TZ / 07 Goodix GF 驱动 /
  08 spi_qsd pinctrl 无条件降级 / **09 GF IRQ 三件套**。`verify_android81.py` 已加 09 的 6 项检查。
- **★ 中断已修好（patch 09 / dbg59）**：02:30 重启后 `/proc/interrupts` 从恒 0 变为
  `479: 13 … msmgpio 121 gf`，随后累计到 137+12；dmesg 有
  `GREE_GOODIX_FP dbg59: hw irq=479 count=… level=1/0`，且计数稳定（非中断风暴）。
  `gpio121` 的 pinctrl 应用/降级日志、双沿 type=BOTH 日志均已在内核中。
- **★ 官方两层架构已开机自启**：`gx_fpd`（`goodix.fp` binder 服务）、`fps_hal`、
  `fingerprint` framework 服务、feature xml、`/dev/goodix_fp`(system:system 0660) 全在；
  `goodix_sensor_init() ok!`、固件 `GFx18M_1.05.11`、算法 `Milan_v_2.01.20` 已读到。
  WiFi / 亮度 / 三 binder 无回归。
- **02:34 用户实测「还是无法录入」→ 新根因（与 IRQ 无关）**：
  `FingerprintService: startEnroll failed, result=1` +
  `FingerGoodix: fingerprint_enroll: invalid or null auth token`。
  反汇编官方上层 HAL `fingerprint_enroll()`（vaddr 0x87f0）：
  `88b0 cbnz w0,892c`（`hat->version != 0` → -93）、
  `88bc cmp x1,x0`（HAT challenge 与 `pre_enroll` 随机数比）、
  `88c8 tbz w0,#1,8958`（`authenticator_type` bit1=0 → -1）。
  8.1 framework 传下来的 `hw_auth_token_t` 内容基本全零，7.1 HAL 因此**在通知传感器等手指之前就返回错误**。
- **HATfix 已部署（02:40，写 `/system` 后已恢复 ro）**：从官方 pristine 上层 HAL
  （md5 `e96b4fcc…`）把上述两条拒绝分支改成 `NOP`（`0x350003e0`→`0xd503201f`、
  `0x36080480`→`0xd503201f`），产物 md5 `809f096814b08d2091258f74d7884e48`；
  其余指令、`goodix_sensor_fp_set_user_id()` / `goodix_sensor_fp_enroll()` 调用点逐字节不变，
  反汇编已验证。脚本 `patch_fp_upper_hat.py` / `deploy_fp_upper_hatfix.sh` /
  `rollback_fp_upper_hatfix.sh`；备份 `/cache/fp_vendor_backup/planb/fingerprint.qcom.so.planb`，
  本地 pristine `fp_official_upper/fingerprint.default.so`。
  **代价**：绕过「录入必须出示有效密码派生 auth token」的校验（单机移植可接受，可一键回滚）。
  注意 `invalid or null auth token` 这句日志**仍会打印**（已降级为警告），判据是 `startEnroll` 不再失败。
- **待用户实测**：屏幕已唤醒、Settings 已 force-stop 并重置到安全设置页、logcat 已清空，
  请**重新**从 设置 → 安全 → 指纹 → 添加指纹 进入（不要停留在旧的录入界面），
  按提示反复触摸传感器，看是否出现录入进度；再锁屏用指纹解锁。
- **仍待处理**：① `fps_hal`/`gx_fpd` 开机竞态（约 50s 后自愈，需改 rc，需另行授权写 `/system`）；
  ② 外放/听筒、拖亮度条肉眼变化、息屏→电源键唤醒肉眼亮屏；③ `sensors.qcom` 每 ~10s 重启（非阻塞）。
- **顺序约束：先指纹、后相机**。相机仍未修（`Number of camera devices: 0`，结论是需要同世代 8.1
  框架库，不再照搬官方 7.1 sensor 库硬凑）。

## 8. 历史状态（2026-11-16 02:12，dbg58 + Plan B 两层架构）


- **dbg54 已生效并验证**：`/proc/version = #83`，`logcat -b kernel` 有
  `GREE_RESTART_TZ_FIX dbg54: set_dload_mode(1) IMEM only, TZ SCM suppressed`；
  pstore 空、无 9s PS_HOLD 循环。电源菜单「重新启动」/`adb reboot` 已可用，不再需要长按 10s 或 TWRP。
- 开机耗时：`start=4.5s` / `system_run=6.1s` / `enable_screen=36.8s`（原 125.6s）；
  WiFi `enabled` + `wlan0 UP` + `wpa_supplicant running`；crash buffer 空。
  19:58 开机慢根因 = `/vendor/etc/init/wlan_g0335d.rc.bak` 被 init 解析、同步 `insmod` 阻塞 121s；
  `.bak` 已移走。异步 rc 实验因打乱 WiFi HAL 时序已被推翻，21:41 恢复同步 rc `01b5a926…`；
  insmod 变快靠 dbg53/54 的 `request_firmware_direct()`，**不是**异步化。
- **指纹阶段 1（内核）已完成并验收**：dbg58 开机后 `/dev/goodix_fp`（`crw-rw---- system system 154,0`）、
  `/sys/bus/spi/devices/spi5.0`、`goodix_fp as /devices/virtual/input/input5` 全部存在，
  内核日志有 `pinctrl failed during init, continuing (err=-22, init_done=1)` 与 `gf_probe --> 603, entry`。
- **指纹阶段 2（vendor）已完成**：HIDL `fps_hal` running，`lshal` 注册
  `android.hardware.biometrics.fingerprint@2.1::IBiometricsFingerprint/default`；
  日志 `Open device[/dev/goodix_fp] success`、`QSEECOMAPI: Loaded image: APP id = 6`、
  `Ta start success`、`fnCa_DownloadCFG OK`，Goodix HAL 持续正常轮询。
- **指纹阶段 3（8.1 HAL 适配）已完成**。反汇编证据（不是推测）：
  `android.hardware.biometrics.fingerprint@2.1-service` 的 `openHal()` 是
  `233c: ldr w4,[x19,#4]` / `2340: cmp w4,#0x201` / `2344: b.ne 2378` —— 检查的是
  **`hw_device_t.version`（device 结构 +4）**，不是 `hw_module_t.module_api_version`。
  官方 HAL 的 `open()` 在 `c110: mov w0,#0x100` / `c114: str w0,[x19,#4]` 写入 256。
  把该立即数改成 `mov w0,#0x201`（`0x52802000`→`0x52804020`）后，
  `Wrong fp version` / `Can't open HAL module` **彻底消失**，服务走到
  `Opening fingerprint hal library...` → 加载 HAL → `fingerprint device enable`。
  > **结论纠错记录**：`Expected 513, got 256` 曾被误判为「第一版 HAL 的旧日志 / 缓冲区残留」；
  > 00:44 整机重启后新 PID 794 在 00:44:41 复现，证明是**每次启动都会发生的真检查**。
  > 改 `module_api_version` 无效，因为服务根本不读那个字段。
- **整机普通重启（00:44:20，用户授权）打通 framework 层**：
  00:05:36 的 `SystemConfig` 因 `android.hardware.fingerprint.xml.disabled` 忽略了 feature，
  重启后重新扫描 → `pm list features` 有 `feature:android.hardware.fingerprint`，
  `dumpsys fingerprint` 返回真正的 `Fingerprint Manager` JSON（原为 `Can't find service`）。
  阶段 1/2 持久化全绿，WiFi / 亮度 / 三 binder 无回归，tombstone 无新增。
- **v3 HAL 部署（00:53，写 `/system` 后已恢复 ro）**：md5 `c55efcaa…`，
  `ctl.restart fps_hal` 后 PID 794→14454，设备打开 / QSEE TA(`APP id=6`) /
  `fnCa_DownloadCFG OK` 全绿，版本错误日志为空。`adb root` 后看到的 50 个 tombstone
  全是 00:05 及更早的历史文件（最新为 00:44 的 0 字节文件），非本次新增。
- **01:08 用户实测：有「指纹」入口、能进录入界面、但录不进去**（`startEnroll failed, result=1`
  + `Bad input argument. NULL Buffer.` + 全程 `Handle is NULL`）。由此做了架构纠错与 v4 修复。
- **★ 架构纠错（勿再沿用旧结论）**：官方 7.1 是**两层**，此前「官方 HAL 不依赖 gx_fpd」
  只对下层成立。
  | 层 | 文件 | 角色 |
  |---|---|---|
  | 上层 HMI | `/lib64/hw/fingerprint.default.so` 182352 B | 真正的 `HMI`，`module_api_version` 本就 `0x0201`；NEEDED `libfp_client.so`；导出 `fingerprint_enroll` / `goodix_sensor_fp_enroll` / `pre_enroll` / `post_enroll` / `load_all_fpdata` |
  | 桥接 | `/lib64/libfp_client.so` 231232 B + `/bin/gx_fpd` 124888 B | binder 服务 `goodix.fp` / `IFingerPrint`，`gx_fpd` NEEDED `libfpservice.so` |
  | 下层实现 | `/lib64/hw/gxfingerprint.default.so` 88160 B | **我们 patch 的就是它**；只导出 `device_*` / `fnCa_*` / `gx_ta_*` |
  - 下层把 `fnCa_OpenSession` / `fnCa_Init` / `fnCa_CloseSession` 做成导出 thunk
    （`c0b8` / `c0bc` / `c0c4`）**专供上层管理 TA 会话**；`fnCa_OpenSession`(0x9608) 是唯一
    调 `gx_ta_start` 处，`fnCa_CloseSession`(0x9670) 是唯一调 `gx_ta_stop` 处。
  - 我们绕过上层直挂 8.1 HIDL ⇒ 初始化关掉会话后再没人重开。
- **TA 生命周期实时复现（01:22:55，`goodix.fp.debug=1` + `ctl.restart fps_hal`）**：
  `gx_ta_start`→`APP id=8`→`Ta start success`→`fnCa_DownloadCFG OK`→
  `preprocessor init(cmd 83) success`→`fingerprint device enable`→
  `gx_ta_stop`→`QSEECom_shutdown_app`→`Ta stop success`→注册 HIDL 服务→每 2s `Handle is NULL`。
- **v4 已部署（01:28，写 `/system` 后已恢复 ro）**：md5 `72444dac…`，PID 16861→17355。
  `gx_ta_stop` / `Handle is NULL` / `Bad input argument` / `Wrong fp version` / `startEnroll failed`
  **全为 0**，无新增 tombstone；`Ta start success` → `fnCa_DownloadCFG OK` →
  `preprocessor init success` → `fingerprint device enable` →（假）`Ta stop success`，TA 常驻。
  副作用：QSEE TA 常驻带来额外空闲耗电；日志仍打印假的 `Ta stop success.`。
  设备备份 `/cache/fp_vendor_backup/lib64/hw/fingerprint.qcom.so.v3`，本地 `.v3bak`。
- **02:10 用户实测「还是没有出现录入进度」→ 新卡点在内核 IRQ（与 HAL 无关）**：
  软件栈已全通（Plan B，见 6.2），`startEnroll` 成功且 HAL 进入 `IMAGE mode` /
  `===> Waiting Finger Down.` / `gf_enable_irq`，但**没有任何 Finger Down / 采图事件**，
  约 21s 后 `fingerprint_cancel` → `onError(5)`。
  `/proc/interrupts` 的 `479: 0 0 0 msmgpio 121 gf` **计数恒为 0**，
  `disable_depth=0` / `spurious=0`；内核日志 `gf_ioctl --> 310` 即 `GF_IOC_ENABLE_IRQ`
  （行号偏移 8，源码 302），HAL 每次都正确启用中断。
  `gf_irq()` 靠 `kill_fasync(SIGIO, POLL_IN)` 通知 HAL，IRQ 不来就不采图。
- 驱动侧（`fp_driver_src/platform.c` + `gf_spi.c`）两个明确缺陷：
  ① `request_threaded_irq(..., IRQF_TRIGGER_RISING | IRQF_ONESHOT, "gf", ...)` 硬编码上升沿，
     `of_get_named_gpio()` 未读 flags；DT `goodix,gpio_irq = <0x36 0x79 0x01>`
     （0x79=121、flags=1 低有效）。
  ② DT 有 `pinctrl-names="eint_as_int"` + `pinctrl-0`，但 `platform.c` 无任何 pinctrl 代码；
     `pinctrl-maps` / `pinctrl-handles` 查不到该 state（与 08 patch 的 pinctrl 代际问题一致）。
  `/sys/kernel/debug/gpio`: `gpio121 : in 0 2mA pull down`。
  `KEY:key touch_status = 0xb0` 是 HAL 轮询 SPI 的导航键状态，不依赖 IRQ。
  ⇒ **下一步需改驱动 + 重编译 + 刷 boot（dbg59），按铁律需用户授权**；
  建议一次覆盖「应用 `eint_as_int` pinctrl」+「双沿/下降沿触发」，只刷一次。
- 已验证：WiFi 开机自动可用、真音频 HAL 加载、亮度滑块生效；启动链 `boot_completed=1` /
  `mount_all returned 1` / `vold.decrypt` 空 / 三个 binder / vibrator sysfs 全绿。
- 仍待用户实测：外放与听筒出声、拖亮度条肉眼变化、息屏→电源键唤醒肉眼亮屏、指纹录入/解锁。
- **顺序约束：先指纹、后相机**。相机仍未修（`Number of camera devices: 0`，结论是需要同世代 8.1
  框架库，不再照搬官方 7.1 sensor 库硬凑）；`sensors.qcom` 每 ~10s 重启（非阻塞）未处理。
- 本轮新增只读分析脚本：`analyze_fp_enroll.sh` / `analyze_fp_ta_lifecycle.sh` /
  `analyze_fp_enable.sh` / `analyze_fp_deep.sh`；产物 `fp_hal_v3.dis`、`fp_hidl_service.dis`、
  `fp_funcs.txt`、`fp_deep_out.txt`、`fp_enable_out.txt`。
