//
// Created by Štěpán Toman on 28.09.2026.
//

#ifndef PEACH_MAP_H
#define PEACH_MAP_H

#include <Peach/Common.h>

mMap* mMapCreate(void);
void mMapDestroy(mMap* map);

// Pass a value, not its address: mMapSet(map, "player", mSprite, sprite).
// Keys and values are copied. Struct pointer fields are shallow copies; the map
// does not free pointed-to resources or destroy GPU handles stored inside values.
// Type must be a complete, non-array value type with ordinary malloc alignment.
#define mMapSet(map, key, type, ...) \
    mMapSetBytes((map), (key), (type[]){__VA_ARGS__}, sizeof(type))

// Byte-copy implementation, also usable for buffers. Source is never retained.
// NULL source and zero size are rejected. Failed inserts preserve existing values.
M_BOOL mMapSetBytes(mMap* map, const char* key, const void* value, size_t valueSize);
// Mutable map-owned storage. Do not free it. Stable across growth and same-size
// replacement; invalidated by removal, different-size replacement, or destruction.
void* mMapGet(const mMap* map, const char* key);
size_t mMapValueSize(const mMap* map, const char* key);
M_BOOL mMapRemove(mMap* map, const char* key);
M_BOOL mMapContains(const mMap* map, const char* key);
// Number of stored key/value pairs (not capacity). NULL maps return zero.
size_t mMapSize(const mMap* map);

// Entries expose the same stored values as mMapGet. Do not insert/remove during iteration.
mMapIter mMap_iter(const mMap* map);
M_BOOL mMap_next(mMapIter* iter, mMapEntry* out_entry);

#endif //PEACH_MAP_H
