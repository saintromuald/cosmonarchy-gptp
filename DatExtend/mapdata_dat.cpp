#include <BW/api.h>
#include <hook_tools.h>
#include "datext.h"

namespace {
    // List of address reference replacements.
    s32 mapdata_r[3] = {0}; // TODO: this
}

namespace DatExt {
    //void mapdata_dat_do_patch(u32 newCount) {
    //  dat_table_patch((DatLoad*)LoadTable::Mapdata_Dat, mapdata_r);
    //}

    //void mapdata_dat_unpatch() {
    //}
}