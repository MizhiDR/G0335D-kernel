# Vendor stubs for G0335D LOS 15.1 bring-up

These are **stubs only**. They stop the linker abort. They do not implement real functionality.

## What they cover
- `libmdmdetect.so` — missing, required by `mm-qcamera-daemon`
- protobuf symbols required by vendor `libcne.so` / `libcneapiclient.so`:
  - `_ZN6google8protobuf2io16CodedInputStream20ReadVarint32FallbackEPj`
  - `_ZN6google8protobuf8internal21StringTypeHandlerBase3NewEv`

## How to build in LOS tree
Copy into the vendor tree, e.g.:
```
vendor/gree/g0335d/stubs/libmdmdetect/
vendor/gree/g0335d/stubs/protobuf_stub/
```
Add to the device makefile:
```
PRODUCT_PACKAGES += libmdmdetect libprotobuf_stub_cne
```
For the protobuf symbols to be visible to CNE, you may also need to preload or wrap.  
Simplest test: build `libmdmdetect` and push it to `/vendor/lib/` and `/vendor/lib64/`.

## Limits
- Camera daemon may start but will fail later.
- CNE / ims / netmgrd may still crash if they call more missing symbols.
- Real blobs from stock firmware are still required for working camera and modem data.
