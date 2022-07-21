#ifndef MC_MCCLOCKVECTOR_H
#define MC_MCCLOCKVECTOR_H

#include "misc/MCOptional.h"
#include "MCShared.h"
#include <stdint.h>
#include <unordered_map>
#include <vector>

struct MCClockVector final
{
private:
    /**
     * @brief Maps thread ids to indices into the 
     * 
     */
    std::unordered_map<tid_t, uint32_t> contents;

public:
 
    uint32_t 
    size() const
    {
        return this->contents.size();
    }

    uint32_t&
    operator[](tid_t tid)
    {
        // NOTE: The `operator[]` overload of
        // unordered_map will create a new key-value
        // pair if `tid` does not exist and will use
        // a _default_ value for the value (0 in this case)
        // which is actually what we want here
        return this->contents[tid];
    }

    MCOptional<uint32_t>
    valueForThread(tid_t tid) const
    {
        const auto iter = this->contents.find(tid);
        if (iter != this->contents.end())
            return MCOptional<uint32_t>::some(iter->second);
        return MCOptional<uint32_t>::nil();
    }
    
    /**
     * @brief 
     * 
     * @param cv1 
     * @param cv2 
     * @return MCClockVector 
     */
    static MCClockVector max(const MCClockVector &cv1, const MCClockVector &cv2);

    /**
     * @brief 
     * 
     * @return MCClockVector 
     */
    static MCClockVector newEmptyClockVector()
    {
        return MCClockVector();
    }
};

#endif // MC_MCCLOCKVECTOR_H