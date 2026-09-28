#include <Peach/Map.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

// FNV-1a hash algorithm.
static size_t hash_key(const char* key) {
    uint64_t hash = 14695981039346656037ULL;
    while (*key) {
        hash ^= (unsigned char)(*key++);
        hash *= 1099511628211ULL;
    }
    return (size_t)hash;
}

static mMapSlot* findEntry(const mMap* map, const char* key) {
    if (map == NULL || key == NULL) return NULL;
    size_t index = hash_key(key) % map->capacity;
    // Deleted entries can fill the table, so every probe must be bounded.
    for (size_t i = 0; i < map->capacity; i++) {
        mMapSlot* entry = &map->entries[index];
        if (entry->is_occupied) {
            if (strcmp(entry->key, key) == 0) return entry;
        } else if (!entry->is_tombstone) {
            return NULL;
        }
        index = (index + 1) % map->capacity;
    }
    return NULL;
}

mMap* mMapCreate(void) {
    mMap* map = malloc(sizeof(mMap));
    if (map == NULL) return NULL;
    *map = (mMap){.capacity = INITIAL_CAPACITY};
    map->entries = calloc(map->capacity, sizeof(mMapSlot));
    if (map->entries == NULL) {
        free(map);
        return NULL;
    }
    return map;
}

void mMapDestroy(mMap* map) {
    if (map == NULL) return;
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].is_occupied) {
            free(map->entries[i].key);
            free(map->entries[i].value);
        }
    }
    free(map->entries);
    free(map);
}

static M_BOOL mMapResize(mMap* map) {
    if (map->capacity > SIZE_MAX / 2 / sizeof(mMapSlot)) return M_FALSE;
    size_t capacity = map->capacity * 2;
    mMapSlot* entries = calloc(capacity, sizeof(mMapSlot));
    if (entries == NULL) return M_FALSE;
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].is_occupied) {
            size_t index = hash_key(map->entries[i].key) % capacity;
            while (entries[index].is_occupied) index = (index + 1) % capacity;
            // Move slot metadata, leaving each stored value at its original address.
            entries[index] = map->entries[i];
        }
    }
    free(map->entries);
    map->entries = entries;
    map->capacity = capacity;
    return M_TRUE;
}

M_BOOL mMapSetBytes(mMap* map, const char* key, const void* value, size_t valueSize) {
    if (map == NULL || key == NULL || value == NULL || valueSize == 0) return M_FALSE;
    mMapSlot* existing = findEntry(map, key);
    if (existing != NULL && existing->valueSize == valueSize) {
        memmove(existing->value, value, valueSize);
        return M_TRUE;
    }

    void* storedValue = malloc(valueSize);
    if (storedValue == NULL) return M_FALSE;
    memcpy(storedValue, value, valueSize);
    if (existing != NULL) {
        free(existing->value);
        existing->value = storedValue;
        existing->valueSize = valueSize;
        return M_TRUE;
    }

    size_t keySize = strlen(key) + 1;
    char* storedKey = malloc(keySize);
    if (storedKey == NULL) {
        free(storedValue);
        return M_FALSE;
    }
    memcpy(storedKey, key, keySize);
    if (map->count >= map->capacity - map->capacity / 4 && !mMapResize(map)) {
        free(storedKey);
        free(storedValue);
        return M_FALSE;
    }
    size_t index = hash_key(storedKey) % map->capacity;
    for (size_t i = 0; i < map->capacity; i++) {
        if (!map->entries[index].is_occupied) {
            map->entries[index] = (mMapSlot){.key = storedKey, .value = storedValue,
                                           .valueSize = valueSize, .is_occupied = M_TRUE};
            map->count++;
            return M_TRUE;
        }
        index = (index + 1) % map->capacity;
    }
    free(storedKey);
    free(storedValue);
    return M_FALSE;
}

void* mMapGet(const mMap* map, const char* key) {
    mMapSlot* entry = findEntry(map, key);
    return entry != NULL ? entry->value : NULL;
}

size_t mMapValueSize(const mMap* map, const char* key) {
    mMapSlot* entry = findEntry(map, key);
    return entry != NULL ? entry->valueSize : 0;
}

M_BOOL mMapContains(const mMap* map, const char* key) {
    return findEntry(map, key) != NULL ? M_TRUE : M_FALSE;
}

M_BOOL mMapRemove(mMap* map, const char* key) {
    mMapSlot* entry = findEntry(map, key);
    if (entry == NULL) return M_FALSE;
    free(entry->key);
    free(entry->value);
    *entry = (mMapSlot){.is_tombstone = M_TRUE};
    map->count--;
    return M_TRUE;
}

size_t mMapSize(const mMap* map) {
    return map != NULL ? map->count : 0;
}

mMapIter mMap_iter(const mMap* map) {
    return (mMapIter){.mMap = map, .index = 0};
}

M_BOOL mMap_next(mMapIter* iter, mMapEntry* out_entry) {
    if (iter == NULL || iter->mMap == NULL || out_entry == NULL) return M_FALSE;
    while (iter->index < iter->mMap->capacity) {
        const mMapSlot* entry = &iter->mMap->entries[iter->index++];
        if (entry->is_occupied) {
            *out_entry = (mMapEntry){.key = entry->key, .value = entry->value, .valueSize = entry->valueSize, .resourceType = entry->resourceType};
            return M_TRUE;
        }
    }
    return M_FALSE;
}
