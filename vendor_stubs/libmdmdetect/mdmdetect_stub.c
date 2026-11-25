/*
 * libmdmdetect stub for G0335D / msm8996 (integrated modem).
 * Implements the symbols commonly required by mm-qcamera-daemon
 * and other QTI userspace. Returns a single internal modem so
 * callers do not treat the device as modem-less.
 *
 * This is not a full Qualcomm clone. It is enough for the daemon
 * to pass the link stage and basic modem-count checks.
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MDM_NAME_INTERNAL "msm8996"

/* AOSP-style: returns number of modems, or -1 on error.
 * mm-qcamera and audio HALs use this (see platform/hardware/qcom/audio).
 */
int32_t count_modems(void)
{
	return 1; /* integrated modem */
}

/* Common extended APIs seen in QTI trees */
int get_system_info(void *info)
{
	(void)info;
	return 0;
}

int get_modem_info(void *info)
{
	(void)info;
	return 0;
}

int get_esoc_details(void *details)
{
	(void)details;
	return 0;
}

int esoc_framework_get_info(void *info)
{
	(void)info;
	return 0;
}

/* Some builds call these */
int libmdmdetect_init(void) { return 0; }
void libmdmdetect_deinit(void) {}

/* Name lookup used by a few camera/audio paths */
const char *get_modem_name(void)
{
	return MDM_NAME_INTERNAL;
}

int is_external_modem(void)
{
	return 0; /* integrated */
}
