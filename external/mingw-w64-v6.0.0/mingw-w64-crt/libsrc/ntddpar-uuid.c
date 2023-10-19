/* Parallel port device GUIDs */
/* from ntddpar.h */

#define INITGUID
#include <basetyps.h>

#define GUID_PARALLEL_DEVICE GUID_DEVINTERFACE_PARALLEL
#define GUID_PARCLASS_DEVICE GUID_DEVINTERFACE_PARCLASS

DEFINE_GUID (GUID_DEVINTERFACE_PARALLEL, 0x97F76EF0, 0xF883, 0x11D0, 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C);
DEFINE_GUID (GUID_DEVINTERFACE_PARCLASS, 0x811FC6A5, 0xF728, 0x11D0, 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1);

