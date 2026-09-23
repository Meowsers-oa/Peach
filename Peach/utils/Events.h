//
// Created by Štěpán Toman on 23.09.2026.
//

#ifndef PEACH_EVENTS_H
#define PEACH_EVENTS_H

#include <stdint.h>

typedef struct mEvent mEvent;

typedef void (*mEventCallback)(const mEvent* event, void* userData);

struct mEvent {
    uint32_t type;       // Event type ID / Enum
    void* sender;        // Emitter pointer (optional, can be NULL)
    void* payload;       // Custom event data (optional, can be NULL)
    int handled;        // Set to true to stop event propagation
};

void mListenToEvent(uint32_t type, mEventCallback callback, void* userData);
void mDispatchEvent(mEvent event);
void mUnsubscribeEvent(uint32_t type, mEventCallback callback);

#endif //PEACH_EVENTS_H
