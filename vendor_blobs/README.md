# Stock vendor blobs (from G0335D unbrick ROM)

Real libs extracted from the Needrom / QPST unbrick dump.

```
vendor_blobs/proprietary/vendor/lib/
vendor_blobs/proprietary/vendor/lib64/
  libmdmdetect.so
  libcne.so
  libcneapiclient.so
```

Copy into the LOS vendor tree:
```
cp -a vendor_blobs/proprietary/vendor/lib* vendor/gree/g0335d/proprietary/vendor/
```

Or add to the device makefile so the build picks them up.

These replace the shims in `vendor_stubs/` for the linker abort.
