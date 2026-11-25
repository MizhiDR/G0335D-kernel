/*
 * Protobuf shim for vendor libcne.so / libcneapiclient.so on LOS 15.1.
 * Provides the symbols that abort the link.
 * Additional common protobuf symbols are stubbed so later calls
 * do not immediately fault.
 *
 * CNE / IMS data will not be fully functional. The goal is a boot
 * that does not SIGABRT in cnd / netmgrd / imsdatadaemon.
 */
#include <cstdint>
#include <cstddef>
#include <cstring>

extern "C" {

/* _ZN6google8protobuf2io16CodedInputStream20ReadVarint32FallbackEPj */
int _ZN6google8protobuf2io16CodedInputStream20ReadVarint32FallbackEPj(
		void *self, uint32_t *value)
{
	(void)self;
	if (value)
		*value = 0;
	return 0;
}

/* _ZN6google8protobuf8internal21StringTypeHandlerBase3NewEv */
void *_ZN6google8protobuf8internal21StringTypeHandlerBase3NewEv(void)
{
	static char dummy[64];
	return dummy;
}

/* A few more that frequently appear next on msm8996 CNE blobs */
void *_ZN6google8protobuf8internal21StringTypeHandlerBase10DeleteEv(void *p)
{
	(void)p;
	return nullptr;
}

int _ZN6google8protobuf2io16CodedInputStream10ReadTagSlowEv(void *self)
{
	(void)self;
	return 0;
}

int _ZN6google8protobuf2io16CodedInputStream17ReadVarint64FallbackEPy(
		void *self, uint64_t *value)
{
	(void)self;
	if (value)
		*value = 0;
	return 0;
}

} /* extern "C" */
