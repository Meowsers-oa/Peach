//
// Created by Štěpán Toman on 23.09.2026.
//

#ifndef PEACH_MAP_H
#define PEACH_MAP_H

#include <stddef.h>

typedef struct {
    char* key;
    void* value;
} mMapEntry;

typedef struct {
    mMapEntry* entries;
    size_t capacity;
    size_t count;
} mMap;

mMap* mMapCreate(void);
int mMapSet(mMap* map, const char* key, void* value);
void* mMapGet(mMap* map, const char* key);
void mMapDestroy(mMap* map);

#endif //PEACH_MAP_H
