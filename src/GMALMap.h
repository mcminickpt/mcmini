#ifndef GMAL_GMALMAP_H
#define GMAL_GMALMAP_H

#include <stdlib.h>
#include <optional>
#include <map>

template<typename Key, typename Value>
class GMALMap {
private:
    std::map<Key, Value> map;
public:
    Value valueForKey(Key key);
    bool remove(Key, Value *);
    bool insert(Key, Value);
    bool contains(Key);
};

template<typename Key, typename Value>
Value GMALMap<Key, Value>::valueForKey(Key key)
{
    return map.find(key)->second;
}

template<typename Key, typename Value>
bool GMALMap<Key, Value>::insert(Key key, Value value)
{
    return map.insert({key, value}).second;
}


#endif //GMAL_GMALMAP_H
