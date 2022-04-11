#ifndef GMAL_GMALMAP_H
#define GMAL_GMALMAP_H

#include <stdlib.h>
#include <optional>

namespace gmal {

    template<typename Key, typename Value>
    class GMALMap {

    public:
        Value valueForKey(Key key);

        bool remove(Key, Value *);

        bool insert(Key, Value);

        bool contains(Key);
    };

    template<typename Key, typename Value>
    class GMALFixedMap : public GMALMap<Key, Value> {

    public:
        Value valueForKey(Key key);

        bool remove(Key, Value *);

        bool insert(Key, Value);

        bool contains(Key);
    };

} /* namespace gmal */

#endif //GMAL_GMALMAP_H
