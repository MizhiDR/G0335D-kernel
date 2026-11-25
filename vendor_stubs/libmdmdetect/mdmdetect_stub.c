/* Minimal stub for libmdmdetect.so
 * Enough for the linker. Camera will not fully work.
 */
#include <stdint.h>
#include <stddef.h>

int get_system_info(void *info) { (void)info; return -1; }
int get_modem_info(void *info) { (void)info; return -1; }
int get_esoc_details(void *details) { (void)details; return -1; }
int esoc_framework_get_info(void *info) { (void)info; return -1; }

/* keep some common names that camera daemons probe */
int libmdmdetect_init(void) { return 0; }
void libmdmdetect_deinit(void) {}
