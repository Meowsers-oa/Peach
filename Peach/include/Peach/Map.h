//
// Created by Štěpán Toman on 28.09.2026.
//

#ifndef PEACH_MAP_H
#define PEACH_MAP_H

#include <Peach/Common.h>

mMap* mMapCreate(void);
void mMapDestroy(mMap* map);

// Copies the value; pointer fields stay shallow. Example: mMapSet(map, key, int, 42).
#define mMapSet(map, key, type, ...) \
    mMapSetBytes((map), (key), (type[]){__VA_ARGS__}, sizeof(type))

M_BOOL mMapSetBytes(mMap* map, const char* key, const void* value, size_t valueSize);
// Map-owned pointer: stable across growth; invalidated by removal or different-size replacement.
void* mMapGet(const mMap* map, const char* key);
size_t mMapValueSize(const mMap* map, const char* key);
M_BOOL mMapRemove(mMap* map, const char* key);
M_BOOL mMapContains(const mMap* map, const char* key);
size_t mMapSize(const mMap* map);

// Do not change map membership during iteration.
mMapIter mMap_iter(const mMap* map);
M_BOOL mMap_next(mMapIter* iter, mMapEntry* out_entry);

#endif //PEACH_MAP_H
