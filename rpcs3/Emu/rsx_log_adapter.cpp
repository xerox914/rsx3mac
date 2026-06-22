#include "rsx_log_adapter.h"

namespace rsx
{
    u32 surf_hash(u32 offset)
    {
        offset ^= offset >> 16;
        offset *= 0x7feb352d;
        offset ^= offset >> 15;
        offset *= 0x846ca68b;
        offset ^= offset >> 16;
        return offset;
    }
}
