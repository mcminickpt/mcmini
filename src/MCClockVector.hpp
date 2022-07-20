#ifndef MC_MCCLOCKVECTOR_H
#define MC_MCCLOCKVECTOR_H

#include <stdint.h>

struct MCClockVector final
{
    void expand();

    uint32_t size() const;

    uint32_t &operator[](uint32_t index) const;

    static MCClockVector max(const MCClockVector &cv1, const MCClockVector &cv2) {
        MCClockVector maxCV;
        // const MCClockVector &shorterCV = cv1.size() < cv2.size() ? cv1 : cv2;

        // // FIXME: Create an iterator for clock vectors
        // // to simply this loop
        // for (const uint32_t &elem : shorterCV) {

        // }

        return maxCV;
    }

    static MCClockVector newEmptyClockVector()
    {
        return MCClockVector();
    }
};

#endif // MC_MCCLOCKVECTOR_H