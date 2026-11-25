# Stock vendor blobs — full fix package (64-bit)

Extracted from the G0335D unbrick ROM. All files are AArch64.

```
vendor_blobs/proprietary/vendor/lib64/
  libmdmdetect.so
  libcne.so
  libcneapiclient.so
  libcneqmiutils.so
  libdiag.so
  libdsutils.so
  libidl.so
  libmmcamera2_mct.so
  libmmcamera2_stats_modules.so
  libmmcamera2_sensor_modules.so
  libmmcamera2_iface_modules.so
  libmmcamera2_imglib_modules.so
  libmmcamera2_pproc_modules.so
  libmmcamera2_cpp_module.so
  libmmcamera2_stats_algorithm.so
  libmmcamera_imglib.so
```

## Install into LOS vendor tree
```
cp -a vendor_blobs/proprietary/vendor/lib64 \
      vendor/gree/g0335d/proprietary/vendor/
```

## Protobuf note
`libcne.so` / `libcneapiclient.so` were built against the stock protobuf.
On LOS 15.1 they may still miss symbols. Keep `vendor_stubs/protobuf_stub`
and the `LD_SHIM_LIBS` line from `vendor_stubs/g0335d_shims.rc` as fallback.

## Limits
- 64-bit only. 32-bit processes need matching 32-bit libs (not in this dump).
- Camera still needs the correct sensor lib + chromatix for the G0335D panel.
- This removes the known linker aborts. It is not a guaranteed working camera/modem.
