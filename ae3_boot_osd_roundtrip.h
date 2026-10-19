#ifndef AE3_BOOT_OSD_ROUNDTRIP_H
#define AE3_BOOT_OSD_ROUNDTRIP_H
#include "ae3_boot_osd_entry.h"
/* Guest-owned original EE OSD getter/setter sequence. The OS configuration
 * must be explicitly supplied to this compatibility-only model; we do not
 * infer the user's BIOS configuration from a post-boot savestate. */
#define AE3_OSD_GET_PC UINT32_C(0x00435B94)
#define AE3_OSD_SET_PC UINT32_C(0x00435B84)
#define AE3_OSD_PARENT_RETURN UINT32_C(0x0043FE98)

typedef struct AE3_EEOsdConfig {
    uint32_t packed;
    uint32_t generation;
    uint8_t bound;
} AE3_EEOsdConfig;

typedef enum AE3_OsdRoundtripResult {
    AE3_OSDR_YIELDED=1,
    AE3_OSDR_GET_PENDING=2,
    AE3_OSDR_SET_PENDING=3,
    AE3_OSDR_PARENT_RETURNED=4,
    AE3_OSDR_UNBOUND=-1,
    AE3_OSDR_MEMORY_FAULT=-2,
    AE3_OSDR_UNSUPPORTED=-3,
    AE3_OSDR_INVALID=-4
} AE3_OsdRoundtripResult;

typedef struct AE3_BootOsdRoundtrip {
    AE3_BootPrefix cpu;
    uint32_t read_count,write_count;
    uint8_t active;
} AE3_BootOsdRoundtrip;

void ae3_ee_osd_reset(AE3_EEOsdConfig *osd);
int ae3_ee_osd_bind(AE3_EEOsdConfig *osd,uint32_t explicit_packed_config);
AE3_OsdRoundtripResult ae3_boot_osd_roundtrip_enter(
    AE3_BootOsdRoundtrip *out,const AE3_BootOsdEntry *prior);
AE3_OsdRoundtripResult ae3_boot_osd_roundtrip_slice(
    AE3_BootOsdRoundtrip *state,AE3_EEMemoryBus *bus,uint32_t budget);
AE3_OsdRoundtripResult ae3_boot_osd_roundtrip_get(
    AE3_BootOsdRoundtrip *state,AE3_EEMemoryBus *bus,
    const AE3_EEOsdConfig *osd);
AE3_OsdRoundtripResult ae3_boot_osd_roundtrip_set(
    AE3_BootOsdRoundtrip *state,const AE3_EEMemoryBus *bus,
    AE3_EEOsdConfig *osd);
#endif