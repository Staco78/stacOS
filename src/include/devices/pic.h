#pragma once
#include <types.h>

namespace Devices
{
    namespace PIC
    {
        void init();
        void disable();
        void maskIRQ(uint8 num);
        void unmaskIRQ(uint8 num);
        void sendEOI(uint8 num);
    } // namespace PIC

} // namespace Devices
