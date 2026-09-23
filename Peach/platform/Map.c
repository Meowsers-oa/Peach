//
// Created by Štěpán Toman on 23.09.2026.
//

#include "Map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define INITIAL_CAPACITY 16
#define LOAD_FACTOR_THRESHOLD 0.75

// FNV-1a 64-bit hash function
static uint64_t hashKey(const char* key) {
    uint64_t hash = 14695981039346656037ULL;
    while (*key) {
        hash ^= (unsigned char)(*key++);
        hash *= 1099511628211ULL;
    }
    return hash;
}

mMap* mMapCreate(void) {
    mMap* map = malloc(sizeof(mMap));
    if (!map) return NULL;

    map->capacity = INITIAL_CAPACITY;
    map->count = 0;
    map->entries = calloc(map->capacity, sizeof(mMapEntry));
    if (!map->entries) {
        free(map);
        return NULL;
    }
    return map;
}

static int mMapResize(mMap* map, size_t new_capacity) {
    mMapEntry* new_entries = calloc(new_capacity, sizeof(mMapEntry));
    if (!new_entries) return 0;

    // Rehash existing entries into the new larger array
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].key != NULL) {
            uint64_t hash = hashKey(map->entries[i].key);
            size_t index = (size_t)(hash & (new_capacity - 1));

            while (new_entries[index].key != NULL) {
                index = (index + 1) & (new_capacity - 1);
            }
            new_entries[index] = map->entries[i];
        }
    }

    free(map->entries);
    map->entries = new_entries;
    map->capacity = new_capacity;
    return 0;
}

int mMapSet(mMap* map, const char* key, void* value) {
    if (!map || !key) return 0;

    // Expand map capacity if load factor threshold is hit
    if ((double)(map->count + 1) / map->capacity > LOAD_FACTOR_THRESHOLD) {
        if (!mMapResize(map, map->capacity * 2)) return 0;
    }

    uint64_t hash = hashKey(key);
    size_t index = (size_t)(hash & (map->capacity - 1));

    while (map->entries[index].key != NULL) {
        // Update value if key already exists
        if (strcmp(map->entries[index].key, key) == 0) {
            map->entries[index].value = value;
            return 1;
        }
        index = (index + 1) & (map->capacity - 1);
    }

    // Insert new key-value pair
    map->entries[index].key = strdup(key);
    if (!map->entries[index].key) return 0;

    map->entries[index].value = value;
    map->count++;
    return 1;
}

void* mMapGet(mMap* map, const char* key) {
    if (!map || !key) return NULL;

    uint64_t hash = hashKey(key);
    size_t index = (size_t)(hash & (map->capacity - 1));

    while (map->entries[index].key != NULL) {
        if (strcmp(map->entries[index].key, key) == 0) {
            return map->entries[index].value;
        }
        index = (index + 1) & (map->capacity - 1);
    }

    return NULL;
}

void mMapDestroy(mMap* map) {
    if (!map) return;

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].key != NULL) {
            free(map->entries[i].key);
        }
    }
    free(map->entries);
    free(map);
}