# G0335D vendor shims (LOS 15.1)

Proper-enough shims so the device can boot past the linker aborts
seen in the logcat:

- `mm-qcamera-daemon`: `libmdmdetect.so` not found
- `cnd` / `netmgrd` / `imsdatadaemon`: protobuf symbol mismatch in
  vendor `libcne.so` / `libcneapiclient.so`

## What is implemented

### libmdmdetect
- `count_modems()` → returns 1 (integrated msm8996 modem)
- `get_modem_name()` → `"msm8996"`
- `is_external_modem()` → 0
- `get_system_info` / `get_modem_info` / `get_esoc_details` /
  `esoc_framework_get_info` → success (0)
- init/deinit

This matches the public contract used by AOSP QTI audio/camera
(see `count_modems()` in platform/hardware/qcom/audio).

### protobuf shim (`libprotobuf_stub_cne`)
Exports the exact symbols that abort the link, plus a few common
follow-up ones:

- `_ZN6google8protobuf2io16CodedInputStream20ReadVarint32FallbackEPj`
- `_ZN6google8protobuf8internal21StringTypeHandlerBase3NewEv`
- `_ZN6google8protobuf8internal21StringTypeHandlerBase10DeleteEv`
- `_ZN6google8protobuf2io16CodedInputStream10ReadTagSlowEv`
- `_ZN6google8protobuf2io16CodedInputStream17ReadVarint64FallbackEPy`

## Build in LOS tree

```
vendor/gree/g0335d/stubs/libmdmdetect/
vendor/gree/g0335d/stubs/protobuf_stub/
```

In device makefile:
```
PRODUCT_PACKAGES += libmdmdetect libprotobuf_stub_cne
```

Copy `g0335d_shims.rc` into the device tree and include it, or add
the `LD_SHIM_LIBS` line to `init.qcom.rc`.

## After flashing
Push if not in the image:
```
adb root
adb remount
adb push libmdmdetect.so /vendor/lib/
adb push libmdmdetect.so /vendor/lib64/
adb push libprotobuf_stub_cne.so /vendor/lib64/
adb reboot
```

## Limits
- Camera will get past the missing-lib abort. Full camera still
  needs the real mm-camera blobs and sensor libs.
- CNE/IMS/netmgrd will get past the protobuf abort. Data/IMS may
  still fail if more symbols are missing — add them to the stub
  as they appear in logcat.
- Real stock blobs remain the correct long-term fix.
