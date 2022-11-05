#include <SCBW/api.h>
#include <hook_tools.h>
#include "datext.h"

namespace {
    // List of address reference replacements.
    s32 portdata_r[] = {/* idle smk */   0,  0x0044EC1D +3, 0x0044F1E2 +3, 0x0044F211 +2, 0x0045E80D +3, 0x0045F3D2 +3, 0x0045F401 +2,  0,
                                         4,  0x0044F250 +3, 0x0044F27F +2, 0x0045F440 +3, 0x0045F46F +2,  0,
                                         8,  0x0044F2BE +3, 0x0044F2ED +2, 0x0045F4AE +3, 0x0045F4DD +2,  0,
                                         12, 0x0044F32C +3, 0x0044F35B +2, 0x0045F51C +3, 0x0045F54B +2,  0,
                                         16, 0x0044F39A +3, 0x0044F3C9 +2, 0x0045F58A +3, 0x0045F5B9 +2,  0,
                                         0,
                        /* talk smk */   0,  0x0044ED01 +3, 0x0044F219 +3, 0x0044F248 +2, 0x0045E8FD +3, 0x0045F409 +3, 0x0045F438 +2,  0,
                                         4,  0x0044F287 +3, 0x0044F2B6 +2, 0x0045F477 +3, 0x0045F4A6 +2,  0,
                                         8,  0x0044F2F5 +3, 0x0044F324 +2, 0x0045F4E5 +3, 0x0045F514 +2,  0,
                                         12, 0x0044F363 +3, 0x0044F392 +2, 0x0045F553 +3, 0x0045F582 +2,  0,
                                         16, 0x0044F3D1 +3, 0x0044F400 +2, 0x0045F5C1 +3, 0x0045F5F0 +2,  0,
                                         0,
                        /* smk change */ 0,  0x0044F414 +6, 0x0044F436 +2, 0x0045F604 +6, 0x0045F626 +2,  0, 0,
                        /* unknown 1  */ 0,  0,0, // these look unused, but I assure you they are not
                        /* unknown 2  */ 0,  0,0,
                        /* unknown 3  */ 0,  0,0};
    
    // Portdata.dat tables
                // factor, add, size,       ptr, ptr, ptr, ...,  0,  ...
    s32 port_consts[] = {4,  0, sizeof(int), 0x0044F409 +1, 0x0045F5F9 +1,  0,
                         0,0,0,0};
}

namespace DatExt {
    void portdata_dat_doPatch(u32 newCount) {
        datTablePatch((DatLoad*)LoadTable::Portdata_Dat, portdata_r);
        const_replace(port_consts, newCount);
    }
}